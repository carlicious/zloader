#ifndef _HAVE_COMMON_H
#define _HAVE_COMMON_H

#define NULL (void *)0
#define get_boot_flag() (((int (*)())0x41E05028)())
#define getenv(name) (((const char *(*)(const char *))0x41E075B8)(name))
#define setenv(name,value) (((int (*)(const char *, const char *))0x41E075C4)(name,value))
#define strtoul(nptr,endptr,base) (((unsigned long (*)(const char *, char **, int))0x41E075D0)(nptr,endptr,base))
#define snprintf(str,size,...) (((int (*)(char *,unsigned long,...))0x41E076D8)(str,size,__VA_ARGS__))
#define printf(...) (((int (*)(...))0x41E0754C)(__VA_ARGS__))
#define do_boot() (((int (*)())0x41E05548)())
#define uboot_boot_kernel(part) (((int (*)(const char *))0x41E04604)(part))
#define uboot_save_env() (((int (*)())0x41E05704)())
#define g_dwBootFlag *(unsigned int *)0x41E23404

#endif /* _HAVE_COMMON_H */
