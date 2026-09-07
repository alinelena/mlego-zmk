#!/usr/bin/env bash
#

set -e

l=$PWD
repo_root="$HOME/playground/keyboard/mlego-zmk"
zmk_folder="$HOME/lavello/zmk/app"
zmk_config="$repo_root/config"
zmk_extra="$HOME/lavello/zmk-helpers"

export ZEPHYR_TOOLCHAIN_VARIANT="${ZEPHYR_TOOLCHAIN_VARIANT:-zephyr}"
export ZEPHYR_SDK_INSTALL_DIR="${ZEPHYR_SDK_INSTALL_DIR:-/opt/zephyr-sdk-0.17.0}"
export ZEPHYR_BASE="${ZEPHYR_BASE:-$HOME/lavello/zmk/zephyr}"

if command -v west &>/dev/null; then
  WEST_CMD="west"
else
  WEST_CMD="micromamba run -n zmk west"
fi

shield="mlego_m66_rev4 ls013b7dh05"
board="nice_nano//zmk"
build_folder="build_mlego_m66_rev4_ls013b7dh05"
uf2_name="mlego_m66_rev4-ls013b7dh05-nice_nano.uf2"

pushd "$zmk_folder" > /dev/null

rm -rf "$build_folder"

$WEST_CMD -z "$ZEPHYR_BASE" build -d "$build_folder" -p always -b "$board" -- \
  -DSHIELD="$shield" \
  -DZMK_CONFIG="$zmk_config" \
  -DZMK_EXTRA_MODULES="$zmk_extra"

if [[ -f "$build_folder/zephyr/zmk.uf2" ]]; then
  cp "$build_folder/zephyr/zmk.uf2" "$l/$uf2_name"
  [[ -d "$repo_root/firmware" ]] && cp "$build_folder/zephyr/zmk.uf2" "$repo_root/firmware/$uf2_name"
  echo "Firmware successfully built: $uf2_name"
fi

popd > /dev/null

echo "Waiting for /run/media/drFaustroll/NICENANO/ ..."
while [ ! -d /run/media/drFaustroll/NICENANO/ ]; do
  sleep 1
  echo -n "."
done

echo " done"

set -x

cp "$repo_root/firmware/$uf2_name" /run/media/drFaustroll/NICENANO/

