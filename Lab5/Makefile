SOURCES=main.c util.c cd_ls_pwd.c
TARGET=main

all: $(TARGET)

$(TARGET): $(SOURCES)
	gcc -w -g -ggdb -O0 -o $@ $^ -m32

diskimage:
	sudo dd if=/dev/zero of=diskimage bs=1024 count=1440 
	sudo mke2fs -b 1024 diskimage 1440
	sudo mount diskimage /mnt
	(cd /mnt; sudo mkdir dir1 dir2 dir1/dir3; sudo touch file1 file2; ls -l)
	sudo umount /mnt
	sudo chmod 777 diskimage

.PHONY: run
run: diskimage
	./main

clean:
	rm -f diskimage $(TARGET)