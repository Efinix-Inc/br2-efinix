#!/bin/bash

# $1 = development board i.e. t120f324, ti60f225
# $2 = path to Efinity project
# $3 = path to RISC-V SDK

DEVKIT=$1
EFINITY_PROJECT=$2
RISCV_IDE=$3

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
BR2_DIR=${SCRIPT_DIR//"/boards/efinix/common"}

function get_column()
{
	# Read the VERSION file to get version
	local column="$1"
	local substr="$2"
	local arr=()
	while IFS= read -r line; do
		if [[ $line == *$substr* ]]; then
			IFS=' ' read -r -a arr <<< "$line"
		fi
	done < $BR2_DIR/VERSION
	echo ${arr[$column]}
}

function get_repo_url()
{
	local substr="$1"
	repo_url=$(get_column "0" "$substr")
	echo $repo_url
}

function get_version()
{
	local substr="$1"
	version=$(get_column "1" "$substr")
	echo $version
}

function usage()
{
	echo
	echo "This script used to modify and compile the first stage bootloader program for booting Linux."
	echo
	echo "usage: $0 Devkit Project IDE [ -e][-d]"
	echo "Positional arguments:"
	echo "	Devkit		Supported devkits are t120f324, ti60f225, ti180j484, ti375c529"
	echo "	Project		Full path of Efinity project directory. For example, $HOME/soc"
	echo "	IDE		RISCV IDE installation directory. For example,"
	echo "			$HOME/efinity/efinity-riscv-ide-2022.2.3"
	echo
	echo "Optional arguments:"
	echo "	-d		Show debug message"
	echo "	-e		Use example design project of Sapphire SoC."
	echo "			It will use the Efinity project from"
	echo "			Project/ip/<sapphire soc>/<Devkit>_devkit/"
	echo "	-h		Show this message"
	echo
	echo "Example,"
	echo "Modify bootloader example design of T120F324"
	echo "$0 t120f324 $HOME/soc $HOME/efinity/efinity-riscv-ide-2022.2.3 -e"
	exit
}

while [ $# -gt 0 ]; do
        case $1 in
                -*)
		break
                ;;
        esac
        shift
done

while getopts ":deh" o; do
        case "${o}" in
        :)
                echo "ERROR: Option -$OPTARG requires an argument"
                ;;
	d)
		DEBUG=1
		;;
	e)
		EXAMPLE_DESIGN=1
		;;
        h)
                usage
                ;;
        \?)
                echo "ERROR: Invalid option -$OPTARG"
                usage
                ;;
        esac
done
shift $((OPTIND-1))

if [[ -z $DEVKIT ]]; then
	echo ERROR: Devkit is not set
	usage
fi

if [[ -z $EFINITY_PROJECT ]]; then
	echo ERROR: Efinity project is not set
	usage
fi

if [[ -z $RISCV_IDE ]]; then
	echo ERROR: RISCV_IDE is not set
	usage
fi

DEVKIT=$(echo $DEVKIT | tr '[:upper:]' '[:lower:]')
JSON_FILE="sapphire-soc-dt-generator/config/default.json"

if [ ! -f $JSON_FILE ]; then
# clone sapphire-soc-dt-generator repository
	dt_name="sapphire-soc-dt-generator"
	DT_REPO=$(get_repo_url "$dt_name")
	dt_version=$(get_version "$dt_name")
	DT_DIR="$SCRIPT_DIR/$dt_name"

        if [ ! -z $dt_version ]; then
                git clone $DT_REPO -b $dt_version $DT_DIR
        else
                git clone $DT_REPO $DT_DIR
        fi
fi

DEVKITS=$(jq '.devkits.Titanium, .devkits.Trion | .[]' $JSON_FILE)

