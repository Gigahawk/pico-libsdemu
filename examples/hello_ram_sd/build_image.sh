#!/usr/bin/env bash

IMAGE_NAME=base_image.img

dd if=/dev/zero of=$IMAGE_NAME bs=1k count=8208

mkfs.vfat -F16 $IMAGE_NAME

python export_image.py $IMAGE_NAME