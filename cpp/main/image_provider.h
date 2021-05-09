#ifndef MATERIALS_CLASSIFIER_IMAGE_PROVIDER_H_
#define MATERIALS_CLASSIFIER_IMAGE_PROVIDER_H_

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"

TfLiteStatus GetImage(tflite::ErrorReporter* error_reporter, int image_width,
                      int image_height, int channels, uint8_t* image_data);

#endif  // MATERIALS_CLASSIFIER_IMAGE_PROVIDER_H_
