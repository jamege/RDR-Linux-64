#include <stddef.h>
#include <stdint.h>

extern void *dlopen(const char *filename, int flags);
extern char *dlerror(void);
extern int mallopt(int option, int value);

#define RTLD_NOW 2
#define RTLD_GLOBAL 0x100
#define M_BIONIC_SET_HEAP_TAGGING_LEVEL (-204)
#define M_HEAP_TAGGING_LEVEL_NONE 0

#define LIBCXX_PATH "/opt/rdr-pi-bionic/game/lib/libc++_shared.so"
#define RDR_PATH "/opt/rdr-pi-bionic/game/lib/librdr-local.so"

static long sys_write(int fd, const void *buf, unsigned long len)
{
    register long x0 __asm__("x0") = fd;
    register long x1 __asm__("x1") = (long)buf;
    register long x2 __asm__("x2") = (long)len;
    register long x8 __asm__("x8") = 64;
    __asm__ volatile("svc 0" : "+r"(x0) : "r"(x1), "r"(x2), "r"(x8) : "memory");
    return x0;
}

static unsigned long slen(const char *s)
{
    unsigned long n = 0;
    while (s && s[n]) ++n;
    return n;
}

static void out(const char *s)
{
    sys_write(2, s, slen(s));
}

static int wrapper_main(void)
{
    out("=== RDR Bionic wrapper v19 ===\n");
    out("[v19] disabling Bionic heap tagging...\n");
    if (!mallopt(M_BIONIC_SET_HEAP_TAGGING_LEVEL, M_HEAP_TAGGING_LEVEL_NONE))
        out("[v19] WARNING: heap-tagging request was rejected\n");
    else
        out("[v19] heap tagging disabled\n");

    out("[v19] loading packaged libc++_shared.so GLOBAL...\n");
    if (!dlopen(LIBCXX_PATH, RTLD_NOW | RTLD_GLOBAL)) {
        out("[v19] libc++ load failed: ");
        out(dlerror());
        out("\n");
        return 20;
    }

    out("[v19] loading allocator-isolated RDR...\n");
    if (!dlopen(RDR_PATH, RTLD_NOW | RTLD_GLOBAL)) {
        out("[v19] RDR load failed: ");
        out(dlerror());
        out("\n");
        return 30;
    }

    out("[v19] RDR loaded successfully\n");
    return 0;
}

__attribute__((noreturn)) void _start(void)
{
    int rc = wrapper_main();
    register long x0 __asm__("x0") = rc;
    register long x8 __asm__("x8") = 93;
    __asm__ volatile("svc 0" : : "r"(x0), "r"(x8) : "memory");
    __builtin_unreachable();
}

