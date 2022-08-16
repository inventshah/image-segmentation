
CFLAGS=-Wall -Wextra -Os -flto
WASM=-msimd128 --target=wasm32 -nostdlib -Wl,--no-entry -Wl,--allow-undefined
SRC=src/meanshift.c src/kmeans.c src/histogram.c src/splitmerge.c src/utils.c

all: wasm native

wasm:
	clang $(CFLAGS) ${WASM} -mbulk-memory -o main.wasm $(SRC) src/utils_wasm.c

native:
	$(CC) $(CFLAGS) -o segment -lpng $(SRC) src/native.c

wasm-slow:
	clang -Wall -Wextra ${WASM} -o main-slow.wasm $(SRC) src/utils_wasm.c
