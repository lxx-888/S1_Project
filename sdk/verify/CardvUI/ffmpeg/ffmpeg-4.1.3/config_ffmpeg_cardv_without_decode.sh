#!/bin/bash

# ./config_ffmpeg_cardv_without_decode.sh -a ~/I6/code/cardv/alkaid -c i6b0 -p cardv -t uclibc -v 4.9.4

while getopts "a:c:p:t:v:" opt; do
  case $opt in
    a)
                alkaid_dir=$OPTARG
                ;;
    c)
                chip=$OPTARG
                ;;
    p)
                product=$OPTARG
                ;;
    t)
                toolchain=$OPTARG
                ;;
    v)
                toolchain_version=$OPTARG
                ;;
        \?)
          echo "Invalid option: -$OPTARG" >&2
          ;;
  esac
done

export PREFIX=$(pwd)/output/$(echo $toolchain)/$(echo $toolchain_version)
export FDKAAC_PATH=$(pwd)/../fdk-aac-2.0.0
export MI_INC_PATH=$(echo $alkaid_dir)/project/release/include
export MI_LIB_PATH=$(echo $alkaid_dir)/project/release/chip/$(echo $chip)/$(echo $product)/common/$(echo $toolchain)/$(echo $toolchain_version)/mi_libs/dynamic

export CC=arm-buildroot-linux-uclibcgnueabihf-
if [ "${toolchain}" = "glibc" ]
then
  export CC=arm-linux-gnueabihf-
fi

./configure \
--prefix=$(echo $PREFIX) \
--enable-cross-compile \
--cross-prefix=$(echo $CC) \
--arch=arm \
--target-os=linux \
--enable-small \
--enable-static \
--enable-shared \
--disable-stripping \
--datadir=$(pwd)/share/ffmpeg \
--disable-doc \
--disable-htmlpages \
--disable-manpages \
--disable-podpages \
--disable-txtpages \
--disable-ffplay \
--disable-ffprobe \
--disable-ffmpeg \
--disable-avdevice \
--disable-avfilter \
--disable-swscale \
--disable-swresample \
--disable-postproc \
--enable-pthreads \
--disable-network \
--disable-dct \
--disable-dwt \
--enable-error-resilience \
--disable-lsp \
--disable-lzo \
--disable-mdct \
--disable-rdft \
--disable-fft \
--disable-faan \
--disable-pixelutils \
--disable-symver \
--disable-everything \
--disable-protocols \
--enable-protocol=file \
--disable-decoders \
--disable-encoders \
--enable-encoder=libfdk_aac \
--enable-libfdk-aac \
--enable-version3 \
--enable-encoder=mjpeg \
--disable-demuxers \
--enable-demuxer=mpegts \
--enable-demuxer=mov \
--disable-muxers \
--enable-muxer=mpegts \
--enable-muxer=mp4 \
--extra-cflags="-I$(echo $FDKAAC_PATH)/include -I$(echo $MI_INC_PATH)" \
--extra-ldflags="-L$(echo $FDKAAC_PATH)/$(echo $toolchain)/$(echo $toolchain_version) -L$(echo $MI_LIB_PATH)" \
