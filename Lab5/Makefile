SOURCES=main.c util.c cd_ls_pwd.c
TARGET=main

all: $(TARGET)

$(TARGET): $(SOURCES)
	gcc -w -g -ggdb -O0 -o $@ $^ -m32

clean:
	rm -f $(TARGET)