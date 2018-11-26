/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include <fcntl.h>      // NOLINT(build/include_order)
#include <getopt.h>     // NOLINT(build/include_order)
#include <sys/time.h>   // NOLINT(build/include_order)
#include <sys/types.h>  // NOLINT(build/include_order)
#include <sys/uio.h>    // NOLINT(build/include_order)
#include <unistd.h>     // NOLINT(build/include_order)

#include "tensorflow/contrib/lite/kernels/register.h"
#include "tensorflow/contrib/lite/model.h"
#include "tensorflow/contrib/lite/interpreter.h"
#include "tensorflow/contrib/lite/optional_debug_tools.h"

#include "bitmap_helpers.h"
#include "get_top_n.h"


namespace beeswax {

void display_usage() {
    std::cout << "LEDL version of label_image\n"
              << "./beeswax [OPTIONS]\n"
              << "--help,         -h: show help message\n"
              << "--image,        -i: image_name.bmp\n"
              << "--image_list,   -f: image_list.txt\n"
              << "--tflite_model, -m: model_name.tflite\n"
              << "                    default: ./mobilenet_quant_v1_224.tflite\n"
              << "--labels,       -l: labels for the model\n"
              << "                    default: ./labels.txt\n"
              << "--verbose,      -v: print more information\n"
              << "--count,        -c: loop interpreter->Invoke() for certain times\n"
              << "--profile,      -p: profiler dump path\n"
              << "                    use absolute paths (or './' for execution dir)\n"
              << "--threads,      -t: number of threads\n"
              << "--accelerated,  -a: [0|1], use Android NNAPI or note\n"
              << "--input_mean,   -b: input mean\n"
              << "--input_std,    -s: input standard deviation\n"
              << "\n";
}

double get_micro_s(struct timespec t) {
    return (t.tv_sec * 1000000.0 + t.tv_nsec/1000.0);
}

bool ReadLabelsFile(const std::string& file_name,
                   std::vector<std::string>* result,
                   size_t* found_label_count) {
    std::ifstream file(file_name);
    if (!file) {
        std::cerr << "Labels file " << file_name << " not found\n";
        return false;
    }
    result->clear();
    std::string line;
    while (std::getline(file, line)) {
        result->push_back(line);
    }
    *found_label_count = result->size();
    const int padding = 16;
    while (result->size() % padding) {
        result->emplace_back();
    }
    return true;
}

void PrepareInference(Settings &s,
                  std::unique_ptr<tflite::FlatBufferModel> &model,
                  std::unique_ptr<tflite::Interpreter> &interpreter) {

    if (s.model_name.empty()) {
        std::cerr << "Model name not set\n";
        exit(-1);
    }

    model = tflite::FlatBufferModel::BuildFromFile(s.model_name.c_str());
    if (!model) {
        std::cerr << "\nFailed to mmap model " << s.model_name << "\n";
        exit(-1);
    }
    if (s.verbose) {
        std::cout << "Loaded model " << s.model_name << "\n";
    }

    model->error_reporter();
    if (s.verbose) {
        std::cout << "Resolved reporter\n";
    }

    tflite::ops::builtin::BuiltinOpResolver resolver;

    tflite::InterpreterBuilder(*model, resolver)(&interpreter);
    if (!interpreter) {
        std::cerr << "Failed to construct interpreter\n";
        exit(-1);
    }
    if (s.verbose) {
        std::cout << "Interpreter built\n";
    }

    interpreter->UseNNAPI(s.accel);

    if (s.verbose) {
        std::cout << "tensors size: " << interpreter->tensors_size() << "\n";
        std::cout << "nodes size: " << interpreter->nodes_size() << "\n";
        std::cout << "inputs: " << interpreter->inputs().size() << "\n";
        std::cout << "input(0) name: " << interpreter->GetInputName(0) << "\n";

        int t_size = interpreter->tensors_size();
        for (int i = 0; i < t_size; i++) {
            if (interpreter->tensor(i)->name)
                std::cout << i << ": " << interpreter->tensor(i)->name << ", "
                                       << interpreter->tensor(i)->bytes << ", "
                                       << interpreter->tensor(i)->type << ", "
                                       << interpreter->tensor(i)->params.scale << ", "
                                       << interpreter->tensor(i)->params.zero_point << "\n";
        }
    }

    if (s.number_of_threads != -1) {
        interpreter->SetNumThreads(s.number_of_threads);
    }
}

void RunInference(Settings &s,
                  std::unique_ptr<tflite::Interpreter> &interpreter,
                  const std::string input_img_name,
                  const int image_index,
                  const std::string profile_dump_path) {


    // Input image dimensions (set in read_bmp)
    int image_width;
    int image_height;
    int image_channels;

    // TODO: read jpeg
    uint8_t* in = read_bmp(input_img_name, &image_width, &image_height,
                                                 &image_channels, &s);


    const std::vector<int> inputs = interpreter->inputs();
    const std::vector<int> outputs = interpreter->outputs();

    if (s.verbose) {
        std::cout << "number of inputs: " << inputs.size() << "\n";
        std::cout << "number of outputs: " << outputs.size() << "\n";
    }

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        std::cerr << "Failed to allocate tensors!";
    }

