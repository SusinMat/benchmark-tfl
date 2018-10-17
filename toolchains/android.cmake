#  Tizen CMake toolchain file
#  Usage Linux:
#   $ mkdir build && cd build
#   $ cmake -DCMAKE_TOOLCHAIN_FILE=path/to/the/tizen.cmake ..
#   $ make -j8

set (ANDROID_TOOLCHAIN "/local/$ENV{USER}/android-toolchain" CACHE PATH "Location of Android toolchain.")

message(STATUS "Android toolchain path: ${ANDROID_TOOLCHAIN}")

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)

# set(__android_toolchain_root "${ANDROID_TOOLCHAIN}/toolchains/${__android_toolchain}")
set(__android_toolchain_prefix "arm-linux-androideabi-")

set(CMAKE_C_COMPILER   "${ANDROID_TOOLCHAIN}/bin/${__android_toolchain_prefix}gcc"     CACHE PATH "c compiler")
set(CMAKE_CXX_COMPILER "${ANDROID_TOOLCHAIN}/bin/${__android_toolchain_prefix}g++"     CACHE PATH "c++ compiler")
set(CMAKE_STRIP        "${ANDROID_TOOLCHAIN}/bin/${__android_toolchain_prefix}strip"   CACHE PATH "strip")
set(CMAKE_AR           "${ANDROID_TOOLCHAIN}/bin/${__android_toolchain_prefix}ar"      CACHE PATH "archive")
set(CMAKE_LINKER       "${ANDROID_TOOLCHAIN}/bin/${__android_toolchain_prefix}ld"      CACHE PATH "linker")
set(CMAKE_NM           "${ANDROID_TOOLCHAIN}/bin/${__android_toolchain_prefix}nm"      CACHE PATH "nm")
set(CMAKE_OBJCOPY      "${ANDROID_TOOLCHAIN}/bin/${__android_toolchain_prefix}objcopy" CACHE PATH "objcopy")
set(CMAKE_OBJDUMP      "${ANDROID_TOOLCHAIN}/bin/${__android_toolchain_prefix}objdump" CACHE PATH "objdump")
set(CMAKE_RANLIB       "${ANDROID_TOOLCHAIN}/bin/${__android_toolchain_prefix}ranlib"  CACHE PATH "ranlib")

set(ANDROID_SYSROOT "${ANDROID_TOOLCHAIN}/sysroot") # Note: parametize version number and architecture
# set(ANDROID_SYSROOT "${__android_rootstraps}/${ANDROID_TARGET}-device.core" CACHE PATH "" FORCE)

message(STATUS "Android SYSROOT: ${ANDROID_SYSROOT}")

set(CMAKE_FIND_ROOT_PATH "${ANDROID_SYSROOT}" "${ANDROID_TOOLCHAIN}/bin")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# add_definitions(-DANDROID_DEPRECATION)
# add_definitions(-DDEPRECATION_WARNING)

# common flags

set(ANDROID_EXTRA_FLAGS "${ANDROID_EXTRA_FLAGS} -gcc-toolchain \"${ANDROID_TOOLCHAIN}\"")
# set(ANDROID_EXTRA_FLAGS "${ANDROID_EXTRA_FLAGS} -ccc-gcc-name-toolchain ${__android_toolchain_prefix}g++")

set(ANDROID_EXTRA_FLAGS "${ANDROID_EXTRA_FLAGS} --sysroot=${ANDROID_SYSROOT}")
set(ANDROID_EXTRA_FLAGS "${ANDROID_EXTRA_FLAGS} -isysroot=${ANDROID_SYSROOT}")
set(CMAKE_SYSROOT "${ANDROID_SYSROOT}")
set(ANDROID_EXTRA_FLAGS "${ANDROID_EXTRA_FLAGS} -fmessage-length=0")
set(ANDROID_EXTRA_FLAGS "${ANDROID_EXTRA_FLAGS} -Wno-gnu")
set(ANDROID_EXTRA_FLAGS "${ANDROID_EXTRA_FLAGS} -fPIC -fPIE")

set(ANDROID_LINKER_FLAGS "${ANDROID_LINKER_FLAGS} -pie")

# archtecture-specific flags
# add_definitions(-D_DEBUG)
# add_definitions(-D__ARM_NEON)
set(ANDROID_EXTRA_FLAGS "${ANDROID_EXTRA_FLAGS} -march=armv7-a")
set(ANDROID_EXTRA_FLAGS "${ANDROID_EXTRA_FLAGS} -mfloat-abi=softfp")
# set(ANDROID_EXTRA_FLAGS "${ANDROID_EXTRA_FLAGS} -mfpu=neon-vfpv4")
# set(ANDROID_EXTRA_FLAGS "${ANDROID_EXTRA_FLAGS} -mtune=cortex-a8")
set(ANDROID_EXTRA_FLAGS "${ANDROID_EXTRA_FLAGS} -mthumb")
set(ANDROID_EXTRA_FLAGS "${ANDROID_EXTRA_FLAGS} -target arm-linux-androideabi")

set(ANDROID_C_FLAGS "${ANDROID_C_FLAGS} ${ANDROID_EXTRA_FLAGS}")
set(ANDROID_CXX_FLAGS "${ANDROID_CXX_FLAGS} ${ANDROID_EXTRA_FLAGS}")
set(ANDROID_FLAGS_RELEASE "-O3")
set(ANDROID_FLAGS_DEBUG "-O0 -g3 -Wall")

set(CMAKE_C_FLAGS           "${CMAKE_C_FLAGS}           ${ANDROID_C_FLAGS}"       CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         ${ANDROID_CXX_FLAGS}"     CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS}  ${ANDROID_LINKER_FLAGS}"  CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${ANDROID_FLAGS_RELEASE}" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}   ${ANDROID_FLAGS_RELEASE}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   ${ANDROID_FLAGS_DEBUG}"   CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_DEBUG     "${CMAKE_C_FLAGS_DEBUG}     ${ANDROID_FLAGS_DEBUG}"   CACHE STRING "" FORCE)

# vim: set ts=4 sts=4 sw=4 et:
