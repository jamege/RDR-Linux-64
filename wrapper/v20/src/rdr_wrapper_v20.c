#include <stddef.h>
#include <stdint.h>

extern void *dlsym(void *, const char *);
extern char *dlerror(void);
extern int mallopt(int, int);

#define RTLD_NOW 2
#define RTLD_DEFAULT ((void *)0)
#define ANDROID_NAMESPACE_TYPE_ISOLATED 1
#define ANDROID_DLEXT_USE_NAMESPACE 0x200
#define SEARCH_PATH "/opt/rdr-pi-bionic/game/lib:/opt/rdr-pi-bionic/runtime/lib64"
#define RDR_PATH "/opt/rdr-pi-bionic/game/lib/librdr.so"

typedef struct android_namespace_t android_namespace_t;
struct android_dlextinfo {
    uint64_t flags;
    void *reserved_addr;
    size_t reserved_size;
    int relro_fd;
    int library_fd;
    long library_fd_offset;
    android_namespace_t *library_namespace;
};
typedef android_namespace_t *(*create_namespace_fn)(const char *, const char *,
    const char *, uint64_t, const char *, android_namespace_t *);
typedef void *(*android_dlopen_ext_fn)(const char *, int,
    const struct android_dlextinfo *);

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
static void out(const char *s) { sys_write(2, s, slen(s)); }
static void *sym(const char *name, const char *fallback)
{
    void *p = dlsym(RTLD_DEFAULT, name);
    return p ? p : dlsym(RTLD_DEFAULT, fallback);
}

static int wrapper_main(void)
{
    create_namespace_fn create_namespace;
    android_dlopen_ext_fn android_dlopen_ext;
    android_namespace_t *ns;
    struct android_dlextinfo ext = {0};

    out("=== RDR Bionic wrapper v20 ===\n");
    out("[v20] disabling Bionic heap tagging...\n");
    if (mallopt(-204, 0)) out("[v20] heap tagging disabled\n");
    else out("[v20] WARNING: heap-tagging request was rejected\n");

    create_namespace = (create_namespace_fn)sym(
        "android_create_namespace", "__loader_android_create_namespace");
    android_dlopen_ext = (android_dlopen_ext_fn)sym(
        "android_dlopen_ext", "__loader_android_dlopen_ext");
    if (!create_namespace || !android_dlopen_ext) {
        out("[v20] Android namespace API unavailable\n");
        return 20;
    }

    out("[v20] creating isolated game namespace...\n");
    ns = create_namespace("rdr_game", SEARCH_PATH, SEARCH_PATH,
        ANDROID_NAMESPACE_TYPE_ISOLATED, SEARCH_PATH, 0);
    if (!ns) {
        out("[v20] namespace creation failed: "); out(dlerror()); out("\n");
        return 21;
    }

    ext.flags = ANDROID_DLEXT_USE_NAMESPACE;
    ext.library_namespace = ns;
    out("[v20] loading RDR and packaged libc++ in one namespace...\n");
    if (!android_dlopen_ext(RDR_PATH, RTLD_NOW, &ext)) {
        out("[v20] RDR load failed: "); out(dlerror()); out("\n");
        return 30;
    }
    out("[v20] RDR loaded successfully\n");
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
