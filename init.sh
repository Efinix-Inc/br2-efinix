#!/bin/bash -e

# args1: board
# args2: path/to/soc.h

OPTIND=1
unset RECONFIGURE
unset RECONFIGURE_ALL
unset OPT_DIR
unset ETHERNET
unset HARDEN_SOC
unset UNIFIED_HW
unset EXTRA_HW_FEATURES

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
	echo "Usage: init.sh [board <t120f324|ti60f225>] [path/to/soc.h <string> ] [-d build directory <string>] [-r reconfigure]"
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
	echo "	-u			Generate Linux device tree for unified hardware design for Ti180J484 and Ti375C529"
	echo "	-s			Set hardware features to enable in the Linux kernel. Must be in comma seperated."
	echo "				For example, spi,i2c,gpio,ethernet,dma,framebuffer"
	echo "	-x			Enable X11 graphics for unified hardware design. This enable framebuffer, DMA and USB drivers."
	echo "				Not compatible with camera (evsoc driver). This optional argument requires -u to be set first."
	echo
	echo "Example usage,"
	echo "$	source init.sh t120f324 ~/efinity/2022.1/project/soc/ip/soc1/T120F324_devkit/embedded_sw/soc1/bsp/efinix/EfxSapphireSoc/include/soc.h"
	echo
	echo "Demo Ti180J484 with unified hardware design"
	echo "$ source init.sh ti180j484 /path/to/soc.h -u"
	echo
	echo "Demo Ti375C529 with unified hardware design"
	echo "$ source init.sh ti375c529 /path/to/soc.h -p -u"
	echo
	echo "Demo Ti375c529 with unified hardware design + X11 graphics"
	echo "$ source init.sh ti375c529 /path/to/soc.h -u -x"
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

	devkits=$(jq '.devkits.Titanium, .devkits.Trion | .[]' $JSON_FILE)

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

	if [[ $HARDEN_SOC ]] && [[ ! $BOARD == "ti375c529" ]]; then
		echo Error: board $BOARD does not support harden SoC.
		echo "The -p optional argument only used for board which support harden SoC such as Ti375C529"
		return 1;
	fi

	if [[ $UNIFIED_HW ]]; then
		if [[ $BOARD == "ti375c529" || $BOARD == "ti180j484" ]]; then
			echo Info: board $BOARD support unified hardware design
		else
			echo Error: board $BOARD does not support unified hardware design
			return 1;
		fi
	fi

	if [ "$(grep SYSTEM_HARD_RISCV_QC32 $SOC_H | awk '{print $3}')" == "1" ]; then
		HARDEN_SOC=1
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
		echo "INFO: Enable compressed extension (RVC) in $BR2_EXTERNAL_DIR/configs/$BUILDROOT_DEFCONFIG"
		sed -i 's/BR2_RISCV_ISA_CUSTOM_RVC=n/BR2_RISCV_ISA_CUSTOM_RVC=y/g' $BR2_EXTERNAL_DIR/configs/$BUILDROOT_DEFCONFIG
		sed -i 's/CONFIG_RISCV_ISA_C=n/CONFIG_RISCV_ISA_C=y/g' $BR2_EXTERNAL_DIR/boards/efinix/$BOARD/linux/linux.config
	fi

	# change the size of DDR to 1024MB due to limitation of physical DDR.
	if [ $HARDEN_SOC ]; then
		sed -i 's/0xe7fff000/0x40000000/g' $SOC_H
	else #Soft Core
		sed -i 's/0xe0000000/0x40000000/g' $SOC_H
	fi
}

