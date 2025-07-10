#!/bin/bash -e

# args1: board
# args2: path/to/soc.h

OPTIND=1
unset RECONFIGURE
unset RECONFIGURE_ALL
unset OPT_DIR
unset EXAMPLE_DESIGN
unset HARDEN_SOC
unset UNIFIED_HW
unset EXTRA_HW_FEATURES
unset X11_GRAPHICS

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

# Text colors
WHITE='\033[0;37m'
BLACK='\033[0;30m'
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'

# Background colors
BG_WHITE='\033[47m'
BG_BLACK='\033[40m'
BG_RED='\033[41m'
BG_GREEN='\033[42m'
BG_YELLOW='\033[43m'
BG_BLUE='\033[44m'
NC='\033[0m'	# No color

# Get the terminal width
COLUMNS=$(tput cols)

function title()
{
	# Print combined text and background colors
	echo -e "\n${BLACK}${BG_WHITE}>>> $1 ${NC}"
}

function self_check()
{
	if [ $2 -eq 0 ]; then
		echo -e "Check: $1 ${RED}NO${NC}"
	else
		echo -e "Check: $1 ${GREEN}YES${NC}"
	fi
}

function usage()
{
	echo "Usage: init.sh [board] [path/to/soc.h] [-c][-d][-r][-a][-e][-u][-s][-x]"
	echo
	echo "Positional arguments:"
	echo "	board			Development kit name such as t120f324, ti60f225, ti180j484, ti375c529"
	echo "	soc			Path to 'soc.h'. This file is located in Efinity project directory."
	echo "				For example,"
	echo "				my_project/embedded_sw/SapphireCore/bsp/efinix/EfxSapphireSoc/include/soc.h"
	echo "				my_project/embedded_sw/efx_hard_soc/bsp/efinix/EfxSapphireSoc/include/soc.h"
	echo
	echo "Optional arguments:"
	echo "	-h			Show this help message and exit"
	echo "	-c			Reset the repo to the original state"
	echo "	-d			Rename default build directory name"
	echo "				By default is <board>_build"
	echo "				Example, if board is ti60f225 then, the name of build directory is 'ti60f225_build'."
	echo "	-r			Reconfigure the Buildroot configuration. This option will not regenerate device tree."
	echo "	-a			Reconfigure the Buildroot configuration and regenerate Linux device tree."
	echo "	-e                      Generate Linux devcie tree for SoC example design."
	echo "	-u			Generate Linux device tree for unified hardware design for Ti180J484 and Ti375C529"
	echo "	-s			Set hardware features to enable in the Linux kernel. Must be in comma seperated."
	echo "				For example, spi,i2c,gpio,ethernet,dma,framebuffer"
	echo "	-x			Enable X11 graphics for unified hardware design. This enable framebuffer, DMA and USB drivers."
	echo "				Not compatible with camera (evsoc driver). This optional argument requires -u to be set first."
	echo
	echo "Example usage,"
	echo "$	source init.sh t120f324 ~/efinity/2022.1/project/soc/ip/soc1/T120F324_devkit/embedded_sw/soc1/bsp/efinix/EfxSapphireSoc/include/soc.h"
	echo
	echo "Demo Ti60F225 with ethernet example design"
	echo "$ source init.sh ti60f225 $(pwd)/boards/efinix/ti60f225/hardware/ethernet/soc.h -e"
	echo
	echo "Demo Ti180J484 with singlecore example design"
	echo "$ source init.sh ti180j484 $(pwd)/boards/efinix/ti180j484/hardware/singlecore/soc.h"
	echo
	echo "Demo Ti180J484 with multicores example design"
	echo "$ source init.sh ti180j484 $(pwd)/boards/efinix/ti180j484/hardware/multicores/soc.h"
	echo
	echo "Demo Ti180J484 with unified hardware design"
	echo "$ source init.sh ti180j484 $(pwd)/boards/efinix/ti180j484/hardware/unified_hw/soc.h -u"
	echo
	echo "Demo Ti375C529 soc example design"
	echo "$ source init.sh ti375c529 $(pwd)/boards/efinix/ti375c529/hardware/soc/soc.h -e"
	echo
	echo "Demo Ti375C529 with unified hardware design"
	echo "$ source init.sh ti375c529 $(pwd)/boards/efinix/ti375c529/hardware/unified_hw/soc.h -u"
	echo
	echo "Demo Ti375c529 with unified hardware design + X11 graphics"
	echo "$ source init.sh ti375c529 $(pwd)/boards/efinix/ti375c529/hardware/unified_hw/soc.h -u -x"
}

