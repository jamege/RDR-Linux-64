#include <stddef.h>
#include <stdint.h>

extern void *dlopen(const char *filename, int flags);
extern char *dlerror(void);

#define RTLD_NOW 2
#define RTLD_GLOBAL 0x100

#ifndef TARGET_PATH
#define TARGET_PATH "/opt/rdr-pi-bionic/work/v15/librdr-diagbreak.so"
#endif

static long sys_write(int fd, const void *buf, unsigned long len) {
    register long x0 __asm__("x0") = fd;
    register const void *x1 __asm__("x1") = buf;
    register unsigned long x2 __asm__("x2") = len;
    register long x8 __asm__("x8") = 64;
    __asm__ volatile("svc 0" : "+r"(x0) : "r"(x1), "r"(x2), "r"(x8) : "memory");
    return x0;
}

__attribute__((noreturn))
static void sys_exit(int code) {
    register long x0 __asm__("x0") = code;
    register long x8 __asm__("x8") = 93;
    __asm__ volatile("svc 0" : : "r"(x0), "r"(x8) : "memory");
    __builtin_unreachable();
}

static unsigned long slen(const char *s) {
    unsigned long n = 0;
    if (!s) return 0;
    while (s[n]) n++;
    return n;
}

static void out(const char *s) {
    sys_write(2, s, slen(s));
}

void _start(void) {
    out("=== RDR BIONIC PROBE v15 ===\n");
    out("[v15] loading copied librdr.so with diagTerminate entry patched to BRK #0\n");

    void *h = dlopen(TARGET_PATH, RTLD_NOW | RTLD_GLOBAL);
    if (!h) {
        out("[v15] dlopen failed: ");
        char *e = dlerror();
        out(e ? e : "(no dlerror)");
        out("\n");
        sys_exit(20);
    }

    out("[v15] SUCCESS: patched librdr.so loaded without reaching diagTerminate\n");
    sys_exit(0);
}
