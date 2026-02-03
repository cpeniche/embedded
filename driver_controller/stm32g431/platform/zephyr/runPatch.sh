#!/bin/bash
set -e
cp -v ~/project/patches/st,stm32-uart-base.yaml ~/zephyr/dts/bindings/serial
cp -v ~/project/patches/uart* ~/zephyr/drivers/serial