    if (s.verbose){
        PrintInterpreterState(interpreter.get());
    }

    // get input dimension from the input tensor metadata
    // assuming one input only
    TfLiteIntArray* dims = interpreter->tensor(inputs[0])->dims;
    if (s.verbose) {
        std::cout << "input: " << inputs[0] << "\n";
    }

    int wanted_height = dims->data[1];
    int wanted_width = dims->data[2];
    int wanted_channels = dims->data[3];

    switch (interpreter->tensor(inputs[0])->type) {
        case kTfLiteFloat32:
            s.input_floating = true;
            // TODO: resize jpeg and use float
            resize<float>(interpreter->typed_tensor<float>(inputs[0]), in, image_height,
                          image_width, image_channels, wanted_height, wanted_width,
                          wanted_channels, &s);
            break;
        case kTfLiteUInt8:
            // TODO: resize jpeg and use int
            resize<uint8_t>(interpreter->typed_tensor<uint8_t>(inputs[0]), in,
                            image_height, image_width, image_channels, wanted_height,
                            wanted_width, wanted_channels, &s);
            break;
        default:
            std::cerr << "cannot handle input type "
                       << interpreter->tensor(inputs[0])->type << " yet";
            exit(-1);
    }
    struct timespec start_time, stop_time;
    clock_gettime(CLOCK_REALTIME, &start_time);
    TfLiteStatus status;
    if (image_index == -1) {
        for (int i = 0; i < s.loop_count; i++) {
            status = interpreter->Invoke();
        }
    } else {
        char formated_profiler_name[30];
        std::string formated_profiler_name_string;
        // Run invoke and dump profile file named "image0000_loop00.json"
        for (int i = 0; i < s.loop_count; i++) {
            sprintf(formated_profiler_name, "image%04d_loop%02d.json", image_index, i);
            formated_profiler_name_string = formated_profiler_name;
            formated_profiler_name_string = profile_dump_path + formated_profiler_name_string;
            status = interpreter->Invoke(formated_profiler_name_string);
        }
    }
    clock_gettime(CLOCK_REALTIME, &stop_time);
    if (s.verbose) {
        std::cout << "Invoked \n";
    }

    if (status != kTfLiteOk) {
        std::cerr << "Failed to invoke tflite!\n";
    }

    const int output_size = 1000;
    const size_t num_results = 5;
    const float threshold = 0.001f;

    std::vector<std::pair<float, int>> top_results;

    int output = interpreter->outputs()[0];
    switch (interpreter->tensor(output)->type) {
        case kTfLiteFloat32:
            get_top_n<float>(interpreter->typed_output_tensor<float>(0), output_size,
                                             num_results, threshold, &top_results, true);
            break;
        case kTfLiteUInt8:
            get_top_n<uint8_t>(interpreter->typed_output_tensor<uint8_t>(0),
                                                 output_size, num_results, threshold, &top_results,
                                                 false);
            break;
        default:
            std::cerr << "cannot handle output type "
                       << interpreter->tensor(inputs[0])->type << " yet";
            exit(-1);
    }

    std::vector<std::string> labels;
    size_t label_count;

    if (!ReadLabelsFile(s.labels_file_name, &labels, &label_count)) {
        exit(-1);
    }

    // Print results
    std::cout << std::endl;
    std::cout << "image-path: " << input_img_name << std::endl;

    std::cout << "loops: " << s.loop_count << std::endl;

