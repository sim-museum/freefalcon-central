/*
 * FF_LINUX: correctness test for the resolve_nocase() path cache in
 * src/compat/linux_stubs.cpp.
 *
 * The cache must never make a lookup differ from the uncached resolver, in
 * particular around files the game CREATES at runtime (campaign saves,
 * logbook, debrief). Run it both ways and compare:
 *
 *   cd /tmp && ar x <build>/src/compat/libcompat.a linux_stubs.cpp.o
 *   g++ -o pathcache_test <ff>/tools/pathcache_test.cpp linux_stubs.cpp.o -lpthread
 *   ./pathcache_test                  # cache on  (default)
 *   FF_NO_PATHCACHE=1 ./pathcache_test  # cache off (reference behaviour)
 *
 * Both must print 0 failures and identical results.
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
extern "C" FILE* fopen_nocase(const char*, const char*);
extern "C" int open_nocase(const char*, int, int);
static int fails = 0;
static void chk(const char* what, bool ok) {
    printf("  %-58s %s\n", what, ok ? "PASS" : "FAIL");
    if (!ok) fails++;
}
static bool readable(const char* p, const char* expect) {
    FILE* f = fopen_nocase(p, "r"); if (!f) return false;
    char b[64] = {0}; size_t n = fread(b, 1, sizeof(b)-1, f); fclose(f); b[n] = 0;
    return strcmp(b, expect) == 0;
}
int main() {
    system("rm -rf /tmp/pctest/data && mkdir -p /tmp/pctest/data/SubDir");
    FILE* f = fopen("/tmp/pctest/data/SubDir/FILE.TXT", "w"); fputs("one", f); fclose(f);

    chk("1. mixed-case lookup resolves (cold, miss)",
        readable("/tmp/pctest/data\\subdir\\file.txt", "one"));
    chk("2. same lookup again (cache hit, same result)",
        readable("/tmp/pctest/data\\subdir\\file.txt", "one"));

    // Create a NEW file through the write path; a naive cache would have cached
    // the earlier negative and/or not seen the new name.
    chk("3. new-file lookup fails before creation",
        fopen_nocase("/tmp/pctest/data/SUBDIR/NEW.TXT", "r") == NULL);
    FILE* w = fopen_nocase("/tmp/pctest/data\\SubDir\\new.txt", "w");
    if (w) { fputs("two", w); fclose(w); }
    chk("4. just-created file is found via a different case",
        readable("/tmp/pctest/data/subdir/NEW.TXT", "two"));

    // Delete the cached target: the cache must not serve a stale hit.
    unlink("/tmp/pctest/data/SubDir/FILE.TXT");
    chk("5. deleted file no longer resolves (stale entry dropped)",
        fopen_nocase("/tmp/pctest/data\\subdir\\file.txt", "r") == NULL);

    // Recreate it with a DIFFERENT case and confirm we now get the new one.
    f = fopen("/tmp/pctest/data/SubDir/file.txt", "w"); fputs("three", f); fclose(f);
    chk("6. recreated (different case) file resolves to new content",
        readable("/tmp/pctest/data\\subdir\\FILE.TXT", "three"));

    // Same via open_nocase / O_CREAT.
    int fd = open_nocase("/tmp/pctest/data\\SubDir\\od.bin", O_WRONLY|O_CREAT|O_TRUNC, 0644);
    if (fd >= 0) { ssize_t r = write(fd, "four", 4); (void)r; close(fd); }
    chk("7. open_nocase O_CREAT file found via different case",
        readable("/tmp/pctest/data/subdir/OD.BIN", "four"));

    printf("  %d failure(s)\n", fails);
    return fails ? 1 : 0;
}
// minimal stubs so linux_stubs.cpp links standalone for this test
extern "C" void* FF_CreateDirectDraw7() { return 0; }
extern "C" { unsigned int vuxGameTime = 0; }
