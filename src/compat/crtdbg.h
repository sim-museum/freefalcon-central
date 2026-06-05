/* FreeFalcon Linux Port - crtdbg.h compatibility (no-op debug heap) */
#ifndef FF_COMPAT_CRTDBG_H
#define FF_COMPAT_CRTDBG_H
#ifdef FF_LINUX
#include <assert.h>
#include <stdlib.h>
#define _ASSERT(expr)  assert(expr)
#define _ASSERTE(expr) assert(expr)
#define _CrtSetDbgFlag(f) 0
#define _CrtDumpMemoryLeaks() 0
#define _CrtCheckMemory() 1
#define _malloc_dbg(size, type, file, line) malloc(size)
#define _free_dbg(ptr, type) free(ptr)
#define _CRTDBG_ALLOC_MEM_DF    0x01
#define _CRTDBG_LEAK_CHECK_DF   0x20
#define _CRTDBG_REPORT_FLAG     (-1)
#define _NORMAL_BLOCK 1
#define _CLIENT_BLOCK 4
#endif
#endif
