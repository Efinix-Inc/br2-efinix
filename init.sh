#!/bin/bash -e

# args1: board
# args2: path/to/soc.h

BOARD=$1
SOC_H=$2
OPT_DIR=$3

INPUT_FILE="VERSION"
JSON_FILE="boards/efinix/common/drivers.json"
devkits=$(jq '.devkits | .[]' $JSON_FILE)

arr=()
substr="buildroot"
buildroot_version=''
BUILDROOT_REPO="https://github.com/buildroot/buildroot.git"

PROJ_DIR=$PWD
BR2_EXTERNAL_DIR=$PROJ_DIR
BUILDROOT_DEFCONFIG=""

function usage()
{
	echo "$0 <board [t120f324|ti60f225]> <path/to/soc.h> <optional workdir>"
	echo
	echo "Example,"
	echo "$0 t120f324 ~/efinity/2022.1/project/soc/ip/soc1/T120F324_devkit/embedded_sw/soc1/bsp/efinix/EfxSapphireSoc/include/soc.h"
}

if [[ -z $BOARD ]]; then
	echo Error: Board is not defined
	usage
	return
fi

if [[ -z $SOC_H ]]; then
	echo Error: soc.h is not defined
	usage
	return
fi

# check the compatible of $BOARD in $JSON_FILE
found=false
for devkit in ${devkits[@]}; do
        if [[ $devkit == *$BOARD* ]]; then
                BOARD=$(echo $devkit | tr -d \")
                BUILDROOT_DEFCONFIG="efinix_"$BOARD"_defconfig"
		found=true
		break
        fi
done

if [[ $found == false ]]; then
	echo Error: board $BOARD is not supported
	return
fi

WORKSPACE="build_$BOARD"

if [[ ! -z $OPT_DIR ]]; then
	WORKSPACE=$OPT_DIR
fi

WORKSPACE_DIR="$PROJ_DIR/../$WORKSPACE"
BUILDROOT_DIR="$PROJ_DIR/../$WORKSPACE/buildroot"
BUILD_DIR="$PROJ_DIR/../$WORKSPACE/build"

mkdir -p $WORKSPACE_DIR

# Read the VERSION file to get buildroot version
while IFS= read -r line; do
	if [[ $line == *$substr* ]];
	then
		IFS=' ' read -r -a arr <<< "$line"
	fi
done < $INPUT_FILE

buildroot_version=${arr[1]}

# clone buildroot repository
if [[ ! -z $buildroot_version ]]; then
	echo Using Buildroot $buildroot_version
	git clone $BUILDROOT_REPO -b $buildroot_version
else
	echo Using Buildroot master/main branch
	git clone $BUILDROOT_REPO
fi

mv buildroot $BUILDROOT_DIR

# copy soc.h to opensbi directory. Opensbi has dependency on soc.h.
OPENSBI_DIR="$BR2_EXTERNAL_DIR/boards/efinix/$BOARD/opensbi"
cp $SOC_H $OPENSBI_DIR/soc.h

SOC_H=$OPENSBI_DIR/soc.h

# check for SPI1
if [[ -z $(cat $SOC_H | grep SYSTEM_SPI_1_IO_CTRL) ]]; then
	echo Error: The generated SoC does not enable SPI1.
	echo        Please regenerate the SoC with SPI1.
	return
fi

# count number of cpu core and modify soc.h
cpu_count=1
for i in {4..1}; do
        num=$(( $i-1))
        substr="SYSTEM_PLIC_SYSTEM_CORES_${num}_EXTERNAL_INTERRUPT"
        if [[ $(cat $SOC_H | grep $substr) ]]; then
            cpu_count=$i
            break
        fi
done

echo "Detecting $cpu_count RISC-V CPU cores"
if [[ $cpu_count -gt 1 ]]; then
	# enable CONFIG_SMP=y in linux.config
	sed -i 's/^CONFIG_SMP=n/CONFIG_SMP=y/g' $BR2_EXTERNAL_DIR/boards/efinix/$BOARD/linux/linux.config
else
	sed -i 's/^CONFIG_SMP=y/CONFIG_SMP=n/g' $BR2_EXTERNAL_DIR/boards/efinix/$BOARD/linux/linux.config
fi

# modify soc.h by appending SYSTEM_CORES_COUNT
IFS=$'\n' read -d '' -r -a lines < $SOC_H
lines[${#lines[@]}-1]="#define SYSTEM_CORES_COUNT $cpu_count"
lines[${#lines[@]}]="#endif"
printf "%s\n" "${lines[@]}" > ${SOC_H}

# generate device tree
python3 boards/efinix/common/device_tree_generator.py $SOC_H $BOARD

# prepare Buildroot build environment
mkdir $BUILD_DIR
cd $BUILD_DIR && \
make O=$PWD BR2_EXTERNAL=$BR2_EXTERNAL_DIR -C $BUILDROOT_DIR $BUILDROOT_DEFCONFIG