found=false
for devkit in ${DEVKITS[@]}; do
	devkit_l=$(echo $devkit | tr '[:upper:]' '[:lower:]')
	if [[ $devkit_l == *$DEVKIT* ]]; then
		found=true
		DEVKIT=$(echo $devkit | tr -d \")
		DEVKIT="${DEVKIT}_devkit"
		break
	fi
done

if [[ $found == false ]]; then
	echo "ERROR: Devkit $DEVKIT is not supported"
	exit 1
fi

echo INFO: Checking RISC-V Toolchain
RISCV_TOOLCHAIN_DIR="toolchain/bin"

toolchain=$(find "$RISCV_IDE/$RISCV_TOOLCHAIN_DIR" -name *-gcc)
$toolchain -dumpversion > /dev/null 2>&1
if [[ $? -eq 0 ]]; then
	export PATH=$RISCV_IDE/$RISCV_TOOLCHAIN_DIR:$PATH
	echo "INFO: Found toolchain in $toolchain"
else
	echo "ERROR: $RISCV_IDE path is invalid"
	exit 1
fi

if [[ "$EFINITY_PROJECT" != /* ]]; then
	EFINITY_PROJECT=$(realpath $EFINITY_PROJECT)
fi

EFINITY_PROJECT_DIR="${EFINITY_PROJECT}"

if [[ $EXAMPLE_DESIGN ]]; then
	DEVKIT_DIR=$DEVKIT
	EFINITY_PROJECT_DIR="$(find "${EFINITY_PROJECT}" -type d -name "${DEVKIT_DIR}")"
fi

EMBEDDED_SW_DIR="${EFINITY_PROJECT_DIR}/embedded_sw"
mapfile -t BSP_DIRS < <(find "${EMBEDDED_SW_DIR}" -type d -name bsp)

echo "INFO: Found ${#BSP_DIRS[@]} BSP directories"
if [ ${#BSP_DIRS[@]} -gt 1 ]; then
	for i in "${!BSP_DIRS[@]}"; do
		echo "$i: ${BSP_DIRS[$i]}"
	done

	# Prompt user for input
	read -p "Select the BSP directory by the number: " selection

	# Validate and use the selection
	if [[ "$selection" =~ ^[0-9]+$ ]] && [ "$selection" -ge 0 ] && [ "$selection" -lt "${#BSP_DIRS[@]}" ]; then
	    BSP_DIR="${BSP_DIRS[$selection]}"
	else
	    echo "Invalid selection."
	    exit 1
	fi
else
	BSP_DIR="${BSP_DIRS[0]}"
fi

echo "INFO: BSP_DIR = $BSP_DIR"
PROJ_DIR=$(dirname "$BSP_DIR")

STANDALONE_DIR="$(find "${PROJ_DIR}" -type d -name standalone)"
if [ -z "$STANDALONE_DIR" ]; then
	echo "ERROR: standalone directory is not found in the current BSP"
	exit 1
fi
BOOTLOADER_DIR="$(find "${STANDALONE_DIR}" -type d -name bootloader)"
BOOTLOADERCONFIG="$(find "${EMBEDDED_SW_DIR}" -type f -name bootloaderConfig.h)"
SOC_H="$(find "${EMBEDDED_SW_DIR}" -type f -name soc.h)"
SOC_MK="$(find "${EMBEDDED_SW_DIR}" -type f -name soc.mk)"
EFXSAPPHIRESOC_DIR="$BSP_DIR/efinix/EfxSapphireSoc"

if [ $DEBUG ]; then
	echo EFINITY_PROJECT = $EFINITY_PROJECT
	echo EFINITY_PROJECT_DIR = $EFINITY_PROJECT_DIR
	echo EMBEDDED_SW_DIR = $EMBEDDED_SW_DIR
	echo BSP_DIR = $BSP_DIR
	echo STANDALONE_DIR = $STANDALONE_DIR
	echo BOOTLOADER_DIR = $BOOTLOADER_DIR
	echo BOOTLOADERCONFIG = $BOOTLOADERCONFIG
	echo SOC_H = $SOC_H
	echo EFXSAPPHIRESOC_DIR = $EFXSAPPHIRESOC_DIR
fi

echo INFO: Update Bootloader Program
cp $SCRIPT_DIR/bootloaderConfig.h $BOOTLOADERCONFIG

echo INFO: Check for SMP Flag
if grep -q SYSTEM_PLIC_SYSTEM_CORES_1_EXTERNAL_INTERRUPT $SOC_H; then
	echo INFO: Enable SMP Flag
	sed -i 's/^#CFLAGS+=-DSMP/CFLAGS+=-DSMP/g' $SOC_MK
else
	echo INFO: Disabled SMP Flag
	sed -i 's/CFLAGS+=-DSMP/#CFLAGS+=-DSMP/g' $SOC_MK
fi

echo INFO: Disabled Debug Flags
sed -i 's/DEBUG?=yes/DEBUG?=no/g' $SOC_MK
sed -i 's/DEBUG_OG?=yes/DEBUG_OG?=no/g' $SOC_MK

echo INFO: Compiling Linux Bootloader for Sapphire SoC
cd $BOOTLOADER_DIR && \
	BSP_PATH=$EFXSAPPHIRESOC_DIR make clean && \
	BSP_PATH=$EFXSAPPHIRESOC_DIR make

	if [ ! $? -eq 0 ]; then
		echo ERROR: Compilation failed
		cd - > /dev/null
		exit 1
	fi
cd - > /dev/null

cp -rf $BOOTLOADER_DIR/build $EFINITY_PROJECT_DIR/linux_bootloader
echo INFO: The bootloader is in $EFINITY_PROJECT_DIR/linux_bootloader/bootloader.hex
