CC=clang
CFLAGS=-Wall -Wextra -Wpedantic -std=c11 -O2

LIBFLAGS =$(shell pkg-config --libs opengl)
LIBFLAGS+=$(shell pkg-config --libs glew)
LIBFLAGS+=$(shell pkg-config --libs glfw3)

TARGET=shadertoy
.PHONY: all, clean
all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $@ $^ -Iinclude $(LIBFLAGS)

clean:
	$(RM) $(TARGET)
