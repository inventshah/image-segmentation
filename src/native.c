#include "api.h"
#include "utils.h"

#include <png.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(char *str) {
    puts(str);
    exit(1);
}
void check_null(void *ptr, char *str) {
    if (ptr == NULL)
        error(str);
}

int random_int(void) { return rand(); }

Image open_image(char *filename) {
    int num_row_bytes, i;

    FILE *fp;

    png_infop info;
    png_structp png;
    png_byte color_type, bit_depth;

    // Allocate space
    Image img = {};

    putsf("Opening %s", filename);
    fp = fopen(filename, "rb");
    check_null(fp, "Unable to open file");

    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    check_null(png, "Unable to create png struct in open_png");

    info = png_create_info_struct(png);
    check_null(info, "Unable to create info struct in open_png");

    if (setjmp(png_jmpbuf(png)))
        error("setjmp() returned true in open_png");

    png_init_io(png, fp);

    png_read_info(png, info);

    // Get image info
    img.width = png_get_image_width(png, info);
    img.height = png_get_image_height(png, info);

    color_type = png_get_color_type(png, info);
    bit_depth = png_get_bit_depth(png, info);

    // Configure the png
    if (bit_depth == 16)
        png_set_strip_16(png);
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png);

    png_read_update_info(png, info);

    // Allocate the image space
    num_row_bytes = png_get_rowbytes(png, info);
    img.data = (png_byte *)malloc(sizeof(png_byte) * img.height * img.width);
    check_null(img.data, "Unable to malloc png_bytep for Image in open_png");
    png_bytep *image = (png_bytep *)malloc(sizeof(png_bytep) * img.height);
    check_null(image, "Unable to malloc png_bytep for Image in open_png");

    for (i = 0; i < img.height; i++) {
        // image[i] = (png_byte *)malloc(sizeof(png_byte) * img.width * 4);
        image[i] = &img.data[i * img.width * 4];
    }

    png_read_image(png, image);

    fclose(fp);
    png_destroy_read_struct(&png, &info, NULL);
    free(image);

    return img;
}

void save_image(Image img, char *filename) {
    int i;
    FILE *fp;

    png_infop info;
    png_structp png;

    putsf("Saving file `%s`", filename);

    fp = fopen(filename, "wb");
    check_null(fp, "Unable to open file");

    png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    check_null(png, "Unable to create png struct in open_png");

    info = png_create_info_struct(png);
    check_null(info, "Unable to create info struct in open_png");

    if (setjmp(png_jmpbuf(png)))
        error("setjmp() returned true in save_png");

    png_init_io(png, fp);

    // Output is 8-bit depth, RGBA format
    png_set_IHDR(png, info, img.width, img.height, 8, PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    check_null(img.data, "Image data was null");

    png_bytep *image = (png_bytep *)malloc(sizeof(png_bytep) * img.height);
    check_null(image, "Unable to malloc png_bytep for Image in open_png");

    size_t num_row_bytes = png_get_rowbytes(png, info);

    for (i = 0; i < img.height; i++) {
        image[i] = (png_byte *)malloc(sizeof(png_byte) * num_row_bytes);
        memcpy(image[i], &img.data[i * img.width * 4], img.width * 4);
    }

    png_write_image(png, image);
    png_write_end(png, NULL);

    fclose(fp);
    for (i = 0; i < img.height; i++) {
        free(image[i]);
    }
    free(image);

    png_destroy_write_struct(&png, &info);
}

int main(int argc, char *argv[]) {
    if (argc < 2)
        error("missing alg name (one of `kmeans`, `meanshift`, `splitmerge` "
              "and `histogram`)");
    if (argc < 3)
        error("missing source image");

    Image img = open_image(argv[2]);

    if (strcmp(argv[1], "kmeans") == 0) {
        if (argc < 4)
            error("kmeans requires a K value");
        int k = atoi(argv[3]);
        kmeans(img.data, img.width, img.height, k, 200);
    } else if (strcmp(argv[1], "meanshift") == 0) {
        if (argc < 4)
            error("meanshift requires a bandwidth value");
        int k = atoi(argv[3]);
        meanshift(img.data, img.width, img.height, k, 200);
    } else if (strcmp(argv[1], "splitmerge") == 0) {
        if (argc < 4)
            error("splitmerge requires a tolerance value");
        int k = atoi(argv[3]);
        split_and_merge(img.data, img.width, img.height, k);
    } else if (strcmp(argv[1], "histogram") == 0) {
        balanced_histogram_thresholding(img.data, img.width, img.height);
    }

    save_image(img, "out.png");
    free(img.data);
}