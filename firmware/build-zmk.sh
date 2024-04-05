#!/usr/bin/env bash
#

l=$PWD
zmk_folder="$HOME/lavello/zmk/app"
zmk_config="$HOME/playground/keyboard/mlego-zmk/config/"
pushd $zmk_folder


build_folder="xiao_rp2040_rev7"
board="seeeduino_xiao_rp2040"
shield="mlego5x13_rev7"


#build_folder="xiao_ble_61_rev8"
#board="seeeduino_xiao_ble"
#shield="mlego5x13_61_r8"

##shield="mlego_m66_rev4"
#shield="mlego_m66_rev4_ls011b7dh03"
#shield="mlego_m66_rev4_ls013b7dh03"
#shield="mlego_m66_rev4_ls013b7dh05"
#shield="mlego_m66_rev4_eink154"
#for s in "" "_ls011b7dh03" "_ls013b7dh03" "_ls013b7dh05" "_eink154" "_eink213"; do
# for s in "_eink154"; do
#for s in "_eink213"; do
#for s in "ls013b7dh05" ; do
#for s in "ls013b7dh03s" ; do
#for s in "ls011b7dh03" ; do
shield="mlego_m66_rev4 $s"
board="nice_nano_v2"
build_folder="mlego_m66_rev4${shield/ /_}"
build_folder="xiao_rev9"
board="seeeduino_xiao_rp2040"
shield="mlego5x13_rev9"


rm -rf "$build_folder"

west build -d "$build_folder" -p always -b $board -- -DSHIELD="$shield" -DZMK_CONFIG=$zmk_config
[[ -f "$build_folder/zephyr/zmk.uf2" ]] && cp "$build_folder/zephyr/zmk.uf2" "$l/$build_folder.uf2"
# done
while [ ! -d /run/media/drFaustroll/NICENANO/ ]; do
    sleep 1
    echo -n "."
done
echo "done"
set -x
cp "$l/$build_folder.uf2" /run/media/drFaustroll/NICENANO/
