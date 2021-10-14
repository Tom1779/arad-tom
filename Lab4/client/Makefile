SOURCES=main.c client.c remote.c local.c
TARGET=main

all: $(TARGET)

$(TARGET): $(SOURCES)
	gcc -o $@ $^ -m32

clean:
	rm -f $(TARGET)