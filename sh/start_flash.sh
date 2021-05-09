#!/bin/bash
. /esp-idf/export.sh
cd ./cpp
idf.py -p "$1" flash
touch ../state/flash_done

