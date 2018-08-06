# benchmark-tfl

## Label image

TFL example. Compile as following:

```
cd $BENCHMARKK_TFL_ROOT_DIR
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../toolchains/tizen.cmake -DTIZEN_DEVICE=ON -DTIZEN_TARGET=mobile-4.0 -DTENSORFLOW_LITE_SOURCE_DIR=$TENSORFLOW_ROOT_DIR/tensorflow/contrib/lite
make -j32
```

You should see the `label_image` executable into your local machine and run it like so:

```
rsync -zah hive:benchmark_tfl/build/label_image .
rsync -zah hive:benchmark_tfl/testdata .
create_tunnel -u
create_tunnel rpi
sdb connect localhost:10101 && sdb root on
sdb push label_image label_image
sdb push testdata/mobilenet_quant_v1_224.tflite mobilenet_quant_v1_224.tflite
sdb push testdata/labels.txt labels.txt
sdb push testdata/grace_hopper.bmp grace_hopper.bmp
sdb shell
./label_image
```