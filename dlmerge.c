#include <stdio.h>
#include "memmod.h"

#define ARG(n) (argv[n + ((n + 1 >= oloc) ? 3 : 1)])
void write_list(FILE *fp, int count, const char *fmt) {
    fprintf(fp, "{ ");
    for(int i = 0; i < count; i++) {
        if(i) fprintf(fp, ", ");
        fprintf(fp, fmt, i + 1);
    }
    fprintf(fp, " };\n");
}
int validate_name(const char *name) {
    char *n = (char*)name;
    for(; *n; n++) {
        if(!(*n >= 'A' && *n <= 'Z') && !(*n >= 'a' && *n <= 'z') && *n != '_')
            return 0;
    }
    return 1;
}
int main(int argc, char **argv) {
    if(argc == 1) {
        printf("usage: dlmerge <files> <options>\n"
               "options:\n"
               "    -o <file> | set output file\n");
        return 0;
    }
    char tmp[512];
    int oloc = 0;
    int count = argc - 1;
    for(int i = 1; i < argc; i++) {
        if(!strcmp(argv[i], "-o")) {
            oloc = i;
            count -= 2;
            break;
        }
    }
    if(!oloc || oloc == argc - 1) {
        printf("no output file specified\n");
        return 0;
    }
    if(count == 0) {
        printf("no input files\n");
        return 0;
    }
    size_t *sizes = malloc(sizeof(size_t) * count);
    void **bufs = malloc(sizeof(void*) * count);
    HMEMORYMODULE *mods = malloc(sizeof(HMEMORYMODULE) * count);
   
    char *outfile = argv[oloc + 1];
    char *ext = outfile;
    int mode = 0;
    for(int i = 0; outfile[i]; i++) {
        if(outfile[i] == '.')
            ext = outfile + i + 1;
    }

    if(!strcmp(ext, "c")) {
        mode = 1;
    } else if(!strcmp(ext, "o")) {
        mode = 2;
    } else if(!strcmp(ext, "dll")) {
        mode = 3;
    } else if(!strcmp(ext, "lib")) {
        mode = 4;
    }

    if(!mode) {
        printf("file type .%s not supported\n", ext);
        return 0;
    }

    if(mode == 1)
        snprintf(tmp, 512, "%s", outfile);
    else
        snprintf(tmp, 512, "%s/../tmp.c", argv[0]);
    FILE *dll = fopen(tmp, "wb");

    for(int i = 0; i < count; i++) {
        FILE *fp = fopen(ARG(i), "rb");
        fseek(fp, 0, SEEK_END);
        sizes[i] = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        bufs[i] = malloc(sizes[i]);
        fread(bufs[i], sizes[i], 1, fp);
        fclose(fp);
        mods[i] = MemoryLoadLibrary(bufs[i], sizes[i]);
    }

    fprintf(dll, "/* --------------------------------------- memmod.c --------------------------------------- */\n\n");
    snprintf(tmp, 512, "%s/../memmod.c", argv[0]);
    FILE *memmod = fopen(tmp, "rb");
    int len;
    while((len = fread(tmp, 1, 512, memmod))) {
        fwrite(tmp, 1, len, dll);
    }
    fclose(memmod);
    
    fprintf(dll, "\n/* --------------------------------------- %.*s.c --------------------------------------- */\n\n", (int)(ext - outfile - 1), outfile);
    fprintf(dll, "#define EXPORT extern __declspec(dllexport)\n");
    for(int i = 0; i < count; i++) {
        fprintf(dll, "static void (*addrs%d[%d])(void);\n", i + 1, MemoryGetExportCount(mods[i]));
    }
    for(int i = 0; i < count; i++) {
        int entry_count = MemoryGetExportCount(mods[i]);
        for(int n = 0; n < entry_count; n++) {
            const char *name = MemoryGetExportName(mods[i], n);
            if(validate_name(name))
                fprintf(dll, "EXPORT void %s() { addrs%d[%d](); }\n", name, i + 1, n);
        }
    }
    for(int i = 0; i < count; i++) {
        int entry_count = MemoryGetExportCount(mods[i]);
        fprintf(dll, "const unsigned char dll%d[%lld] = {", i + 1, sizes[i]);
        for(int n = 0; n < sizes[i]; n++) {
            if(n % 16 == 0) fprintf(dll, "\n   ");
            fprintf(dll, "0x%02x,", ((unsigned char*)bufs[i])[n]);
        }
        fprintf(dll, "\n};\n");
    }
    fprintf(dll,
        "HMEMORYMODULE mods[%d];\n"
        "void *addrs[%d] = ", count, count);
    write_list(dll, count, "addrs%d");
    fprintf(dll, "const size_t sizes[%d] = ", count);
    write_list(dll, count, "sizeof(dll%d)");
    fprintf(dll, "const unsigned char *dlls[%d] = ", count);
    write_list(dll, count, "dll%d"); 
    fprintf(dll, 
        "static void _dlmload() {\n"
        "    for(int i = 0; i < %d; i++) {\n"
        "        mods[i] = MemoryLoadLibrary(dlls[i], sizes[i]);\n"
        "        int count = MemoryGetExportCount(mods[i]);\n"
        "        for(int n = 0; n < count; n++) {\n"
        "            ((void**)addrs[i])[n] = MemoryGetExportAddr(mods[i], n);\n"
        "        }\n"
        "    }\n"
        "}\n"
        "static void _dlmclose() {\n"
        "    for(int i = 0; i < %d; i++) {\n"
        "        MemoryFreeLibrary(mods[i]);\n"
        "    }\n"
        "}\n", count, count);
    fprintf(dll, 
        "#ifdef DLL\n"
        "BOOL WINAPI DllMain(HINSTANCE handle, DWORD reason, LPVOID unused) {\n"
        "    switch(reason) {\n"
        "    case DLL_PROCESS_ATTACH:\n"
        "        _dlmload();\n"
        "        break;\n"
        "    case DLL_PROCESS_DETACH:\n"
        "        _dlmclose();\n"
        "        break;\n"
        "    }\n"
        "    return TRUE;\n"
        "}\n"
        "#endif\n");
    fprintf(dll, 
        "#ifdef LIB\n"
        "EXPORT void dlmload() {\n"
        "    _dlmload();\n"
        "}\n"
        "EXPORT void dlmclose() {\n"
        "    _dlmclose();\n"
        "}\n"
        "#endif\n");
    fclose(dll);
    if(mode == 1) goto end;
    switch(mode) {
        case 2: snprintf(tmp, 512, "clang -c %s/../tmp.c -DLIB -DDLL -o %s", argv[0], outfile); break;
        case 3: snprintf(tmp, 512, "clang %s/../tmp.c -DDLL -shared -o %s", argv[0], outfile); break;
        case 4: snprintf(tmp, 512, "clang -c %s/../tmp.c -DLIB -o %s/../tmp.o && zig lib /out:%s %s/../tmp.o", argv[0], argv[0], outfile, argv[0]); break;
        default: return 0;
    }
    system(tmp);
end:
    printf("%s created\n", outfile);
    return 0;
}
