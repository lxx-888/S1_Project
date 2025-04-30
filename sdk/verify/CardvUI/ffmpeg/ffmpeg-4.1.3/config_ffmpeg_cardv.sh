#!/bin/bash

# ./config_ffmpeg_cardv.sh -a ~/I6/code/cardv/alkaid -c i6e -p cardv -t glibc -v 9.1.0

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
--disable-static \
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
--enable-swscale \
--enable-swresample \
--disable-postproc \
--enable-pthreads \
--disable-network \
--disable-dct \
--disable-dwt \
--enable-error-resilience \
--disable-lsp \
--disable-lzo \
--enable-mdct \
--enable-rdft \
--enable-fft \
--enable-faan \
--disable-pixelutils \
--disable-symver \
--disable-everything \
--disable-protocols \
--enable-protocol=file \
--enable-parser=h264 \
--disable-parser=hevc \
--disable-decoders \
--disable-decoder=hevc \
--enable-decoder=h264 \
--disable-decoder=ssh264 \
--disable-decoder=sshevc \
--enable-decoder=aac \
--enable-decoder=mjpeg \
--disable-encoders \
--enable-encoder=libfdk_aac \
--enable-libfdk-aac \
--enable-version3 \
--disable-demuxers \
--enable-demuxer=aac \
--enable-demuxer=mpegts \
--enable-demuxer=mov \
--enable-demuxer=image_jpeg_pipe \
--disable-muxers \
--enable-muxer=mpegts \
--enable-muxer=mp4 \
--extra-cflags="-I$(echo $FDKAAC_PATH)/include -I$(echo $MI_INC_PATH)" \
--extra-ldflags="-L$(echo $FDKAAC_PATH)/$(echo $toolchain)/$(echo $toolchain_version) -L$(echo $MI_LIB_PATH)" \
