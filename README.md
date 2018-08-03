# benchmark-tfl

## Label image
TensorFlow Lite example

First, compile TFL for the target: (Requires CMAKE 3.6)

```
cd $TENSORFLOW_ROOT_DIR/tensorflow/contrib/lite
sh ./download_dependencies.sh
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/tmp/$USER/tensorflow_lite -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/tizen.cmake -DTIZEN_DEVICE=ON -DTIZEN_TARGET=mobile-4.0
make install
```

Then, compile with label\_image:

```
cd $BENCHMARKK_TFL_ROOT_DIR
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchains/tizen.cmake -DTIZEN_DEVICE=ON -DTIZEN_TARGET=mobile-4.0
make
```
