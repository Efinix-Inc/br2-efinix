#!/bin/bash -e

# args1: board
# args2: path/to/soc.h

OPTIND=1
unset RECONFIGURE
unset RECONFIGURE_ALL
unset OPT_DIR
unset ETHERNET

BOARD=$1
SOC_H=$2

INPUT_FILE="VERSION"
JSON_FILE="boards/efinix/common/sapphire-soc-dt-generator/config/drivers.json"

substr=""
buildroot_version=''
BUILDROOT_REPO="https://github.com/buildroot/buildroot.git"

PROJ_DIR=$PWD
BR2_EXTERNAL_DIR=$PROJ_DIR
BUILDROOT_DEFCONFIG=""

EFINIX_DIR="$BR2_EXTERNAL_DIR/boards/efinix"
COMMON_DIR="$EFINIX_DIR/common"
DT_DIR="$COMMON_DIR/sapphire-soc-dt-generator"
DT_REPO="https://github.com/Efinix-Inc/sapphire-soc-dt-generator.git"

function usage()
{
	echo "Usage: $0 [board <t120f324|ti60f225>] [path/to/soc.h <string> ] [-d build directory <string>] [-r reconfigure]"
	echo
	echo "Positional arguments:"
	echo "	board			development kit name such as t120f324, ti60f225"
	echo "	soc			path to soc.h. This file is located in Efinity project directory"
	echo
	echo "Optional arguments:"
	echo "	-h			Show this help message and exit"
	echo "	-d			Rename default build directory name"
	echo "				By default is <board>_build"
	echo "				Example, if board is ti60f225 then name of build directory is ti60f225_build"
	echo "	-r			Reconfigure the Buildroot configuration"
	echo "	-a			Reconfigure the Buildroot configuration and regenerate Linux device tree"
	echo "	-e                      Generate Linux configuration with ethernet support"

	echo "Example usage,"
	echo "$0 t120f324 ~/efinity/2022.1/project/soc/ip/soc1/T120F324_devkit/embedded_sw/soc1/bsp/efinix/EfxSapphireSoc/include/soc.h"
}

