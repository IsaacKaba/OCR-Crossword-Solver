# Simple SDL mini Makefile
# gcc *.c ../segmentation/sdl_operations.c ../segmentation/segmentation.c -lSDL2 -lSDL2_image -lm


CC=gcc

CFLAGS= -Wall -Wextra -std=c99 -O0
LDFLAGS=
LDLIBS= -lm -lSDL2 -lSDL2_image

all: xor ocr

xor : train.o layer.o neural_network.o ../segmentation/sdl_operations.o
ocr : train.o layer.o neural_network.o ../segmentation/sdl_operations.o
xor.o : train.h layer.h neural_network.h ../segmentation/sdl_operations.h
ocr.o : train.h layer.h neural_network.h ../segmentation/sdl_operations.h

clean:
	${RM} *.o
	${RM} *.d
	${RM} xor
	${RM} ocr


# END
