void *dlopen(const char *filename, int flags) {
    (void)filename;
    (void)flags;
    return (void*)0;
}

char *dlerror(void) {
    return (char*)0;
}
