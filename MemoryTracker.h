#pragma once
#include <cstddef>

class MemoryTracker {
private:
    inline static size_t activeAllocations = 0;
    inline static size_t totalAllocations = 0;

public:
    static void OnAlloc() {
        activeAllocations++;
        totalAllocations++;
    }

    static void OnFree() {
        if (activeAllocations > 0) {
            activeAllocations--;
        }
    }

    static size_t GetActiveAllocations() {
        return activeAllocations;
    }

    static size_t GetTotalAllocations() {
        return totalAllocations;
    }
};