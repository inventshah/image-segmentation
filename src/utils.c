#include "utils.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

int putsf(const char *fmt, ...) {
    char message[256];
    va_list args;
    va_start(args, fmt);
    int ret = stbsp_vsnprintf(message, 256, fmt, args);
    va_end(args);
    puts(message);
    return ret;
}

// gamma = 2
static uint8_t gamma_correct(int32_t num) { return (num * num) >> 8; }
static uint8_t gamma_reset(int32_t num) { return sqrt(num << 8); }

void img_gamma_correct_impl(Image img) {
    for (int32_t y = 0; y < img.height; y++) {
        for (int32_t x = 0; x < img.width; x++) {
            uint8_t *pixel = img_get(img, x, y);

            uint8_t r = gamma_correct(pixel[0]);
            uint8_t g = gamma_correct(pixel[1]);
            uint8_t b = gamma_correct(pixel[2]);
            uint8_t a = pixel[3];

            img_set(img, x, y, r, g, b, a);
        }
    }
}
void img_gamma_correct_noop(Image img) { (void)img; }
void (*img_gamma_correct)(Image) = &img_gamma_correct_impl;

void img_gamma_reset_impl(Image img) {
    for (int32_t y = 0; y < img.height; y++) {
        for (int32_t x = 0; x < img.width; x++) {
            uint8_t *pixel = img_get(img, x, y);

            uint8_t r = gamma_reset(pixel[0]);
            uint8_t g = gamma_reset(pixel[1]);
            uint8_t b = gamma_reset(pixel[2]);
            uint8_t a = pixel[3];

            img_set(img, x, y, r, g, b, a);
        }
    }
}
void img_gamma_reset_noop(Image img) { (void)img; }
void (*img_gamma_reset)(Image) = &img_gamma_reset_impl;

EXPORT("change_gamma_status")
void change_gamma_status(int32_t on) {
    img_gamma_correct = on ? &img_gamma_correct_impl : &img_gamma_correct_noop;
    img_gamma_reset = on ? &img_gamma_reset_impl : &img_gamma_reset_noop;
}

int8_t img_in_bounds(Image img, int32_t x, int32_t y) {
    return x >= 0 && y >= 0 && x <= img.width && y <= img.height;
}

uint8_t *img_get(Image img, int32_t x, int32_t y) {
    return &img.data[(y * img.width + x) * 4];
}

void img_set(Image img, int x, int y, uint8_t red, uint8_t green, uint8_t blue,
             uint8_t alpha) {
    uint8_t *pixel = &img.data[(y * img.width + x) * 4];
    pixel[0] = red;
    pixel[1] = green;
    pixel[2] = blue;
    pixel[3] = alpha;
}

void img_set_alpha(Image img, int32_t x, int32_t y, uint8_t alpha) {
    img.data[(y * img.width + x) * 4 + 3] = alpha;
}

uint8_t clip(int32_t c) { return c > 0 ? c > 255 ? 255 : (uint8_t)c : 0; }

void vec_add_rgba(Vec4 *v, const uint8_t *rgb) {
    v->r += rgb[0];
    v->g += rgb[1];
    v->b += rgb[2];
    v->a += rgb[3];
}
void vec_add(Vec4 *v, Vec4 *u) {
    v->r += u->r;
    v->g += u->g;
    v->b += u->b;
    v->a += u->a;
}
void vec_div(Vec4 *v, int32_t n) {
    v->r /= n;
    v->g /= n;
    v->b /= n;
    v->a /= n;
}
void vec_mul(Vec4 *v, int32_t n) {
    v->r *= n;
    v->g *= n;
    v->b *= n;
    v->a *= n;
}
void vec_cpy(Vec4 *src, Vec4 *dest) {
    dest->r = src->r;
    dest->g = src->g;
    dest->b = src->b;
    dest->a = src->a;
}
uint8_t vec_eq(Vec4 v, Vec4 u) {
    return v.r == u.r && v.g == u.g && v.b == u.b && v.a == u.a;
}
