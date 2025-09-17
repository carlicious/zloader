TARGETS := zloader.bin.gz.uImage_patched

all:	$(TARGETS)

CC := aarch64-linux-gnu-gcc
LD := aarch64-linux-gnu-ld
CFLAGS := -nostartfiles -nostdlib -c -ffreestanding -fomit-frame-pointer -Os -Wall
OBJCOPY := aarch64-linux-gnu-objcopy
PHP := php
GZIP := gzip
MKIMAGE := mkimage
DD := dd

%.o:	%.c
	$(CC) $(CFLAGS) $< -o $@

%.bin: %.elf
	$(OBJCOPY) -j .text -O binary $< $@

zloader_3_2.bin: zloader.bin.gz.uImage
	$(DD) if=$< bs=64 skip=1 | $(GZIP) -d -c > $@ || true

get_boot_flag.elf: get_boot_flag.o
	$(LD) -dT LinkerScript -Ttext 0x41E05028 $< -o $@

zld_swapBootImage.elf: zld_swapBootImage.o
	$(LD) -dT LinkerScript -Ttext 0x41E05314 $< -o $@

do_boot.elf: do_boot.o
	$(LD) -dT LinkerScript -Ttext 0x41E051A8 $< -o $@

zloader_3_2_patched.bin: zloader_3_2.bin get_boot_flag.bin zld_swapBootImage.bin do_boot.bin
	$(PHP) patch.php $< $@

zloader.bin.gz.uImage_patched: zloader_3_2_patched.bin
	$(GZIP) -f --best -k $<
	$(MKIMAGE) -n 'zld-3.2 03/07/2025 11:51:27' -A arm64 -O u-boot -C gzip -T standalone -a 0x41E00200 -e 0x41E003F4 -d $<.gz $@
	rm $<.gz

clean:
	rm -f *.o *.elf zloader_3_2.bin get_boot_flag.bin zld_swapBootImage.bin do_boot.bin zloader_3_2_patched.bin $(TARGETS)