#ifndef UTILS_H_INCLUDE
#define UTILS_H_INCLUDE

#include <stdint.h>

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#if __has_attribute(export_name)
#define EXPORT(name) __attribute__((export_name(name)))
#else
#define EXPORT(name)
#endif

extern int puts(const char *);
extern int random_int(void);

double sqrt(double);
void *malloc(unsigned long n);
void free(void *p);
void memory_reset(void);

int putsf(const char *fmt, ...);

typedef struct {
    uint8_t *data;
    int32_t width;
    int32_t height;
} Image;

int8_t img_in_bounds(Image img, int32_t x, int32_t y);
uint8_t *img_get(Image img, int32_t x, int32_t y);
void img_set(Image img, int32_t x, int32_t y, uint8_t red, uint8_t green,
             uint8_t blue, uint8_t alpha);
void img_set_alpha(Image img, int x, int y, uint8_t alpha);

extern void (*img_gamma_correct)(Image);
extern void (*img_gamma_reset)(Image);

uint8_t clip(int32_t c);

typedef struct {
    int32_t r;
    int32_t g;
    int32_t b;
    int32_t a;
} Vec4;

void vec_add_rgba(Vec4 *v, const uint8_t *rgb);
void vec_add(Vec4 *v, Vec4 *u);
void vec_div(Vec4 *v, int32_t n);
void vec_mul(Vec4 *v, int32_t n);
void vec_cpy(Vec4 *src, Vec4 *dest);
uint8_t vec_eq(Vec4 v, Vec4 u);

#endif