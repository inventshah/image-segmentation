#include "utils.h"

uint8_t random_color(void) { return random_int() & 0xff; }

Vec4 random_center(void) {
    return (Vec4){random_color(), random_color(), random_color(), 255};
}

int32_t distance_from_center(Vec4 v, const uint8_t *rgb) {
    int32_t r = v.r - rgb[0];
    int32_t g = v.g - rgb[1];
    int32_t b = v.b - rgb[2];

    r *= r;
    g *= g;
    b *= b;

    return r + g + b;
}

int32_t calculate_centers(int32_t *nums, Vec4 *sums, Vec4 *old_centers,
                          int32_t k) {
    int32_t done = 1;

    for (int32_t i = 0; i < k; i++) {
        if (nums[i]) {
            vec_div(&sums[i], nums[i]);

            if (!vec_eq(old_centers[i], sums[i]))
                done = 0;

            vec_cpy(&sums[i], &old_centers[i]);
        } else {
            old_centers[i] = random_center();
            done = 0;
        }
    }

    return done;
}

EXPORT("kmeans")
void kmeans(uint8_t *data, int32_t width, int32_t height, int32_t k,
            int32_t max_gens) {
    uint8_t done = 0;

    Vec4 centers[k], sums[k];
    int32_t nums[k];

    Image img = {data, width, height};

    img_gamma_correct(img);

    for (int32_t i = 0; i < k; i++) {
        centers[i] = random_center();

        sums[i] = (Vec4){0, 0, 0, 0};
        nums[i] = 0;
    }

    for (int32_t gen = 0; !done && gen < max_gens; gen++) {
        for (int32_t x = 0; x < img.width; x++) {
            for (int32_t y = 0; y < img.height; y++) {
                uint8_t closest_label = 0;
                int32_t current_dist, min_dist = 256 * 256 * 256;

                for (uint8_t center = 0; center < k; center++) {
                    current_dist = distance_from_center(centers[center],
                                                        img_get(img, x, y));
                    if (current_dist < min_dist) {
                        min_dist = current_dist;
                        closest_label = center;
                    }
                }

                img_set_alpha(img, x, y, closest_label);

                vec_add_rgba(&sums[closest_label], img_get(img, x, y));

                nums[closest_label]++;
            }
        }

        done = calculate_centers(nums, sums, centers, k);

        for (int32_t i = 0; i < k; i++) {
            sums[i] = (Vec4){0, 0, 0, 0};
            nums[i] = 0;
        }
    }

    for (int32_t i = 0; i < k; i++) {
        putsf("Center %d: rgb(%d, %d, %d)", i, centers[i].r, centers[i].g,
              centers[i].b);
    }

    for (int32_t x = 0; x < img.width; x++) {
        for (int32_t y = 0; y < img.height; y++) {
            int label = img_get(img, x, y)[3];
            Vec4 center = centers[label];

            img_set(img, x, y, clip(center.r), clip(center.g), clip(center.b),
                    255);
        }
    }

    img_gamma_reset(img);
}
