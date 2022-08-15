#include "utils.h"

int32_t index3D(int32_t x, int32_t y, int32_t z) {
    return x * 64 * 64 + y * 64 + z;
}

int32_t pixel_to_index(uint8_t *pixel) {
    int32_t red = pixel[0] >> 2;
    int32_t green = pixel[1] >> 2;
    int32_t blue = pixel[2] >> 2;

    return index3D(red, green, blue);
}

int32_t clamp(int32_t num) {
    if (num < 0)
        return 0;
    if (num > 63)
        return 63;
    return num;
}

int32_t fmt(uint8_t *pixel) {
    uint8_t red = pixel[0] >> 2;
    uint8_t green = pixel[1] >> 2;
    uint8_t blue = pixel[2] >> 2;
    return (red << 0) | (green << 8) | (blue << 16);
}

int32_t vec_to_value(Vec4 v) { return (v.r << 0) | (v.g << 8) | (v.b << 16); }
Vec4 value_to_vec(int32_t value) {
    return (Vec4){(value >> 0) & 255, (value >> 8) & 255, (value >> 16) & 255,
                  0};
}

#define IN(a, b, c) (a <= b && b <= c)

EXPORT("meanshift")
void meanshift(uint8_t *data, int32_t width, int32_t height, int32_t bandwidth,
               int32_t max_gens) {
    static int32_t count_map[64 * 64 * 64];
    static int32_t color_map[64 * 64 * 64];
    Image img = {data, width, height};
    img_gamma_correct(img);

    for (int i = 0; i < 64 * 64 * 64; i++) {
        count_map[i] = 0;
        color_map[i] = 0;
    }

    for (int32_t x = 0; x < img.width; x++) {
        for (int32_t y = 0; y < img.height; y++) {
            uint8_t *pixel = img_get(img, x, y);
            int32_t index = pixel_to_index(pixel);

            if (!count_map[index])
                color_map[index] = fmt(pixel);

            count_map[index]++;
        }
    }

    uint8_t done = 0;
    for (int32_t gen = 0; !done && gen < max_gens; gen++) {
        putsf("gen = %d", gen);
        done = 1;
        for (int32_t x = 0; x < img.width; x++) {
            for (int32_t y = 0; y < img.height; y++) {
                uint8_t *pixel = img_get(img, x, y);
                int32_t index = pixel_to_index(pixel);

                if (color_map[index] < 0)
                    continue;

                Vec4 avg_center = {0, 0, 0, 0};
                Vec4 curr_center = value_to_vec(color_map[index]);

                int32_t count = 0;

                int32_t rl = clamp(curr_center.r - bandwidth);
                int32_t ru = clamp(curr_center.r + bandwidth);

                int32_t gl = clamp(curr_center.g - bandwidth);
                int32_t gu = clamp(curr_center.g + bandwidth);

                int32_t bl = clamp(curr_center.b - bandwidth);
                int32_t bu = clamp(curr_center.b + bandwidth);

                for (int32_t red = rl; red <= ru; red++) {
                    for (int32_t green = gl; green <= gu; green++) {
                        for (int32_t blue = bl; blue <= bu; blue++) {
                            int32_t idx = index3D(red, green, blue);
                            // Vec4 v = value_to_vec(color_map[idx]);
                            Vec4 v = {red, green, blue, 0};
                            vec_mul(&v, count_map[idx]);
                            vec_add(&avg_center, &v);
                            count += count_map[idx];
                        }
                    }
                }

                vec_div(&avg_center, count);

                if (vec_eq(avg_center, curr_center)) {
                    color_map[index] |= 0xFF000000;
                } else {
                    done = 0;
                    avg_center.r = clamp(avg_center.r);
                    avg_center.g = clamp(avg_center.g);
                    avg_center.b = clamp(avg_center.b);

                    color_map[index] = vec_to_value(avg_center);
                }
            }
        }
    }

    for (int32_t x = 0; x < img.width; x++) {
        for (int32_t y = 0; y < img.height; y++) {
            uint8_t *pixel = img_get(img, x, y);
            int32_t index = pixel_to_index(pixel);
            int32_t count = color_map[index];

            Vec4 rgb = value_to_vec(count);

            rgb.r <<= 2;
            rgb.g <<= 2;
            rgb.b <<= 2;

            img_set(img, x, y, (uint8_t)rgb.r, (uint8_t)rgb.g, (uint8_t)rgb.b,
                    255);
        }
    }

    img_gamma_reset(img);
}
