# zloader patch

Patched zloader found in Zyxel EX5601 (Odido T-56) routers allowing for unsigned firmware and easy dual boot.

## tl;dr

Just download `bl2.img`, `fip.bin`, `zloader.bin.gz.uImage_patched` or run `make` to construct
the latter yourself. The output should look something like this:
```
dd if=zloader.bin.gz.uImage bs=64 skip=1 | gzip -d -c > zloader_3_2.bin || true
372+1 records in
372+1 records out
23868 bytes (24 kB, 23 KiB) copied, 0.00102806 s, 23.2 MB/s
aarch64-linux-gnu-gcc -nostartfiles -nostdlib -c -ffreestanding -fomit-frame-pointer -Os -Wall get_boot_flag.c -o get_boot_flag.o
aarch64-linux-gnu-ld -dT LinkerScript -Ttext 0x41E05028 get_boot_flag.o -o get_boot_flag.elf
aarch64-linux-gnu-objcopy -j .text -O binary get_boot_flag.elf get_boot_flag.bin
aarch64-linux-gnu-gcc -nostartfiles -nostdlib -c -ffreestanding -fomit-frame-pointer -Os -Wall zld_swapBootImage.c -o zld_swapBootImage.o
aarch64-linux-gnu-ld -dT LinkerScript -Ttext 0x41E05314 zld_swapBootImage.o -o zld_swapBootImage.elf
aarch64-linux-gnu-objcopy -j .text -O binary zld_swapBootImage.elf zld_swapBootImage.bin
aarch64-linux-gnu-gcc -nostartfiles -nostdlib -c -ffreestanding -fomit-frame-pointer -Os -Wall do_boot.c -o do_boot.o
aarch64-linux-gnu-ld -dT LinkerScript -Ttext 0x41E051A8 do_boot.o -o do_boot.elf
aarch64-linux-gnu-objcopy -j .text -O binary do_boot.elf do_boot.bin
php patch.php zloader_3_2.bin zloader_3_2_patched.bin
gzip -f --best -k zloader_3_2_patched.bin
mkimage -n 'zld-3.2 03/07/2025 11:51:27' -A arm64 -O u-boot -C gzip -T standalone -a 0x41E00200 -e 0x41E003F4 -d zloader_3_2_patched.bin.gz zloader.bin.gz.uImage_patched
Image Name:   zld-3.2 03/07/2025 11:51:27
Created:      Wed Sep 17 23:29:12 2025
Image Type:   AArch64 U-Boot Standalone Program (gzip compressed)
Data Size:    23995 Bytes = 23.43 KiB = 0.02 MiB
Load Address: 41e00200
Entry Point:  41e003f4
rm zloader_3_2_patched.bin.gz
```

Upload the files to `/tmp` on the device. Flash them with the following commands:
```
mtd erase BL2
mtd write /tmp/bl2.img BL2

mtd erase FIP
mtd write /tmp/fip.bin FIP

mtd erase zloader
mtd write /tmp/zloader.bin.gz.uImage_patched zloader

sync
```

Set the boot partition:
```
fw_setenv boot_flag 0
```
for `ubi`, or
```
fw_setenv boot_flag 1
```
for `ubi2`.

## Why?

I recently subscribed to Odido (Dutch ISP) and received a T-56 router (branded Zyxel EX5601).
The hardware is excellent but the software is a mess (as usual).
Therefore I opted to flash OpenWrt.

Unfortunately, my device got updated to firmware version `V5.70(ACEA.0)T56C_b10_0410`.
The OpenWrt wiki states it explicitly as not supported, whereas previous versions are.
I decided to spend some time researching because the security of these kinds of devices
is usually terrible, so there must still be some avenues left to get it to run OpenWrt.

We used to be able to plug a serial adapter during the boot process and use [mtk_uartboot](https://https://github.com/981213/mtk_uartboot)
to talk to the MediaTek boot loader, load custom code and run it. Unfortunately, since the
new firmware update, an efuse is blown that forever disables it.
Interestingly, the boot process can still be interrupted. When we do so, we get dropped to
a prompt implemented in `zloader`. If the engineering flag is set, it allows for all sorts
of cool things, like raw reading and writing to RAM.

Naturally, I spent some time researching `zloader` and found that it checks an RSA signature
over the root file system, but _not_ the kernel. This is already pretty exploitable but it
turns out that `zloader` itself is contained in a U-Boot legacy image format, which doesn't
support cryptographic signatures. So patching `zloader` seems to be the obvious way forward.

## How?

The T-56 has two firmware banks that allows for failover if an update is unsuccessful.
There's a function, which I dubbed `get_boot_flag()`, that reads the so-called `zyfwinfo`
for each bank and decides which firmware to boot based on the boot priority element.
`get_boot_flag()` also validates the RSA signature. Clearly, this check must be removed.

I figured that I may want to boot the stock firmware some day, and updating `zyfwinfo`
(including its checksum) seems like a pain. So I ended up re-implementing `get_boot_flag()`,
which now just ignores `zyfwinfo` entirely and just boot whatever based on the `boot_flag`
variable. I also patched the `ATSW` command so that it updates `boot_flag` and doesn't touch
`zyfwinfo`.

Furthermore, `zloader` tries to find the kernel command line by looking it up in the kernel's
device tree. This makes it incompatible with OpenWrt, because this property isn't present.
I patched the code that works around the issue.

Finally, I like engineering mode. It allows me to do dirty things like raw read/write from/to
RAM. I've patched `zloader` so that it's always enabled, regardless of `EngDebugFlag`. Might
save me a brick one day.
