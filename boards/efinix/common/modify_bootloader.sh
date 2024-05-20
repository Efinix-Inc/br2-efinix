#!/bin/bash

# $1 = development board i.e. t120f324, ti60f225
# $2 = path to Efinity project
# $3 = path to RISC-V SDK

DEVKIT=$1
EFINITY_PROJECT=$2
RISCV_IDE=$3

unset EXAMPLE_DESIGN
unset DEBUG

function usage()
{
	echo
	echo "This script used to modify and compile the first stage bootloader program for booting Linux."
	echo
	echo "usage: $0 Devkit Project IDE [ -e][-d]"
	echo "Positional arguments:"
	echo "	Devkit		Supported devkits are t120f324, ti60f225, ti180j484, ti375c529"
	echo "	Project		Efinity project directory. For example, $HOME/soc"
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
		echo enable debug message
		DEBUG=1
		;;
	e)
		echo using example design
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

DEVKIT=$(echo $DEVKIT | tr '[:upper:]' '[:lower:]')
JSON_FILE="sapphire-soc-dt-generator/config/drivers.json"
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
	echo "Error: Devkit $DEVKIT is not supported"
	exit 1
fi

echo Info: $DEVKIT devkit is supported

SAPPHIRE_IP=

if [[ $EXAMPLE_DESIGN ]];then
	DEVKIT_DIR=$DEVKIT
	if [[ "Ti375C529_devkit" == *$DEVKIT* ]]; then
		EXAMPLE_DESIGN_DIR="/ip/EfxSapphireHpSoc_slb/$DEVKIT_DIR"
	else
		tmp=$(find $EFINITY_PROJECT -type d -name $DEVKIT_DIR)
		if [ -z $tmp ]; then
			echo Error: Example design for $DEVKIT is not exists
			exit 1
		fi

		EXAMPLE_DESIGN_DIR=${tmp//$EFINITY_PROJECT}
		ip_name=${EXAMPLE_DESIGN_DIR//$DEVKIT_DIR}
		ip_name=${ip_name//"/ip/"}
		ip_name=${ip_name//"/"}
		EFINITY_PROJECT_NAME=$ip_name
		SAPPHIRE_IP=$ip_name

	fi
else
	DEVKIT_DIR=
	EXAMPLE_DESIGN_DIR=""
	tmp=$(find $EFINITY_PROJECT -type d -name bsp)
	embedded_sw="$EFINITY_PROJECT/embedded_sw/"
	ip_name=${tmp//"/bsp"}
	ip_name=${ip_name//$embedded_sw}
	SAPPHIRE_IP=$ip_name

fi

if [[ "Ti375C529_devkit" == *$DEVKIT* ]]; then
	EFINITY_PROJECT_PATH="$EFINITY_PROJECT$EXAMPLE_DESIGN_DIR/embedded_sw/efx_hard_soc"
else
	EFINITY_PROJECT_PATH="$EFINITY_PROJECT$EXAMPLE_DESIGN_DIR/embedded_sw/$SAPPHIRE_IP"
fi

EfxSapphireSoc_DIR="$EFINITY_PROJECT_PATH/bsp/efinix/EfxSapphireSoc"
APP_DIR="$EfxSapphireSoc_DIR/app"
SOC_H=$EfxSapphireSoc_DIR/include/soc.h

if [ $DEBUG ]; then
	echo
	echo "******** DEBUG *******"
	echo EFINITY_PROJECT=$EFINITY_PROJECT
	echo EXAMPLE_DESIGN_DIR=$EXAMPLE_DESIGN_DIR
	echo DEVKIT_DIR=$DEVKIT_DIR
	echo EFINITY_PROJECT_PATH=$EFINITY_PROJECT_PATH
	echo EfxSapphireSoc_DIR=$EfxSapphireSoc_DIR
	echo APP_DIR=$APP_DIR
	echo SOC_H=$SOC_H
	echo "***********************"
	echo
fi

RISCV_TOOLCHAIN="efinity-riscv-ide-[0-9]+\.[0-9]+\.[0-9]"

if [[ $RISCV_IDE =~ $RISCV_TOOLCHAIN ]]; then
	RISCV_IDE=$RISCV_IDE/toolchain/bin
	export PATH=$RISCV_IDE:$PATH
else
	echo Error: RISCV_IDE is not valid
	exit
fi

BOOTLOADER_DIR=$EFINITY_PROJECT_PATH/software/standalone/bootloader
echo BOOTLOADER_DIR=$BOOTLOADER_DIR
if [[ "Ti375C529_devkit" == *$DEVKIT* ]]; then
	cp bootloaderConfig.h $BOOTLOADER/src
	echo $BOOTLOADER_SRC
else
	cp bootloaderConfig.h $APP_DIR
fi

# check for SMP
SMP=$(cat $SOC_H | grep SYSTEM_PLIC_SYSTEM_CORES_1_EXTERNAL_INTERRUPT)
if [[ ! -z $SMP ]]; then
    # enable SMP flag
    sed -i 's/^#CFLAGS+=-DSMP/CFLAGS+=-DSMP/g' $EfxSapphireSoc_DIR/include/soc.mk
fi

# compile bootloader program
cd $EFINITY_PROJECT_PATH/software/standalone/bootloader && \
	BSP_PATH=$EfxSapphireSoc_DIR make clean && \
	BSP_PATH=$EfxSapphireSoc_DIR make

	if [ ! $? -eq 0 ]; then
		echo Error: compilation failed
		cd -
		exit 1
	fi
cd - > /dev/null

cp -r $BOOTLOADER_DIR/build $EFINITY_PROJECT/linux_bootloader
echo The bootloader is in $EFINITY_PROJECT/linux_bootloader/bootloader.hex
echo done
