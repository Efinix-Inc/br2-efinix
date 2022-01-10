#!/bin/bash

BOARD_DIR="$(dirname $0)/../$2/"
GENIMAGE_CFG="${BOARD_DIR}/genimage.cfg"
GENIMAGE_TMP="${BUILD_DIR}/genimage.tmp"
OUTPUT_DIR="${O}/images"

rm -rf "${GENIMAGE_TMP}"

genimage                               \
        --rootpath "${TARGET_DIR}"     \
        --tmppath "${GENIMAGE_TMP}"    \
        --inputpath "${BINARIES_DIR}"  \
        --outputpath "${BINARIES_DIR}" \
        --config "${GENIMAGE_CFG}"

rm -rf $BINARIES_DIR/nfs_root
mkdir $BINARIES_DIR/nfs_root
tar -xf $BINARIES_DIR/rootfs.tar -C $BINARIES_DIR/nfs_root