function sanity_check()
{
	if [[ ! -f $SOC_H ]]; then
		echo "Error: $SOC_H file not exists"
		return
	fi

	# check the compatible of $BOARD in $JSON_FILE
	local found=false
	local devkit_l
	BOARD=$(echo $BOARD | tr '[:upper:]' '[:lower:]')

	devkits=$(jq '.devkits | .[]' $JSON_FILE)

	for devkit in ${devkits[@]}; do
		devkit_l=$(echo $devkit | tr '[:upper:]' '[:lower:]')
		if [[ $devkit_l == *$BOARD* ]]; then
			BOARD=$(echo $devkit_l | tr -d \")
			BUILDROOT_DEFCONFIG="efinix_"$BOARD"_defconfig"
			found=true
			break
		fi
	done

	if [[ $found == false ]]; then
		echo Error: board $BOARD is not supported
		return 1
	fi

	# check for SPI1. SPI1 is used by the SD card to store Linux image
	if [[ -z $(cat $SOC_H | grep SYSTEM_SPI_1_IO_CTRL) ]]; then
		echo Error: The generated SoC does not enable SPI1.
		echo        Please regenerate the SoC with SPI1.
		return
	fi
}

function modify_soc_h()
{
	# modify soc.h by appending SYSTEM_CORES_COUNT
	if [[ ! $(cat $SOC_H | grep SYSTEM_CORES_COUNT) ]]; then
		IFS=$'\n' read -d '' -r -a lines < $SOC_H
		lines[${#lines[@]}-1]="#define SYSTEM_CORES_COUNT $cpu_count"
		lines[${#lines[@]}]="#endif"
		printf "%s\n" "${lines[@]}" > ${SOC_H}
	fi

	# check for floating point from soc.h and modify buildroot defconfig
	fp=$(cat ${SOC_H} | grep FPU | awk  '{print $3}' | head -1)
	if [[ $fp == 0 ]]; then
		# disable the floating point
		echo "INFO: Disable floating point in $BR2_EXTERNAL_DIR/configs/$BUILDROOT_DEFCONFIG"
		sed -i 's/^BR2_RISCV_ISA_CUSTOM_RVF=y/BR2_RISCV_ISA_CUSTOM_RVF=n/g' $BR2_EXTERNAL_DIR/configs/$BUILDROOT_DEFCONFIG
		sed -i 's/^BR2_RISCV_ISA_CUSTOM_RVD=y/BR2_RISCV_ISA_CUSTOM_RVD=n/g' $BR2_EXTERNAL_DIR/configs/$BUILDROOT_DEFCONFIG
		sed -i 's/^BR2_RISCV_ABI_ILP32D=y/BR2_RISCV_ABI_ILP32=y/g' $BR2_EXTERNAL_DIR/configs/$BUILDROOT_DEFCONFIG

		echo "INFO: Disable Linux FPU support in $BR2_EXTERNAL_DIR/boards/efinix/$BOARD/linux/linux.config"
		sed -i 's/^CONFIG_FPU=y/CONFIG_FPU=n/g' $BR2_EXTERNAL_DIR/boards/efinix/$BOARD/linux/linux.config
	fi

	# check for compressed extension from soc.h
	ext_c=$(cat ${SOC_H} | grep SYSTEM_RISCV_ISA_EXT_C | awk '{print $3}' | head -1)
	if [ $ext_c == 1 ]; then
		# enable compressed extension flag in buildroot defconfig
		echo "INFO: Enable compressed extensin in $BR2_EXTERNAL_DIR/configs/$BUILDROOT_DEFCONFIG"
		sed -i 's/BR2_RISCV_ISA_CUSTOM_RVC=n/BR2_RISCV_ISA_CUSTOM_RVC=y/g' $BR2_EXTERNAL_DIR/configs/$BUILDROOT_DEFCONFIG
		sed -i 's/CONFIG_RISCV_ISA_C=n/CONFIG_RISCV_ISA_C=y/g' $BR2_EXTERNAL_DIR/boards/efinix/$BOARD/linux/linux.config
	fi
}

function generate_device_tree()
{
	# generate device tree
	if [ $ETHERNET ]; then
		python3 $DT_DIR/device_tree_generator.py \
			-s $DT_DIR/config/linux_slaves.json \
			-c $DT_DIR/config/linux_spi.json \
			-c $DT_DIR/config/ethernet.json \
			$SOC_H $BOARD linux
	else
		python3 $DT_DIR/device_tree_generator.py \
			-s $DT_DIR/config/linux_slaves.json \
			-c $DT_DIR/config/linux_spi.json \
			$SOC_H $BOARD linux
	fi

	cp $DT_DIR/dts/sapphire.dtsi $COMMON_DIR/dts/sapphire.dtsi
	cp $DT_DIR/dts/linux.dts $EFINIX_DIR/$BOARD/linux/linux.dts
}

function prepare_buildroot_env()
{
	# prepare Buildroot build environment
	mkdir $BUILD_DIR
	cd $BUILD_DIR && \
	make O=$PWD BR2_EXTERNAL=$BR2_EXTERNAL_DIR -C $BUILDROOT_DIR $BUILDROOT_DEFCONFIG
}

function get_cpu_count()
{
	# count number of cpu core
	cpu_count=1
	for i in {4..1}; do
		num=$(( $i-1))
		substr="SYSTEM_PLIC_SYSTEM_CORES_${num}_EXTERNAL_INTERRUPT"
		if [[ $(cat $SOC_H | grep $substr) ]]; then
			cpu_count=$i
			break
		fi
	done
}

function check_smp()
{
	get_cpu_count
	echo "INFO: Detecting $cpu_count RISC-V CPU cores"
	if [[ $cpu_count -gt 1 ]]; then
		# enable CONFIG_SMP=y in linux.config
		sed -i 's/^CONFIG_SMP=n/CONFIG_SMP=y/g' $BR2_EXTERNAL_DIR/boards/efinix/$BOARD/linux/linux.config
	else
		sed -i 's/^CONFIG_SMP=y/CONFIG_SMP=n/g' $BR2_EXTERNAL_DIR/boards/efinix/$BOARD/linux/linux.config
	fi
}

function get_version()
{
	# Read the VERSION file to get version

	local arr=()
	while IFS= read -r line; do
		if [[ $line == *$substr* ]]; then
			IFS=' ' read -r -a arr <<< "$line"
		fi
	done < $INPUT_FILE

	version=${arr[1]}
}

while [ $# -gt 0 ]
do
	case $1 in
		-*) break
		;;
	esac
	shift
done

while getopts ":d:raeh" o; do
	case "${o}" in
		:)
                        echo "ERROR: Option -$OPTARG requires an argument"
                        ;;
		d)
			OPT_DIR=${OPTARG}
			;;
		a)
			RECONFIGURE_ALL=1
			RECONFIGURE=1
			;;
		r)
			RECONFIGURE=1
			;;
		e)
			ETHERNET=1
			;;
		h)
			usage
			return
			;;
		\?)
			echo "ERROR: Invalid option -$OPTARG"
			usage
			;;
		esac
