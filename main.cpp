#include <orbis/kernel.h>
#include <orbis/libkernel.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

void PatchMemory(uint64_t address, const void* data, size_t size) {
    uintptr_t page_size = sysconf(_SC_PAGESIZE);
    uintptr_t base = address & ~(page_size - 1);
    if (mprotect((void*)base, page_size, PROT_READ | PROT_WRITE | PROT_EXEC) == 0) {
        memcpy((void*)address, data, size);
    }
}

void* TrainerThread(void* arg) {
    sleep(6);
    while (true) {
        usleep(500000);
    }
    return nullptr;
}

extern "C" int module_start(size_t argc, const void* args) {
    ScePthread thread;
    scePthreadCreate(&thread, NULL, TrainerThread, NULL, "IsaacTrainerThread");
    return 0;
}

extern "C" int module_stop(size_t argc, const void* args) {
    return 0;
}
