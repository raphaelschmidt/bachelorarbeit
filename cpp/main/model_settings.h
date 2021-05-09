#ifndef MATERIALS_MODEL_SETTINGS_H_
#define MATERIALS_MODEL_SETTINGS_H_
constexpr int kNumCols = 96;
constexpr int kNumRows = 96;
constexpr int kNumChannels = 3;
constexpr int kMaxImageSize = kNumCols * kNumRows * kNumChannels;
constexpr int kCategoryCount = 2;
extern const char* kCategoryLabels[kCategoryCount];
#endif