
#!/bin/sh

#CROSS_COMPILE=/home/media/jeff.li/toolchain/arm-linux-gnueabihf-4.8.3-201404/bin/arm-linux-gnueabihf-
#LDFLAGS="-L/home/media/jeff.li/toolchain/arm-linux-gnueabihf-4.8.3-201404/lib -lc -lgcc -lm -ldl  "

CROSS_COMPILE=/home/media/jeff.li/toolchain/arm-buildroot-linux-uclibcgnueabihf-4.9.4/bin/arm-buildroot-linux-uclibcgnueabihf-
LDFLAGS="-L/home/media/jeff.li/toolchain/arm-buildroot-linux-uclibcgnueabihf-4.9.4/lib -lc -lgcc -lm -ldl  "

FLAGS="--disable-shared --enable-static --prefix=/home/media/jeff.li/I6/code/alkaid/sdk/verify/fdk-aac-2.0.0 --host=x86_64"
CFLAGS="-fPIC -O2"
CPPFLAGS="-fPIC -O2"

export CXX="${CROSS_COMPILE}g++"
export LDFLAGS="$LDFLAGS"
export CFLAGS="$CFLAGS"
export CPPFLAGS="$CPPFLAGS"
export CC="${CROSS_COMPILE}gcc"

./configure $FLAGS \