function generate_device_tree()
{
	local linux_slaves="$DT_DIR/config/linux_slaves.json"
	local base_cmd="python3 $DT_DIR/device_tree_generator.py "
	local end_cmd="$SOC_H $BOARD linux"
	local spi="-c $DT_DIR/config/linux_spi.json "
	local ethernet="-c $DT_DIR/config/ethernet.json "
	local ethernet_ed="-c $DT_DIR/config/ed_ti375c529.json "
	local sdhc="-c $DT_DIR/config/sdhc.json "
	local unified_hw="-c $DT_DIR/config/unified_hw.json "
	local unified_hw_softcore="-c $DT_DIR/config/unified_hw_softcore.json "
	local x11_graphics="-c $DT_DIR/config/framebuffer.json "

	if [ $ETHERNET ]; then
		if [ $HARDEN_SOC ]; then
			base_cmd+=$ethernet_ed
		else
			base_cmd+=$ethernet
		fi
	fi

	if [ $SDHC ]; then
		base_cmd+=$sdhc
	else
		base_cmd+=$spi
	fi

	if [ $HARDEN_SOC ]; then
		if [ ! -f "$EFINIX_DIR/$BOARD/u-boot/uboot.dts.spi" ]; then
			cp $EFINIX_DIR/$BOARD/u-boot/uboot.dts $EFINIX_DIR/$BOARD/u-boot/uboot.dts.spi
			cp $EFINIX_DIR/$BOARD/u-boot/uboot.dts.mmc $EFINIX_DIR/$BOARD/u-boot/uboot.dts
		fi
	else
		if [ -f "$EFINIX_DIR/$BOARD/u-boot/uboot.dts.spi" ]; then
			mv $EFINIX_DIR/$BOARD/u-boot/uboot.dts.spi $EFINIX_DIR/$BOARD/u-boot/uboot.dts
		fi
	fi

	if [ $UNIFIED_HW ]; then
		if [ $HARDEN_SOC ] || [ "$BOARD" == "ti180j484" ]; then
			#Copy the custom soc.h for evsoc kernel
			cp "$SOC_H" "$PROJ_DIR/kernel_modules/evsoc/src/soc.h"
			echo "Copy $SOC_H to $PROJ_DIR/kernel_modules/evsoc/src/soc.h"

			# Create a temp for linux_slave json file to disable spi1
			linux_slaves="$DT_DIR/config/linux_slaves_modified.json"
			jq '.child.spi_mmc.status = "disabled"' "$DT_DIR/config/linux_slaves.json" > "$linux_slaves"

			if [ $X11_GRAPHICS ]; then
				echo INFO: Enable X11 graphics
				base_cmd+=$x11_graphics
			fi
		fi

		if [ $HARDEN_SOC ]; then
			base_cmd+=$unified_hw
		elif [ "$BOARD" == "ti180j484" ]; then
			base_cmd+=$unified_hw_softcore
			cp $EFINIX_DIR/$BOARD/u-boot/uboot.dts $EFINIX_DIR/$BOARD/u-boot/uboot.dts.spi
			cp $EFINIX_DIR/$BOARD/u-boot/uboot.dts.mmc $EFINIX_DIR/$BOARD/u-boot/uboot.dts
		else
			echo "Error: Unified hardware not support for $BOARD"
			return
		fi
	fi

	base_cmd+="-s $linux_slaves "
	base_cmd+=$end_cmd
	echo DEBUG: device tree cmd: $base_cmd
	eval $base_cmd

	if [ ! $? -eq 0 ]; then
		echo "Error: Failed to generate device tree."
		return 1
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

function set_kernel_config()
{
	local kernel_frag_dir="\$(BR2_EXTERNAL_EFINIX_PATH)/boards/efinix/common/kernel"
	local hw_features=""
	local br2_defconfig="configs/efinix_${BOARD}_defconfig"
	local br2_kernel_cfg_keyword="BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES"
	local br2_linux_kernel_cfg=""
	local feature

	# Read from SOC_H to get the hardware features
	local base_features=("spi" "i2c" "gpio")

	for feature in "${base_features[@]}"; do
		if grep -i -q $feature $SOC_H; then
			echo INFO: hardware feature: $feature
			br2_linux_kernel_cfg+=" $kernel_frag_dir/$feature.config"
		fi
	done

	if [ ! -z ${EXTRA_HW_FEATURES} ]; then
		IFS=',' read -ra hw_features <<< "${EXTRA_HW_FEATURES}"

		for feature in "${hw_features[@]}"; do
			echo INFO: hardware feature: $feature

			[ "$feature" = "spi" ] && br2_linux_kernel_cfg+=" $kernel_frag_dir/spi.config"
			[ "$feature" = "i2c" ] && br2_linux_kernel_cfg+=" $kernel_frag_dir/i2c.config"
			[ "$feature" = "gpio" ] && br2_linux_kernel_cfg+=" $kernel_frag_dir/gpio.config"
			[ "$feature" = "mmc" ] && br2_linux_kernel_cfg+=" $kernel_frag_dir/mmc.config"
			[ "$feature" = "ethernet" ] && br2_linux_kernel_cfg+=" $kernel_frag_dir/ethernet.config"
			[ "$feature" = "dma" ] && br2_linux_kernel_cfg+=" $kernel_frag_dir/dma.config"
			[ "$feature" = "fb" ] && br2_linux_kernel_cfg+=" $kernel_frag_dir/framebuffer.config"
			[ "$feature" = "usb" ] && br2_linux_kernel_cfg+=" $kernel_frag_dir/usb.config"
		done
	fi

	echo INFO: $br2_kernel_cfg_keyword=\"$br2_linux_kernel_cfg\"
	if grep -q ${br2_kernel_cfg_keyword} ${br2_defconfig}; then
		sed -i "/$br2_kernel_cfg_keyword/c\\$br2_kernel_cfg_keyword=\"$br2_linux_kernel_cfg\"" "${br2_defconfig}"
	else
		echo "$br2_kernel_cfg_keyword=\"$br2_linux_kernel_cfg\"" >> "${br2_defconfig}"
	fi
}

while [ $# -gt 0 ]
do
	case $1 in
		-*) break
		;;
	esac
	shift
done

while getopts ":d:s:raehux" o; do
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
		u)
			UNIFIED_HW=1
			EXTRA_HW_FEATURES="mmc,ethernet,"
			;;
		s)
			EXTRA_HW_FEATURES=${OPTARG}
			;;
		x)
			if [ "$UNIFIED_HW" = "1" ]; then
				X11_GRAPHICS=1
				EXTRA_HW_FEATURES+="fb,dma,usb,"
			else
				echo "ERROR: -x option requires -u to be set first."
				return
			fi
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

	if [ ! $? -eq 0 ]; then
		echo Error: Failed to clone $DT_REPO
		echo Check you internet connection
		return 1
	fi