    std::cout << "top-5:";
    for (auto it = top_results.begin(); it != top_results.end(); it++) {
        const float confidence = it->first;
        const int index = it->second;

        if(it == top_results.begin()){
            std::cout << " " << labels[index];
            printf(" (%.2f%%)", confidence * 100);
        } else {
            std::cout << " | " << labels[index];
            printf(" (%.2f%%)", confidence * 100);
        }
    }
    std::cout << std::endl;

    std::cout << "average-time: "
              << (get_micro_s(stop_time) - get_micro_s(start_time)) / (s.loop_count * 1000)
              << " ms \n";


    double start_time_d = start_time.tv_sec + start_time.tv_nsec/1000000000.0;
    double stop_time_d = stop_time.tv_sec + stop_time.tv_nsec/1000000000.0;

    printf("start-end: %.3f %.3f\n", start_time_d, stop_time_d);

    delete [] in;
}


void ParseSettings(Settings &s,
                   std::string &input_img,
                   std::string &input_img_list,
                   int argc,
                   char** argv) {
    int c;

    while (1) {
        static struct option long_options[] = {
            {"accelerated",  required_argument, 0, 'a'},
            {"input_mean",   required_argument, 0, 'b'},
            {"count",        required_argument, 0, 'c'},
            {"image_list",   required_argument, 0, 'f'},
            {"help",         no_argument,       0, 'h'},
            {"image",        required_argument, 0, 'i'},
            {"labels",       required_argument, 0, 'l'},
            {"tflite_model", required_argument, 0, 'm'},
            {"profile",      required_argument, 0, 'p'},
            {"input_std",    required_argument, 0, 's'},
            {"threads",      required_argument, 0, 't'},
            {"verbose",      no_argument,       0, 'v'},
            {0, 0, 0, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "a:b:c:f:hi:l:m:p:s:t:v", long_options,
                                        &option_index);

        /* Detect the end of the options. */
        if (c == -1) break;

        switch (c) {
            case 'a':
                s.accel             = strtol(optarg, NULL, 10);
                break;
            case 'b':
                s.input_mean        = strtod(optarg, NULL);
                break;
            case 'c':
                s.loop_count        = strtol(optarg, NULL, 10);
                break;
            case 'i':
                input_img           = optarg;
                break;
            case 'f':
                input_img_list      = optarg;
                break;
            case 'l':
                s.labels_file_name  = optarg;
                break;
            case 'm':
                s.model_name        = optarg;
                break;
            case 'p':
                s.profile           = optarg;
                break;
            case 's':
                s.input_std         = strtod(optarg, NULL);
                break;
            case 't':
                s.number_of_threads = strtol(optarg, NULL, 10);
                break;
            case 'v':
                s.verbose           = true;
                break;
            case 'h':
                display_usage();
                exit(0);
            case '?':
                /* getopt_long already printed an error message. */
                display_usage();
                exit(-1);
            default:
                exit(-1);
        }
    }
}

int Main(int argc, char** argv) {
    Settings s;
    std::string input_img;
    std::string input_img_list;
    std::unique_ptr<tflite::FlatBufferModel> model;
    std::unique_ptr<tflite::Interpreter> interpreter;
    int image_index = 0;

    ParseSettings(s, input_img, input_img_list, argc, argv);

    PrepareInference(s, model, interpreter);

    // run inference with single image
    if(!input_img.empty()) {
        if(s.profile != ""){
            RunInference(s, interpreter, input_img, image_index, s.profile);
        } else {
            RunInference(s, interpreter, input_img);
        }
    // run inference with list of images from input_img_list
    } else if(!input_img_list.empty()){
        std::ifstream input_file(input_img_list);
        std::string line;

        if (input_file.is_open()) {
            while(std::getline(input_file, line)) {
                if(s.profile != ""){
                    RunInference(s, interpreter, line, image_index, s.profile);
                } else {
                    RunInference(s, interpreter, line);
                }
                image_index++;
            }
            input_file.close();
        } else {
            std::cerr << "Unable to open file " << input_img_list << std::endl;
        }
    } else {
        std::cerr << "No image input set. Use flag -i or -f" << std::endl;
        display_usage();
    }

    return 0;
}

}  // namespace beeswax

int main(int argc, char** argv) {
    return beeswax::Main(argc, argv);
}
