
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* const symbol_names[] =
{
    "GetSDKVersion",
    "GetPluginVersion",
    "InitializePlugin",
    "FinalizePlugin",
    0,
};

static void die(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);

    exit(2);
}

static void usage(const char* my_name)
{
    printf("Usage:  %s PLUGIN1.so PLUGIN2.so ...\n", my_name);
    exit(1);
}

static void test_load(const char* plugin)
{
    void *handle = dlopen(plugin, RTLD_NOW);
    if (handle == 0)
        die("Cannot load %s: %s", plugin, dlerror());

    for (int i=0; symbol_names[i]; ++i)
    {
        const char* symbol_name = symbol_names[i];
        void *symbol = dlsym(handle, symbol_name);
        if (symbol == 0)
            die("Symbol %s not found in %s", symbol_name, plugin);
    }

    dlclose(handle);
}

int main(int argc, char* argv[])
{
    if (argc <= 1)
        usage(argv[0]);
    if (0 == strcmp(argv[1], "-h") || 0 == strcmp(argv[1], "--help"))
        usage(argv[0]);

    for (int i=1; i<argc; ++i)
        test_load(argv[i]);
    return 0;
}

