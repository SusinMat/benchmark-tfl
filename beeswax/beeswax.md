# Beeswax
By LEDL inspired by TensorFlow Lite's label_image.

## Usage

```
LEDL version of label_image
./beeswax [OPTIONS]
--help,         -h: show help message
--image,        -i: image_name.bmp
--image_list,   -f: image_list.txt
--tflite_model, -m: model_name.tflite
                    default: ./mobilenet_quant_v1_224.tflite
--labels,       -l: labels for the model
                    default: ./labels.txt
--verbose,      -v: [0|1] print more information
--count,        -c: loop interpreter->Invoke() for certain times
--threads,      -t: number of threads
--accelerated,  -a: [0|1], use Android NNAPI or note
--input_mean,   -b: input mean
--input_std,    -s: input standard deviation
```

## Prepare

To run it you need three files:
- `./mobilenet_quant_v1_224.tflite`
    - a tensorflow flatbuffer model
- `./labels.txt`.
    - a text file with the labels of your model
- `./grace_hopper.bmp`
    - an image

## Compile

```sh
cd $BENCHMARKK_ROOT_DIR
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DTENSORFLOW_LITE_SOURCE_DIR=$TENSORFLOW_ROOT_DIR/tensorflow/contrib/lite \
         -DCMAKE_TOOLCHAIN_FILE=../toolchains/tizen.cmake \
         -DTIZEN_DEVICE=ON \
         -DTIZEN_TARGET=mobile-4.0
make -j8
```

Additionally, you can set the following arguments with cmake. They are shown with their default values:

```sh
-DTENSORFLOW_LITE_INSTALL_DIR=/tmp/$USER/tensorflow_lite
-DTIZEN_SDK=$HOME/tizen-studio
```

You should see the `beeswax` ARM executable inside the build directory.   

## Send executable to raspberry pi

```sh
# Pull back the executable and testdata from hive
rsync -zah hive:benchmark_tfl/build/beeswax .
rsync -zah hive:benchmark_tfl/testdata .

# Connect to raspberry pi
create_tunnel -u
create_tunnel rpi
sdb connect localhost:10101 && sdb root on

# Send executable and testdata to raspberry pi
sdb push beeswax beeswax
sdb push testdata/mobilenet_quant_v1_224.tflite mobilenet_quant_v1_224.tflite
sdb push testdata/labels.txt labels.txt
sdb push testdata/grace_hopper.bmp grace_hopper.bmp

# Enter raspberry pi shell
sdb shell

# Execute beeswax
./beeswax --help
```

## Run it

```sh
$ ./beeswax -i grace_hopper.bmp

image-path: grace_hopper.bmp
top-5: military uniform | Windsor tie | mortarboard | bow tie | drumstick
time: 308.549 ms
```