function sanity_check()
{
	title "Sanity Check"

	# check the compatible of $BOARD in $JSON_FILE
	local found=false
	local devkit_l
	BOARD=$(echo $BOARD | tr '[:upper:]' '[:lower:]')

	devkits=$(jq '.devkits.Titanium, .devkits.Trion | .[]' $JSON_FILE)

	for devkit in ${devkits[@]}; do
		devkit_l=$(echo $devkit | tr '[:upper:]' '[:lower:]'| sed 's/^"//;s/"$//')
		if [[ "$devkit_l" == "$BOARD" ]]; then
			BOARD=$(echo $devkit_l | tr -d \")
			BUILDROOT_DEFCONFIG="efinix_"$BOARD"_defconfig"
			found=true
			break
		fi
	done

	if [[ $found == false ]]; then
		echo ERROR: board $BOARD is not supported
		return 1
	fi

	if [[ $UNIFIED_HW ]]; then
		if [[ $BOARD == "ti375c529" || $BOARD == "ti180j484" || $BOARD == "ti375n1156" ]]; then
			echo INFO: board $BOARD support unified hardware design
		else
			echo ERROR: board $BOARD does not support unified hardware design
			return 1;
		fi
	fi

	if [ "$(grep SYSTEM_HARD_RISCV_QC32 $SOC_H | awk '{print $3}')" == "1" ]; then
		HARDEN_SOC=1
	fi
}

function check_soc_configuration()
{
	local smp_flag
	title "Checking Efinix RISC-V Sapphire SoC Configuration"

	get_cpu_count
        echo "INFO: Detecting $cpu_count RISC-V CPU cores"
        if [[ $cpu_count -gt 1 ]]; then
                # enable CONFIG_SMP=y in linux.config
                sed -i 's/^CONFIG_SMP=n/CONFIG_SMP=y/g' $BR2_EXTERNAL_DIR/boards/efinix/$BOARD/linux/linux.config
		smp_flag=1
        else
                sed -i 's/^CONFIG_SMP=y/CONFIG_SMP=n/g' $BR2_EXTERNAL_DIR/boards/efinix/$BOARD/linux/linux.config
		smp_flag=0
        fi
	self_check "Enable SMP support ..." "$smp_flag"

	# modify soc.h
	# remove last line '#endif'
	sed '/#endif/d' $SOC_H > ${SOC_H}.temp
	mv ${SOC_H}.temp $SOC_H

	# append SYSTEM_CORES_COUNT
	grep -q SYSTEM_CORES_COUNT $SOC_H || echo "#define SYSTEM_CORES_COUNT $cpu_count" >> $SOC_H

	# append addresses for AXI interconnect to soc.h
	if [ $UNIFIED_HW ] || [ $EXAMPLE_DESIGN ]; then

		sed -i '/SYSTEM_AXI_A_BMB/d' $SOC_H
		echo INFO: Append addresses for AXI interconnect
		if [[ $BOARD = "ti375c529" || $BOARD = "ti375n1156" ]]; then
			grep -q SYSTEM_AXI_SLAVE $SOC_H || \
			cat <<-EOF >> $SOC_H
			#define SYSTEM_AXI_SLAVE_0_IO_CTRL 0xe8000000
			#define SYSTEM_AXI_SLAVE_0_IO_CTRL_SIZE 0x1000000
			#define SYSTEM_AXI_SLAVE_1_IO_CTRL 0xe9000000
			#define SYSTEM_AXI_SLAVE_1_IO_CTRL_SIZE 0x10000
			#define SYSTEM_AXI_SLAVE_2_IO_CTRL 0xe9100000
			#define SYSTEM_AXI_SLAVE_2_IO_CTRL_SIZE 0x10000
			#define SYSTEM_AXI_SLAVE_3_IO_CTRL 0xe9200000
			#define SYSTEM_AXI_SLAVE_3_io_CTRL_SIZE 0x10000
			#define SYSTEM_AXI_A_BMB 0xe8000000
			#define SYSTEM_AXI_A_BMB_SIZE 0x1000000
			#define SYSTEM_AXI_B_BMB 0xe9000000
			#define SYSTEM_AXI_B_BMB_SIZE 0x10000
			#define SYSTEM_AXI_C_BMB 0xe9100000
			#define SYSTEM_AXI_C_BMB_SIZE 0x10000
			#define SYSTEM_AXI_D_BMB 0xe9200000
			#define SYSTEM_AXI_D_BMB_SIZE 0x10000
EOF
		elif [ $BOARD = "ti180j484" ]; then
			grep -q SYSTEM_AXI_SLAVE $SOC_H || \
			cat <<-EOF >> $SOC_H
			#define SYSTEM_AXI_SLAVE_0_IO_CTRL 0xe1000000
			#define SYSTEM_AXI_SLAVE_0_IO_CTRL_SIZE 0x1000000
			#define SYSTEM_AXI_SLAVE_1_IO_CTRL 0xe1800000
			#define SYSTEM_AXI_SLAVE_1_IO_CTRL_SIZE 0x10000
			#define SYSTEM_AXI_SLAVE_2_IO_CTRL 0xe1810000
			#define SYSTEM_AXI_SLAVE_2_IO_CTRL_SIZE 0x10000
			#define SYSTEM_AXI_A_BMB 0xe1000000
			#define SYSTEM_AXI_A_BMB_SIZE 0x1000000
			#define SYSTEM_AXI_B_BMB 0xe1800000
			#define SYSTEM_AXI_B_BMB_SIZE 0x10000
			#define SYSTEM_AXI_C_BMB 0xe1810000
			#define SYSTEM_AXI_C_BMB_SIZE 0x10000
		EOF
		fi
	fi

	# append '#endif' to soc.h
	grep -q "#endif" $SOC_H || echo "#endif" >> $SOC_H

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
	self_check "Floating point support ..." "$fp"

	# check for compressed extension from soc.h
	ext_c=$(cat ${SOC_H} | grep SYSTEM_RISCV_ISA_EXT_C | awk '{print $3}' | head -1)
	if [ $ext_c == 1 ]; then
		# enable compressed extension flag in buildroot defconfig
		echo "INFO: Enable compressed extension (RVC) in $BR2_EXTERNAL_DIR/configs/$BUILDROOT_DEFCONFIG"
		sed -i 's/BR2_RISCV_ISA_CUSTOM_RVC=n/BR2_RISCV_ISA_CUSTOM_RVC=y/g' $BR2_EXTERNAL_DIR/configs/$BUILDROOT_DEFCONFIG
		sed -i 's/CONFIG_RISCV_ISA_C=n/CONFIG_RISCV_ISA_C=y/g' $BR2_EXTERNAL_DIR/boards/efinix/$BOARD/linux/linux.config
	fi
	self_check "Compressed extension support ..." "$ext_c"

	# change the size of DDR to 1024MB due to limitation of physical DDR.
	ddr_size=$(grep SYSTEM_DDR_BMB_SIZE $SOC_H | awk '{print $3}')
	phy_ddr_size=0x40000000

	if (( $ddr_size > $phy_ddr_size )); then
		sed -i "s/$ddr_size/$phy_ddr_size/g" $SOC_H
	fi
}

function generate_device_tree()
{
	local linux_dt_config="$DT_DIR/config/linux"
	local uboot_dt_config="$DT_DIR/config/uboot"
	local base_cmd="python3 $DT_DIR/device_tree_generator.py "
	local end_cmd="$SOC_H $BOARD"

	local uboot_spi0="-s $uboot_dt_config/spi0.json "
	local uboot_spi1="-s $uboot_dt_config/spi1.json "
	local uboot_mmc="-c $uboot_dt_config/mmc.json "

	local linux_slaves="-s $linux_dt_config/slaves.json "
	local spi="-c $linux_dt_config/spi.json "
	local spi_mmc="-s $linux_dt_config/spi_mmc.json "
	local ethernet="-c $linux_dt_config/ethernet.json "
	local ti375_common="-c $linux_dt_config/ti375_common.json "
	local unified_hw="-c $linux_dt_config/unified_hw.json "
	local unified_hw_softcore="-c $linux_dt_config/unified_hw_softcore.json "
	local evsoc="-c $linux_dt_config/evsoc.json "
	local x11_graphics="-c $linux_dt_config/framebuffer.json "

	local linux_dt+=$base_cmd
	local uboot_dt+=$base_cmd

	title "Generate U-Boot Device Tree"
	uboot_dt+=$uboot_spi0

	if [ $UNIFIED_HW ] || [ $EXAMPLE_DESIGN ]; then
		if [ "$BOARD" = "ti60f225" ]; then
			uboot_dt+=$uboot_spi1
		else
			uboot_dt+=$uboot_mmc
		fi
	else
		uboot_dt+=$uboot_spi1
	fi

	uboot_dt+="-d $DT_DIR/uboot_dts $end_cmd uboot"
	echo DEBUG: uboot dts: $uboot_dt
	eval $uboot_dt

	if [ ! $? -eq 0 ]; then
                echo "ERROR: Failed to generate U-Boot device tree."
                return 1
        fi

	cp $DT_DIR/uboot_dts/uboot.dts $EFINIX_DIR/$BOARD/u-boot/uboot.dts

	title "Generate Linux Device Tree"
	linux_dt+=$spi

	if [ $EXAMPLE_DESIGN ]; then
		if [ $HARDEN_SOC ]; then
			linux_dt+=$ti375_common
		else
			linux_dt+=$ethernet
		fi
	fi

	if [ $UNIFIED_HW ]; then
		if [ $HARDEN_SOC ] || [ "$BOARD" == "ti180j484" ]; then
			if [ $X11_GRAPHICS ]; then
				echo INFO: Enable X11 graphics
				linux_dt+=$x11_graphics

			else
				linux_dt+=$evsoc
				#Copy the custom soc.h for evsoc kernel
				cp "$SOC_H" "$PROJ_DIR/kernel_modules/evsoc/src/soc.h"
				echo "INFO: Copy $SOC_H to $PROJ_DIR/kernel_modules/evsoc/src/soc.h"
			fi
		fi

		if [ $HARDEN_SOC ]; then
			linux_dt+=$unified_hw
		elif [ "$BOARD" == "ti180j484" ]; then
			linux_dt+=$unified_hw_softcore
		else
			echo "ERROR: Unified hardware not support for $BOARD"
			return
		fi

	else
		linux_dt+=$spi_mmc
	fi

	linux_dt+=$linux_slaves
	linux_dt+="$end_cmd linux"
	echo DEBUG: device tree cmd: $linux_dt
	eval $linux_dt

	if [ ! $? -eq 0 ]; then
		echo "ERROR: Failed to generate Linux device tree."
		return 1
	fi

	cp $DT_DIR/dts/sapphire.dtsi $COMMON_DIR/dts/sapphire.dtsi
	cp $DT_DIR/dts/linux.dts $EFINIX_DIR/$BOARD/linux/linux.dts
}

function prepare_buildroot_env()
{
	# prepare Buildroot build environment
	title "Prepare Buildroot Build Environment"
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

	title "Set Kernel Configuration"

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

function add_packages()
{
        title "Add Packages"
	local br2_defconfig_path="$BR2_EXTERNAL_DIR/configs"
        local br2_defconfig="$BR2_EXTERNAL_DIR/configs/$BUILDROOT_DEFCONFIG"

        if [ $UNIFIED_HW ]; then
		grep -q EVSOC $br2_defconfig || \
		cat $br2_defconfig_path/evsoc_fragment >> $br2_defconfig
	fi

        if [ $X11_GRAPHICS ]; then
                grep -q BR2_PACKAGE_DESKTOP_ENVIRONMENT $br2_defconfig || \
		cat $br2_defconfig_path/x11_fragment >> $br2_defconfig

		# remove EVSOC packages
		grep -v "EVSOC" $br2_defconfig > ${br2_defconfig}.temp
		mv ${br2_defconfig}.temp $br2_defconfig
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

while getopts ":d:s:raehuxc" o; do
	case "${o}" in
		:)
                        echo "ERROR: Option -$OPTARG requires an argument"
                        ;;
		c)
			mod_files=$(git status | grep modified | awk '{print $2}')
			for f in ${mod_files[@]};
			do
				git checkout -- $f
			done
			echo INFO: Repo reset to the original state
			return
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
			EXAMPLE_DESIGN=1
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
	echo ERROR: Board is not defined
	return 1
fi

if [[ -z $SOC_H ]]; then
	echo ERROR: soc.h is not defined
	return 1
elif [[ ! -f $SOC_H ]]; then
	echo ERROR: No such file for $SOC_H
	return 1
fi


# clone sapphire-soc-dt-generator repository
if [ ! -d $DT_DIR ]; then
	title "Cloning sapphire-soc-dt-generator Repository"
        substr="sapphire-soc-dt-generator"
        get_version $substr
        dt_version=$version

        if [ ! -z $dt_version ]; then
                git clone $DT_REPO -b $dt_version $DT_DIR
        else
                git clone $DT_REPO $DT_DIR
        fi

	if [ ! $? -eq 0 ]; then
		echo ERROR: Failed to clone $DT_REPO
		echo Check you internet connection
		return 1
	fi
fi

sanity_check || return 1

if [ $EXAMPLE_DESIGN ]; then
	if [ "$BOARD" = "ti60f225" ]; then
		EXTRA_HW_FEATURES+="ethernet,"
	fi

	if [[ "$BOARD" = "ti375c529" || "$BOARD" = "ti375n1156" ]]; then
		EXTRA_HW_FEATURES+="mmc,ethernet,"
	fi
fi

set_kernel_config
add_packages

WORKSPACE="build_$BOARD"

if [[ ! -z $OPT_DIR ]]; then
	WORKSPACE=$OPT_DIR
fi

WORKSPACE_DIR="$PROJ_DIR/../$WORKSPACE"
BUILDROOT_DIR="$PROJ_DIR/../$WORKSPACE/buildroot"
BUILD_DIR="$PROJ_DIR/../$WORKSPACE/build"
OPENSBI_DIR="$BR2_EXTERNAL_DIR/boards/efinix/$BOARD/opensbi"

if [[ $RECONFIGURE == 1 ]]; then
	title "Reconfigure Buildroot"
	# reconfigure the .config

	if [[ -d "$BUILD_DIR" ]]; then
		if [[ $RECONFIGURE_ALL == 1 ]]; then
			cp $SOC_H $OPENSBI_DIR/soc.h
			SOC_H=$OPENSBI_DIR/soc.h

			check_soc_configuration || return 1
			generate_device_tree || return 1
		fi
		cd $BUILD_DIR && \
		make O=$PWD BR2_EXTERNAL=$BR2_EXTERNAL_DIR -C $BUILDROOT_DIR $BUILDROOT_DEFCONFIG
		return 0
	else
		echo "ERROR: $BUILD_DIR not exist"
		return 1
	fi
fi

mkdir -p $WORKSPACE_DIR

substr="buildroot"
get_version $substr
buildroot_version=$version

if [ ! -d $BUILDROOT_DIR ]; then
	# clone buildroot repository
	title "Clone Buildroot repository"
	if [[ ! -z $buildroot_version ]]; then
		echo Using Buildroot $buildroot_version
		git clone $BUILDROOT_REPO -b $buildroot_version
	else
		echo Using Buildroot master/main branch
		git clone $BUILDROOT_REPO
	fi

	if [ ! $? -eq 0 ]; then
		echo ERROR: Failed to clone $BUILDROOT_REPO
		echo Check you internet connection
		return 1
	fi

	mv buildroot $BUILDROOT_DIR

	title "Apply Patch"
	# apply out of tree patches for buildroot
	git -C $BUILDROOT_DIR reset --hard $buildroot_version
	git -C $BUILDROOT_DIR am $BR2_EXTERNAL_DIR/patches/buildroot/$buildroot_version/*patch
fi || return 1

title "Copy soc.h file to OpenSBI directory"
# copy soc.h to opensbi directory. Opensbi has dependency on soc.h.
echo "INFO: copy $SOC_H to $OPENSBI_DIR/soc.h"
cp $SOC_H $OPENSBI_DIR/soc.h
SOC_H=$OPENSBI_DIR/soc.h

check_soc_configuration || return 1
generate_device_tree || return 1
prepare_buildroot_env || return 1
