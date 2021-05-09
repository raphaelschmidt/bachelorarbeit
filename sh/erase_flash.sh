#!/bin/bash
. /esp-idf/export.sh
cd ./cpp
esptool.py -p "$1" erase_flash
touch ../state/erase_flash_done

