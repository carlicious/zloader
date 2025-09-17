#include "common.h"

int get_boot_flag_replace() {
    int dwBootFlag;
    const char *szBootFlag;

    if ((szBootFlag = getenv("boot_flag")) == NULL) {
        dwBootFlag = 0;
    } else {
        dwBootFlag = strtoul(szBootFlag, NULL, 0);
    }

    if (dwBootFlag != 0 && dwBootFlag != 1) {
        dwBootFlag = -1;
    }

    return dwBootFlag;
}
