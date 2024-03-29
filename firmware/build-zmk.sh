#!/usr/bin/env bash
#

l=$PWD
zmk_folder="$HOME/lavello/zmk/app"
zmk_config="$HOME/playground/keyboard/mlego-zmk/config/"
pushd $zmk_folder


build_folder="xiao_rp2040_rev7"
board="seeeduino_xiao_rp2040"
shield="mlego5x13_rev7"

build_folder="xiao_ble_rev8"
board="seeeduino_xiao_ble"
shield="mlego5x13_rev8"

#build_folder="xiao_ble_61_rev8"
#board="seeeduino_xiao_ble"
#shield="mlego5x13_61_r8"

build_folder="mlego_m66_rev4-ls013d705"
board="nice_nano_v2"
shield="mlego_m66_rev4"


rm -rf $build_folder

west build -d $build_folder -p always -b $board -- -DSHIELD=$shield -DZMK_CONFIG=$zmk_config
set -x 
[[ -f $build_folder/zephyr/zmk.uf2 ]] && cp $build_folder/zephyr/zmk.uf2 $l/$build_folder.uf2
[[ -d /run/media/drFaustroll/NICENANO/ ]] && cp $l/$build_folder.uf2 /run/media/drFaustroll/NICENANO/
