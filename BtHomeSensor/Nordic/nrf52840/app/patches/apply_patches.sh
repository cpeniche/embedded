#!/usr/bin/env bash
#
# Applies the local SSD1683 bring-up patch on top of a fresh west
# workspace checkout. See ../Docs/SSD1683_bringup_bugs_and_fixes.md for
# what it actually changes and why.
#
# Usage: run from anywhere, with the west workspace's zephyr/ checkout
# present at the path below (adjust ZEPHYR_DIR if your workspace layout
# differs). The LVGL checkout (modules/lib/gui/lvgl) needs no patch -
# its only local change was debugging instrumentation, since reverted.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ZEPHYR_DIR="${ZEPHYR_DIR:-$HOME/zephyr}"

apply_patch() {
	local target_dir="$1"
	local patch_file="$2"

	if [ ! -d "$target_dir" ]; then
		echo "error: $target_dir does not exist" >&2
		exit 1
	fi

	if git -C "$target_dir" apply --check --reverse "$patch_file" 2>/dev/null; then
		echo "already applied, skipping: $patch_file"
		return
	fi

	if ! git -C "$target_dir" apply --check "$patch_file"; then
		echo "error: $patch_file does not apply cleanly to $target_dir" >&2
		echo "       (checkout revision may not match the one these patches were made against)" >&2
		exit 1
	fi

	git -C "$target_dir" apply "$patch_file"
	echo "applied: $patch_file -> $target_dir"
}

apply_patch "$ZEPHYR_DIR" "$SCRIPT_DIR/zephyr.patch"
