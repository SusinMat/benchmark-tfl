# benchmark-tfl

## Label image
TensorFlow Lite example

First, compile TFL for the target: (Requires CMAKE 3.6)

```
cd $TENSORFLOW_ROOT_DIR/tensorflow/contrib/lite
sh ./download_dependencies.sh
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/tmp/$USER/tensorflow_lite -DTIZEN_TARGET=mobile-4.0 -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/tizen.cmake -DCMAKE_VERBOSE_MAKEFILE=ON
make install
```

Then, compile with label\_image:

```
mkdir build
cd build
cmake .. -DTIZEN_TARGET=mobile-4.0 -DCMAKE_TOOLCHAIN_FILE=../toolchains/tizen.cmake
make
```
