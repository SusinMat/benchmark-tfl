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

#ifndef BEESWAX_H
#define BEESWAX_H

#include <vector>
#include <memory>

#include "tensorflow/contrib/lite/interpreter.h"
#include "tensorflow/contrib/lite/model.h"

namespace beeswax {

// Inference settings
struct Settings {
    bool verbose = false;
    std::string profile = "";
    bool accel = false;
    bool input_floating = false;
    int loop_count = 1;
    float input_mean = 127.5f;
    float input_std = 127.5f;
    std::string model_name = "./mobilenet_quant_v1_224.tflite";
    std::string labels_file_name = "./labels.txt";
    std::string input_layer_type = "uint8_t";
    int number_of_threads = 4;
};

// Display beeswax usage
void display_usage();

double get_us(struct timeval t);

// Takes a file name, and loads a list of labels from it, one per line, and
// returns a vector of the strings. It pads with empty strings so the length
// of the result is a multiple of 16, because our model expects that.
bool ReadLabelsFile(const std::string &file_name,
                    std::vector<std::string> *result,
                    size_t *found_label_count);

// Parse command line arguments
void ParseSettings(Settings &s,
                   std::string &input_img,
                   std::string &input_img_list,
                   int argc,
                   char** argv);

void PrepareInference(Settings &s,
                      std::unique_ptr<tflite::FlatBufferModel> &model,
                      std::unique_ptr<tflite::Interpreter> &interpreter);

// Run inference with settings s on the img input_img_name
void RunInference(Settings &s,
                  std::unique_ptr<tflite::Interpreter> &interpreter,
                  const std::string input_img_name,
                  const int imageindex = -1,
                  const std::string profile_dump_path = "");

// Main function
int Main(int argc, char** argv);

}  // namespace beeswax

#endif  // BEESWAX_H