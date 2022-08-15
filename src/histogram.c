#include "utils.h"

void populate_histogram(int32_t *histogram, Image img) {
    for (int32_t x = 0; x < img.width; x++) {
        for (int32_t y = 0; y < img.height; y++) {
            uint8_t *pixel = img_get(img, x, y);

            histogram[0 + pixel[0]]++;
            histogram[256 + pixel[1]]++;
            histogram[2 * 256 + pixel[2]]++;
        }
    }
}

int32_t get_weight(int32_t *histogram, int32_t start, int32_t end) {
    int32_t weight = 0;

    for (int32_t i = start; i < end; i++)
        weight += histogram[i];

    return weight;
}

uint8_t balance(int32_t *histogram) {
    int32_t start = 0, end = 255;
    int32_t middle = (start + end) / 2;

    int32_t left_weight = get_weight(histogram, start, middle);
    int32_t right_weight = get_weight(histogram, middle, end + 1);
    while (start <= end) {
        if (right_weight > left_weight) {
            right_weight -= histogram[end];
            end--;
            if ((start + end) / 2 < middle) {
                right_weight += histogram[middle];
                left_weight -= histogram[middle];
                middle--;
            }
        } else {
            left_weight -= histogram[start];
            start++;
            if ((start + end) / 2 >= middle) {
                middle++;
                left_weight += histogram[middle];
                right_weight -= histogram[middle];
            }
        }
    }

    return clip(middle);
}

EXPORT("histogram")
void balanced_histogram_thresholding(uint8_t *data, int32_t width,
                                     int32_t height) {
    static int32_t histogram[3 * 256];
    Image img = {data, width, height};
    img_gamma_correct(img);

    populate_histogram(histogram, img);

    uint8_t red = balance(histogram + 0);
    uint8_t green = balance(histogram + 256);
    uint8_t blue = balance(histogram + 256 * 2);

    putsf("Threshold = rgb(%u, %u, %u)", red, green, blue);

    for (int32_t x = 0; x < img.width; x++) {
        for (int32_t y = 0; y < img.height; y++) {
            uint8_t *rgb = img_get(img, x, y);
            if (rgb[0] < red && rgb[1] < green && rgb[2] < blue) {
                img_set(img, x, y, 0, 0, 0, 255);
            } else if (rgb[0] > red && rgb[1] > green && rgb[2] > blue) {
                img_set(img, x, y, 255, 255, 255, 255);
            } else {
                img_set(img, x, y, 127, 127, 127, 255);
            }
        }
    }

    img_gamma_reset(img);
}
