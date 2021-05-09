#!/bin/bash
. /esp-idf/export.sh
cd ./cpp
cp ../mobileNetV2_imagenet_96_96_3model.cc ./main/materials_model_data.cc
idf.py build
touch ../state/build_done

