#include <orbis/kernel.h>
#include <orbis/libkernel.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <atomic>

// --- 1. Базовые структуры и константы ---
#define TARGET_FIRMWARE_CHECK 1
static std::atomic_bool g_TrainerRunning{false};
static ScePthread g_TrainerThread{};
static bool g_TrainerThreadCreated = false;

// --- 2. Системные функции управления памятью и защитой (1-5) ---
inline size_t GetSystemPageSize() {
    return (size_t)sysconf(_SC_PAGESIZE);
}

inline uintptr_t AlignToPageDown(uintptr_t addr, size_t pageSize) {
    return addr & ~(pageSize - 1);
}

inline uintptr_t AlignToPageUp(uintptr_t addr, size_t pageSize) {
    return (addr + pageSize - 1) & ~(pageSize - 1);
}

bool ProtectMemoryRegion(uint64_t address, size_t size, int prot) {
    if (size == 0) {
        return true;
    }

    size_t pageSize = GetSystemPageSize();
    uintptr_t start = AlignToPageDown((uintptr_t)address, pageSize);
    uintptr_t end = AlignToPageUp((uintptr_t)address + size, pageSize);
    size_t regionSize = end - start;

    if (regionSize == 0) {
        return false;
    }

    return mprotect((void*)start, regionSize, prot) == 0;
}

void SafeWriteMemory(uint64_t destination, const void* source, size_t size) {
    if (source == nullptr || size == 0 || destination == 0) {
        return;
    }

    if (ProtectMemoryRegion(destination, size, PROT_READ | PROT_WRITE | PROT_EXEC)) {
        memcpy((void*)destination, source, size);
    }
}

void SafeReadMemory(uint64_t source, void* destination, size_t size) {
    if (destination == nullptr || source == 0 || size == 0) {
        return;
    }

    memcpy(destination, (void*)source, size);
}

// --- 3. Функции патчинга значений (6-9) ---
void PatchInt32(uint64_t address, int32_t value) {
    SafeWriteMemory(address, &value, sizeof(value));
}

void PatchFloat(uint64_t address, float value) {
    SafeWriteMemory(address, &value, sizeof(value));
}

void NopMemoryRegion(uint64_t address, size_t size) {
    if (size == 0 || address == 0) {
        return;
    }

    uint8_t* nopBuffer = new uint8_t[size];
    memset(nopBuffer, 0x90, size); // Инструкция NOP для x86_64
    SafeWriteMemory(address, nopBuffer, size);
    delete[] nopBuffer;
}

bool CheckMemoryValid(uint64_t address) {
    if (address == 0) {
        return false;
    }

    volatile uint32_t val = 0;
    SafeReadMemory(address, (void*)&val, sizeof(val));
    return val != 0xDEADBEEF;
}

// --- 4. Вспомогательные утилиты потоков и ожидания (10-13) ---
void TrainerSleep(unsigned int milliseconds) {
    usleep(milliseconds * 1000);
}

bool IsTrainerActive() {
    return g_TrainerRunning.load(std::memory_order_acquire);
}

void SignalTrainerStop() {
    g_TrainerRunning.store(false, std::memory_order_release);
}

void InitializeTrainerSubsystem() {
    g_TrainerRunning.store(true, std::memory_order_release);
}

// --- 5. Основная логика трейнера и цикл удержания (14-15) ---
void ExecuteResourcePops() {
    // Здесь выполняются постоянные проверки и заморозка ресурсов (монеты, ключи, бомбы)
    // Пример структуры под офсеты Айзека:
    // PatchInt32(BaseAddress + 0xOffsetCoins, 99);
}

void* MainTrainerLoop(void* arg) {
    // Обязательная задержка перед стартом, чтобы игра успела распаковать модули в память
    sleep(7);
    InitializeTrainerSubsystem();

    while (g_TrainerRunning.load(std::memory_order_acquire)) {
        ExecuteResourcePops();
        TrainerSleep(250); // Проверка 4 раза в секунду для снижения нагрузки на процессор PS4
    }

    return nullptr;
}

// --- 6. Системные точки входа GoldHEN PRX (16-17) ---
extern "C" int module_start(size_t argc, const void* args) {
    if (g_TrainerThreadCreated) {
        return 0;
    }

    int ret = scePthreadCreate(&g_TrainerThread, NULL, MainTrainerLoop, NULL, "IsaacTrainerMainThread");
    if (ret == 0) {
        g_TrainerThreadCreated = true;
        return 0;
    }

    return -1;
}

extern "C" int module_stop(size_t argc, const void* args) {
    SignalTrainerStop();

    if (g_TrainerThreadCreated) {
        scePthreadJoin(g_TrainerThread, NULL);
        g_TrainerThreadCreated = false;
    }

    return 0;
}
