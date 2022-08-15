#ifndef API_H_INCLUDE
#define API_H_INCLUDE

#include "utils.h"

#include <stdarg.h>

void kmeans(uint8_t *data, int32_t width, int32_t height, int32_t k,
            int32_t max_gens);
void meanshift(uint8_t *data, int32_t width, int32_t height, int32_t bandwidth,
               int32_t max_gens);
void split_and_merge(uint8_t *data, int32_t width, int32_t height,
                     int32_t tolerance);
void balanced_histogram_thresholding(uint8_t *data, int32_t width,
                                     int32_t height);

#endif