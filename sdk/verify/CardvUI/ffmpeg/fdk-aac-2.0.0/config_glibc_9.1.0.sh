
#!/bin/sh

CROSS_COMPILE=arm-linux-gnueabihf-
LDFLAGS="-lc -lgcc -lm -ldl  "

#CROSS_COMPILE=/home/media/jeff.li/toolchain/arm-buildroot-linux-uclibcgnueabihf-4.9.4/bin/arm-buildroot-linux-uclibcgnueabihf-
#LDFLAGS="-L/home/media/jeff.li/toolchain/arm-buildroot-linux-uclibcgnueabihf-4.9.4/lib -lc -lgcc -lm -ldl  "

#FLAGS="--enable-shared --enable-static --prefix=/home/media/jeff.li/I6e/code/cardv/alkaid/sdk/verify/Cardvimpl/fdk-aac-2.0.0 --host=arm-linux-gnueabihf"
FLAGS="--enable-shared --enable-static --prefix=$(pwd)/output/glibc/9.1.0 --host=arm-linux-gnueabihf"
CFLAGS="-fPIC  -g -O2"
CPPFLAGS="-fPIC  -g -O2"

export CXX="${CROSS_COMPILE}g++"
export LDFLAGS="$LDFLAGS"
export CFLAGS="$CFLAGS"
export CPPFLAGS="$CPPFLAGS"
export CC="${CROSS_COMPILE}gcc"

./configure $FLAGS \
