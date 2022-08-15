
CFLAGS=-Wall -Wextra -Os -flto
WASM=-msimd128 -mbulk-memory --target=wasm32 -nostdlib -Wl,--no-entry -Wl,--allow-undefined

all: wasm native

wasm:
	clang $(CFLAGS) ${WASM} -o main.wasm src/meanshift.c src/kmeans.c src/histogram.c src/splitmerge.c src/utils.c src/utils_wasm.c

native:
	$(CC) $(CFLAGS) -o segment -lpng src/native.c src/kmeans.c src/meanshift.c src/histogram.c src/splitmerge.c src/utils.c