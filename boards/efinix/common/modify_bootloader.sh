#!/bin/bash

# $1 = development board i.e. t120f324, ti60f225
# $2 = path to Efinity project
# $3 = path to RISC-V SDK
# $4 = efinity installation directory

DEVKIT=$1
EFINITY_PROJECT=$2
RISCV_IDE=$3
EFINITY_HOME=$4

JSON_FILE="devkits.json"
DEVKITS=$(jq '.devkits | .[]' $JSON_FILE)

function usage()
{
	echo "This script used to modify bootloader program for booting Linux."
	echo
	echo "command"
	echo "$0 <devkit> <efinity project directory> <RISCV IDE directory> <Efinity installation directory>"
	echo
	echo "supported <devkit> are t120f324, ti60f225"
	echo "<Efinity project directory> can be $HOME/soc/ip/soc1"
	echo "<RISCV IDE> can be $HOME/efinity/efinity-riscv-ide-2022.2.3"
	echo "<Efinity installation directory> can be $HOME/efinity/2022.1/bin"
	echo
	echo "Example command for t120f324 devkit"
	echo "$0 t120f324 $HOME/soc/ip/soc1 $HOME/efinity/efinity-riscv-ide-2022.2.3 $HOME/efinity/2022.1/bin"
	exit
}

if [[ -z $DEVKIT ]]; then
	echo Error: Devkit is not set
	usage
fi

if [[ -z $EFINITY_PROJECT ]]; then
	echo Error: Efinity project is not set
	usage
fi

if [[ -z $RISCV_IDE ]]; then
	echo Error: RISCV_IDE is not set
	usage
fi

if [[ -z $EFINITY_HOME ]]; then
	echo Error: EFINITY_HOME is not set
	usage
fi

source $EFINITY_HOME/setup.sh

DEVKIT=$(echo $DEVKIT | tr '[:upper:]' '[:lower:]')

found=false
for devkit in ${DEVKITS[@]}; do
	if [[ $devkit == *$DEVKIT* ]]; then
		found=true
		DEVKIT=$(echo $devkit | tr -d \")
		break
	fi
done

if [[ $found == false ]]; then
	echo "Error: Devkit $DEVKIT is not supported"
	exit 1
fi

echo Info: $DEVKIT devkit is supported

IFS='/' read -ra ARRAY <<< "$EFINITY_PROJECT"
EFINITY_PROJECT_NAME=${ARRAY[-1]}

EFINITY_PROJECT_PATH="$EFINITY_PROJECT/$DEVKIT/embedded_sw/$EFINITY_PROJECT_NAME"
EfxSapphireSoc_DIR="$EFINITY_PROJECT_PATH/bsp/efinix/EfxSapphireSoc"
APP_DIR="$EfxSapphireSoc_DIR/app"
SOC_H=$EfxSapphireSoc_DIR/include/soc.h

RISCV_TOOLCHAIN="efinity-riscv-ide-[0-9]+\.[0-9]+\.[0-9]"

if [[ $RISCV_IDE =~ $RISCV_TOOLCHAIN ]]; then
	RISCV_IDE=$RISCV_IDE/toolchain/bin
	export PATH=$RISCV_IDE:$PATH
else
	echo Error: RISCV_IDE is not valid
	exit
fi

# copy bootloaderConfig.h to $APP_DIR
cp bootloaderConfig.h $APP_DIR

# check for SMP
SMP=$(cat $SOC_H | grep SYSTEM_PLIC_SYSTEM_CORES_1_EXTERNAL_INTERRUPT)
if [[ ! -z $SMP ]]; then
    # enable SMP flag
    sed -i 's/^#CFLAGS+=-DSMP/CFLAGS+=-DSMP/g' $EfxSapphireSoc_DIR/include/soc.mk
fi

# Ti180 require 3 bytes addressing mode for SPI flash
if [[ $DEVKIT =~ "ti180" ]]; then
	cat $EfxSapphireSoc_DIR/include/soc.mk | grep DEFAULT_ADDRESS_BYTE >> /dev/null
	if [[ ! $? -eq 0 ]]; then
		echo CFLAGS += -DDEFAULT_ADDRESS_BYTE >> $EfxSapphireSoc_DIR/include/soc.mk
	fi
fi

# compile bootloader program
cd $EFINITY_PROJECT_PATH/software/standalone/bootloader && \
	BSP_PATH=$EfxSapphireSoc_DIR make clean && \
	BSP_PATH=$EfxSapphireSoc_DIR make
cd -

FPU=$(cat $SOC_H | grep SYSTEM_CORES_0_FPU | awk '{print $3}')
RAM_SIZE=$(cat $SOC_H | grep SYSTEM_RAM_A_SIZE | awk '{print $3}')

# generate new ram
python3 $EFINITY_PROJECT_PATH/tool/binGen.py -f $FPU -s $RAM_SIZE -b $EFINITY_PROJECT_PATH/software/standalone/bootloader/build/bootloader.bin

# copy new ram to EFINITY_PROJECT
cp -r rom/* $EFINITY_PROJECT/$DEVKIT
rm -rf rom

echo done
