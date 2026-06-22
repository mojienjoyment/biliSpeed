// Minimal C++ runtime stubs - no libc++ dependency
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

extern "C" {
// Thread-safe static local variable guards (simplified, non-thread-safe is fine for Zygisk)
int  __cxa_guard_acquire(uint64_t* g) { if (*g & 1) return 0; *g |= 1; return 1; }
void __cxa_guard_release(uint64_t*)   {}
void __cxa_guard_abort(uint64_t*)     {}
void __cxa_pure_virtual()             {}
}

void* operator new(size_t s)          { return malloc(s); }
void* operator new[](size_t s)        { return malloc(s); }
void  operator delete(void* p)        noexcept { free(p); }
void  operator delete[](void* p)      noexcept { free(p); }
