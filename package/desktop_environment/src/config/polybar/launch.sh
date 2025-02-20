#!/bin/sh

DIR=$(dirname $(readlink -f "$0"))

source /etc/profile

# Terminate already running bar instances
killall -q polybar

# Launch the bar
mod=$(polybar --list-monitors | cut -d":" -f1)
MONITOR=$mon polybar -q main -c "$DIR"/config.ini &
