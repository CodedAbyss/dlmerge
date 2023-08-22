# dlmerge
## Merge many dynamic libraries into a single binary _(and more)_

This would not have been possible without the work on [MemoryModule](https://github.com/fancycode/MemoryModule).

### Example Usage:

- Merging two Dlls: `dlmerge a.dll b.dll -o dynamic.dll`
- Merging three Dlls into a static library: `dlmerge a.dll b.dll c.dll -o static.lib`
- Generate .c file containing two Dlls: `dlmerge a.dll b.dll -o gen.c`
- Generate .o file containing three Dlls: `dlmerge a.dll b.dll -o gen.o`

Let's say you were compiling your code against 3 dll's like so: `gcc main.c a.dll b.dll c.dll -o main.exe`

You can link statically by doing the following:
> ```
> dlmerge a.dll b.dll c.dll -o all.lib
> gcc main.c all.lib -o main.exe
> ```
> or
> ```
> dlmerge a.dll b.dll c.dll -o all.c
> gcc main.c all.c -DLIB -o main.exe
> ```

## Notes
- When linking as a dll, the inner libraries are loaded and freed automatically via `DllMain`.
- When linking as a lib (like in the previous examples) inner libraries must be loaded and freed by the functions `dlmload` and `dlmclose`.
- By defining `LIB` the c file with compile `dlmload` and `dlmclose`
- By defining `DLL` the c file with compile `DllMain`
- When outputting an object file, `dlmload`, `dlmclose`, and `DllMain` are defined

An example of how to use a static linked merged library:
> ```c
> extern void dlmload();
> extern void dlmclose();
> void main() {
>     dlmload();
>     // your code here
>     dlmclose();
> }
> ```

## Disclaimers
- Currently only works with DLLs
- Relies on clang and zig lib (because I don't have the VS toolchain installed out of spite)

## Future Plans
- [x] support outputting .c / .o / .dll / .lib
- [ ] support various compilers / linkers
- [ ] auto-detect compilers / linkers
- [ ] support .so on mac & linux

## Mechanism:
1. We store the original bytes of the DLLs as constant byte arrays and load the libraries from memory.
2. On load, the inner DLLs are loaded, imports are resolved, and the master library redirects its exports to the inner DLLs' functions.

### The redirection works on the following principle:
- Exported functions are type-less memory addresses which we can jump to
- Calling a `void function(void)` is a jump, and doesn't affect parameter or return value registers

Conclusion - we can link functions with identical type signatures by calling `void functions(void)`

### For Instance:
```c
void (*ptr)();
int add(int a, int c) {
    ptr();
}
int add_real(int a, int b) {
    return a + b;
}
void main() {
    ptr = ((void(*)())add_real);
    printf("%d\n", add(6, 9)); // prints 15
}
```