fi

sanity_check

if [[ $? -gt 0 ]]; then
       return
fi

set_kernel_config

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

			check_smp || return 1
			modify_soc_h || return 1
			generate_device_tree || return 1
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

if [ ! -d $BUILDROOT_DIR ]; then
	# clone buildroot repository
	if [[ ! -z $buildroot_version ]]; then
		echo Using Buildroot $buildroot_version
		git clone $BUILDROOT_REPO -b $buildroot_version
	else
		echo Using Buildroot master/main branch
		git clone $BUILDROOT_REPO
	fi

	if [ ! $? -eq 0 ]; then
		echo Error: Failed to clone $BUILDROOT_REPO
		echo Check you internet connection
		return 1
	fi

	mv buildroot $BUILDROOT_DIR

	# apply out of tree patches for buildroot
	git -C $BUILDROOT_DIR reset --hard $buildroot_version
	git -C $BUILDROOT_DIR am $BR2_EXTERNAL_DIR/patches/buildroot/$buildroot_version/*patch
fi || return 1

# copy soc.h to opensbi directory. Opensbi has dependency on soc.h.
cp $PROJ_DIR/$SOC_H $OPENSBI_DIR/soc.h
SOC_H=$OPENSBI_DIR/soc.h

check_smp || return 1
modify_soc_h || return 1
generate_device_tree || return 1
prepare_buildroot_env || return 1
