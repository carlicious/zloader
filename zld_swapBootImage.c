#include "common.h"

int zld_swapBootImage_replace() {
    char szNewBootFlag[8];
    int dwNewBootFlag;

    dwNewBootFlag = !g_dwBootFlag;
    snprintf(szNewBootFlag, sizeof(szNewBootFlag), "%d", dwNewBootFlag);
    setenv("boot_flag", szNewBootFlag);
    uboot_save_env();
    g_dwBootFlag = dwNewBootFlag;

    printf("next boot partition will be %s\n", dwNewBootFlag ? "ubi2" : "ubi");
    return 0;
}
