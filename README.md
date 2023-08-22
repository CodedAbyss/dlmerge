# dlmerge
## Merge many dynamic libraries into a single library

This would not have been possible without the work on [MemoryModule](https://github.com/fancycode/MemoryModule).

Merging two Dlls: `dlmerge a.dll b.dll -o c.dll`

Currently only works with DLLs

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
