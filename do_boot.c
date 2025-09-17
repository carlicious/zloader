#include "common.h"

int do_boot_replace() {
    int dwStatus;
    char szBootArgs[256];
    const char *szBootPartition;

    dwStatus = do_boot();
    if (g_dwBootFlag == 0 || g_dwBootFlag == 1) {
        szBootPartition = g_dwBootFlag ? "ubi2" : "ubi";
        snprintf(szBootArgs, sizeof(szBootArgs), "console=ttyS0,115200n1 loglevel=8 earlycon=uart8250,mmio32,0x11002000 rootubi=%s", szBootPartition);
        setenv("bootargs", szBootArgs);
        uboot_boot_kernel(szBootPartition);
    }
    return dwStatus;
}