done
shift $((OPTIND-1))

if [[ -z $BOARD ]]; then
	echo Error: Board is not defined
	return
fi

if [[ -z $SOC_H ]]; then
	echo Error: soc.h is not defined
	return
fi

# clone sapphire-soc-dt-generator repository
if [ ! -d $DT_DIR ]; then
        substr="sapphire-soc-dt-generator"
        get_version $substr
        dt_version=$version

        if [ ! -z $dt_version ]; then
                git clone $DT_REPO -b $dt_version $DT_DIR
        else
                git clone $DT_REPO $DT_DIR
        fi
fi

sanity_check

if [[ $? -gt 0 ]]; then
       return
fi
WORKSPACE="build_$BOARD"

if [[ ! -z $OPT_DIR ]]; then
	WORKSPACE=$OPT_DIR
fi

WORKSPACE_DIR="$PROJ_DIR/../$WORKSPACE"
BUILDROOT_DIR="$PROJ_DIR/../$WORKSPACE/buildroot"
BUILD_DIR="$PROJ_DIR/../$WORKSPACE/build"
OPENSBI_DIR="$BR2_EXTERNAL_DIR/boards/efinix/$BOARD/opensbi"

if [[ $RECONFIGURE == 1 ]]; then
	# reconfigure the .config

	if [[ -d "$BUILD_DIR" ]]; then
		if [[ $RECONFIGURE_ALL == 1 ]]; then
			cp $SOC_H $OPENSBI_DIR/soc.h
			SOC_H=$OPENSBI_DIR/soc.h

			check_smp
			modify_soc_h
			generate_device_tree
		fi
		cd $BUILD_DIR && \
		make O=$PWD BR2_EXTERNAL=$BR2_EXTERNAL_DIR -C $BUILDROOT_DIR $BUILDROOT_DEFCONFIG 
	else
		echo "Error: $BUILD_DIR not exist"
	fi
	return
fi

mkdir -p $WORKSPACE_DIR

substr="buildroot"
get_version $substr
buildroot_version=$version

# clone buildroot repository
if [[ ! -z $buildroot_version ]]; then
	echo Using Buildroot $buildroot_version
	git clone $BUILDROOT_REPO -b $buildroot_version
else
	echo Using Buildroot master/main branch
	git clone $BUILDROOT_REPO
fi

mv buildroot $BUILDROOT_DIR

# apply out of tree patches for buildroot
cd $BUILDROOT_DIR && \
git am $BR2_EXTERNAL_DIR/patches/buildroot/$buildroot_version/*patch && \
cd -

# copy soc.h to opensbi directory. Opensbi has dependency on soc.h.
cp $SOC_H $OPENSBI_DIR/soc.h
SOC_H=$OPENSBI_DIR/soc.h

check_smp
modify_soc_h
generate_device_tree
prepare_buildroot_env
