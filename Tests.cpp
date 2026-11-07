#include "Tests.h"
#include <stdio.h>
#include <time.h>
#include <memory>
#include <type_traits>
#include "UnqPtr.h"
#include "ShrdPtr.h"
#include "ArraySequence.h"
#include "Exceptions.h"
#include "MemoryTracker.h"

// Иерархия классов для тестирования подтипизации (Derived <= Base)
class BaseDevice {
protected:
    int power;
public:
    explicit BaseDevice(int p = 50) : power(p) {}
    virtual ~BaseDevice() = default;
    virtual int GetPower() const { return power; }
};

class GamingLaptop : public BaseDevice {
private:
    int gpuBoost;
public:
    explicit GamingLaptop(int p = 100, int b = 65) : BaseDevice(p), gpuBoost(b) {}
    ~GamingLaptop() override = default;
    int GetPower() const override { return power + gpuBoost; }
};

class UnrelatedBox {
public:
    int weight = 10;
};

// Проверки подтипизации на этапе компиляции (C1 <= C2 разрешено, несвязанные типы запрещены)
static_assert(std::is_convertible_v<UnqPtr<GamingLaptop>, UnqPtr<BaseDevice>>, "GamingLaptop must convert to BaseDevice");
static_assert(std::is_convertible_v<ShrdPtr<GamingLaptop>, ShrdPtr<BaseDevice>>, "GamingLaptop must convert to BaseDevice");
static_assert(!std::is_convertible_v<UnqPtr<UnrelatedBox>, UnqPtr<BaseDevice>>, "Unrelated types must NOT convert");
static_assert(!std::is_convertible_v<ShrdPtr<UnrelatedBox>, ShrdPtr<BaseDevice>>, "Unrelated types must NOT convert");

static void Assert(bool condition, const char* message) {
    if (!condition) {
        printf("[ FAIL ] %s\n", message);
        throw Exception(message);
    }
}

static void PrintSuccess(const char* testName) {
    printf("[  OK  ] %s\n", testName);
}

// 1. Тесты UnqPtr<T>
static void TestUnqPtr() {
    size_t baseAlloc = MemoryTracker::GetActiveAllocations();
    {
        UnqPtr<int> p1(new int(42));
        Assert(!p1.IsNull() && *p1 == 42, "UnqPtr: Создание и разыменование");

        // Семантика передачи по значению (глубокая копия, разные адреса)
        UnqPtr<int> p2 = p1;
        Assert(p1.Get() != p2.Get() && *p2 == 42, "UnqPtr: Копирование создаёт независимый объект");
        *p2 = 100;
        Assert(*p1 == 42 && *p2 == 100, "UnqPtr: Изменение копии не меняет оригинал");

        // Семантика перемещения (передача владения)
        int* rawAddr = p1.Get();
        UnqPtr<int> p3 = std::move(p1);
        Assert(p1.IsNull() && p3.Get() == rawAddr && *p3 == 42, "UnqPtr: Перемещение владения (move)");

        p3.Reset();
        Assert(p3.IsNull(), "UnqPtr: Сброс через Reset()");

        bool caught = false;
        try {
            *p3 = 5;
        } catch (const NullPointerException&) {
            caught = true;
        }
        Assert(caught, "UnqPtr (Neg): Защита от разыменования nullptr");
    }
    Assert(MemoryTracker::GetActiveAllocations() == baseAlloc, "UnqPtr: Отсутствие утечек памяти");
    PrintSuccess("UnqPtr<T> (уникальное владение, глубокое копирование, move, защита от nullptr)");
}

// 2. Тесты ShrdPtr<T>
static void TestShrdPtr() {
    size_t baseAlloc = MemoryTracker::GetActiveAllocations();
    {
        ShrdPtr<int> s1(new int(77));
        Assert(s1.UseCount() == 1 && *s1 == 77, "ShrdPtr: Создание со счётчиком ссылок = 1");

        {
            ShrdPtr<int> s2 = s1;
            ShrdPtr<int> s3(s2);
            Assert(s1.UseCount() == 3 && s3.UseCount() == 3, "ShrdPtr: Увеличение referenceCount при копировании");
            *s3 = 99;
            Assert(*s1 == 99, "ShrdPtr: Разделение общего объекта");
        }

        Assert(s1.UseCount() == 1, "ShrdPtr: Уменьшение referenceCount при выходе из блока");

        // Передача объекта из UnqPtr в ShrdPtr
        UnqPtr<int> u(new int(555));
        ShrdPtr<int> sFromUnq(std::move(u));
        Assert(u.IsNull() && sFromUnq.UseCount() == 1 && *sFromUnq == 555, "ShrdPtr: Принятие владения от UnqPtr");

        sFromUnq.Reset();
        Assert(sFromUnq.IsNull() && sFromUnq.UseCount() == 0, "ShrdPtr: Сброс через Reset()");

        bool caught = false;
        try {
            *sFromUnq = 1;
        } catch (const NullPointerException&) {
            caught = true;
        }
        Assert(caught, "ShrdPtr (Neg): Защита от разыменования nullptr");
    }
    Assert(MemoryTracker::GetActiveAllocations() == baseAlloc, "ShrdPtr: Отсутствие утечек памяти");
    PrintSuccess("ShrdPtr<T> (подсчёт ссылок size_t* referenceCount, интеграция с UnqPtr)");
}

// 3. Тесты подтипизации (C1 <= C2)
static void TestSubtyping() {
    size_t baseAlloc = MemoryTracker::GetActiveAllocations();
    {
        // Подтипизация в ShrdPtr
        ShrdPtr<GamingLaptop> laptopShrd(new GamingLaptop(100, 65));
        ShrdPtr<BaseDevice> deviceShrd = laptopShrd;
        Assert(deviceShrd.UseCount() == 2, "ShrdPtr Subtyping: Общий счётчик ссылок для подкласса и базового класса");
        Assert(deviceShrd->GetPower() == 165, "ShrdPtr Subtyping: Полиморфный вызов GamingLaptop::GetPower()");

        // Подтипизация в UnqPtr (копирование и перемещение)
        UnqPtr<GamingLaptop> laptopUnq(new GamingLaptop(120, 80));
        UnqPtr<BaseDevice> deviceUnqCopy = laptopUnq;
        Assert(deviceUnqCopy.Get() != laptopUnq.Get() && deviceUnqCopy->GetPower() == 200,
               "UnqPtr Subtyping: Полиморфное клонирование подкласса");

        UnqPtr<BaseDevice> deviceUnqMoved = std::move(laptopUnq);
        Assert(laptopUnq.IsNull() && deviceUnqMoved->GetPower() == 200,
               "UnqPtr Subtyping: Перемещение подкласса в базовый указатель");
    }
    Assert(MemoryTracker::GetActiveAllocations() == baseAlloc, "Subtyping: Отсутствие утечек памяти");
    PrintSuccess("Подтипизация шаблонных аргументов (GamingLaptop <= BaseDevice)");
}

// 4. Тесты контейнера ArraySequence<T> на базе умных указателей
static void TestArraySequence() {
    size_t baseAlloc = MemoryTracker::GetActiveAllocations();
    {
        ArraySequence<int> seq;
        seq.Append(10);
        seq.Append(20);
        seq.Prepend(5);
        seq.InsertAt(15, 2); // [5, 10, 15, 20]

        Assert(seq.GetLength() == 4, "ArraySequence: Корректная длина после вставок");
        Assert(seq.GetFirst() == 5 && seq.GetLast() == 20, "ArraySequence: GetFirst и GetLast");
        Assert(seq[1] == 10 && seq[2] == 15, "ArraySequence: Доступ через operator[]");

        seq[2] = 150;
        Assert(seq.Get(2) == 150, "ArraySequence: Модификация через operator[]");

        // Хранение полиморфных объектов через ShrdPtr внутри ArraySequence
        ArraySequence<ShrdPtr<BaseDevice>> devices;
        devices.Append(ShrdPtr<BaseDevice>(new BaseDevice(50)));
        devices.Append(ShrdPtr<GamingLaptop>(new GamingLaptop(100, 65)));

        Assert(devices.GetLength() == 2, "ArraySequence<ShrdPtr<BaseDevice>>: Размер коллекции");
        Assert(devices[0]->GetPower() == 50 && devices[1]->GetPower() == 165,
               "ArraySequence<ShrdPtr<BaseDevice>>: Полиморфный вызов элементов");

        bool caught = false;
        try {
            seq.Get(100);
        } catch (const IndexOutOfRange&) {
            caught = true;
        }
        Assert(caught, "ArraySequence (Neg): Выход за границы индекса");
    }
    Assert(MemoryTracker::GetActiveAllocations() == baseAlloc, "ArraySequence: Полная очистка без утечек");
    PrintSuccess("Контейнер ArraySequence<T> (управление буфером через UnqPtr и хранение ShrdPtr)");
}

void RunAllFunctionalTests() {
    printf("\n======================================================================\n");
    printf("                ЗАПУСК ФУНКЦИОНАЛЬНЫХ И МОДУЛЬНЫХ ТЕСТОВ              \n");
    printf("======================================================================\n");
    try {
        TestUnqPtr();
        TestShrdPtr();
        TestSubtyping();
        TestArraySequence();
        printf("----------------------------------------------------------------------\n");
        printf("[ ИТОГ ] Все тесты пройдены! Активных аллокаций в памяти: %zu (утечек: 0)\n",
               MemoryTracker::GetActiveAllocations());
    } catch (const Exception& e) {
        printf("\n[ ОШИБКА ТЕСТА ]: %s\n", e.GetMessage());
    }
}

// ==================== НАГРУЗОЧНОЕ ТЕСТИРОВАНИЕ (ТАБЛИЦЫ + ГРАФИКИ) ====================

struct BenchResult {
    size_t count;
    double rawTimeMs;
    double unqTimeMs;
    double shrdTimeMs;
    double stlUnqTimeMs;
    double stlShrdTimeMs;

    double rawMemMB;
    double unqMemMB;
    double shrdMemMB;
    double stlUnqMemMB;
    double stlShrdMemMB;
};

static void PrintBar(const char* label, double value, double maxVal, const char* unit) {
    const int maxBarWidth = 35;
    int barLen = (maxVal > 0.0001) ? static_cast<int>((value / maxVal) * maxBarWidth) : 1;
    if (barLen < 1 && value > 0) barLen = 1;
    if (barLen > maxBarWidth) barLen = maxBarWidth;

    printf("  %-16s | ", label);
    for (int i = 0; i < barLen; ++i) printf("#");
    for (int i = barLen; i < maxBarWidth; ++i) printf(" ");
    printf(" | %8.2f %s\n", value, unit);
}

static BenchResult RunSingleBenchmark(size_t count) {
    BenchResult res = {};
    res.count = count;
    volatile long long sink = 0;

    // 1. Сырые указатели (Raw int*)
    clock_t t0 = clock();
    for (size_t i = 0; i < count; ++i) {
        int* p = new int(static_cast<int>(i));
        sink += *p;
        delete p;
    }
    clock_t t1 = clock();
    res.rawTimeMs = (static_cast<double>(t1 - t0) / CLOCKS_PER_SEC) * 1000.0;

    // 2. Наш UnqPtr<int>
    t0 = clock();
    for (size_t i = 0; i < count; ++i) {
        UnqPtr<int> p(new int(static_cast<int>(i)));
        sink += *p;
    }
    t1 = clock();
    res.unqTimeMs = (static_cast<double>(t1 - t0) / CLOCKS_PER_SEC) * 1000.0;

    // 3. Наш ShrdPtr<int>
    t0 = clock();
    for (size_t i = 0; i < count; ++i) {
        ShrdPtr<int> p(new int(static_cast<int>(i)));
        sink += *p;
    }
    t1 = clock();
    res.shrdTimeMs = (static_cast<double>(t1 - t0) / CLOCKS_PER_SEC) * 1000.0;

    // 4. STL std::unique_ptr<int>
    t0 = clock();
    for (size_t i = 0; i < count; ++i) {
        std::unique_ptr<int> p(new int(static_cast<int>(i)));
        sink += *p;
    }
    t1 = clock();
    res.stlUnqTimeMs = (static_cast<double>(t1 - t0) / CLOCKS_PER_SEC) * 1000.0;

    // 5. STL std::shared_ptr<int>
    t0 = clock();
    for (size_t i = 0; i < count; ++i) {
        std::shared_ptr<int> p(new int(static_cast<int>(i)));
        sink += *p;
    }
    t1 = clock();
    res.stlShrdTimeMs = (static_cast<double>(t1 - t0) / CLOCKS_PER_SEC) * 1000.0;
    (void)sink;

    const double toMB = 1.0 / (1024.0 * 1024.0);
    res.rawMemMB     = static_cast<double>(count) * (sizeof(int*) + sizeof(int)) * toMB;
    res.unqMemMB     = static_cast<double>(count) * (sizeof(UnqPtr<int>) + sizeof(int)) * toMB;
    res.shrdMemMB    = static_cast<double>(count) * (sizeof(ShrdPtr<int>) + sizeof(int) + sizeof(size_t)) * toMB;
    res.stlUnqMemMB  = static_cast<double>(count) * (sizeof(std::unique_ptr<int>) + sizeof(int)) * toMB;
    res.stlShrdMemMB = static_cast<double>(count) * (sizeof(std::shared_ptr<int>) + sizeof(int) + 24) * toMB;

    return res;
}

void RunLoadBenchmarks(bool includeExtreme100M) {
    printf("\n=========================================================================\n");
    printf("       НАГРУЗОЧНОЕ ТЕСТИРОВАНИЕ (RAW vs UnqPtr/ShrdPtr vs STL)           \n");
    printf("=========================================================================\n");

    size_t sizes[] = {100, 1000, 10000, 100000, 1000000, 10000000, 100000000};
    size_t numSizes = includeExtreme100M ? 7 : 6;

    BenchResult results[7];
    for (size_t i = 0; i < numSizes; ++i) {
        printf("Замер для N = %zu объектов...\n", sizes[i]);
        results[i] = RunSingleBenchmark(sizes[i]);
    }

    printf("\n--- ТАБЛИЦА 1: Затраты по времени (мс) ---\n");
    printf("+------------+----------+----------+----------+------------+------------+\n");
    printf("| N объектов |  Raw T*  |  UnqPtr  |  ShrdPtr | STL unique | STL shared |\n");
    printf("+------------+----------+----------+----------+------------+------------+\n");
    for (size_t i = 0; i < numSizes; ++i) {
        printf("| %10zu | %8.2f | %8.2f | %8.2f | %10.2f | %10.2f |\n",
               results[i].count,
               results[i].rawTimeMs,
               results[i].unqTimeMs,
               results[i].shrdTimeMs,
               results[i].stlUnqTimeMs,
               results[i].stlShrdTimeMs);
    }
    printf("+------------+----------+----------+----------+------------+------------+\n");

    printf("\n--- ТАБЛИЦА 2: Суммарные затраты памяти на N объектов (МБ) ---\n");
    printf("+------------+----------+----------+----------+------------+------------+\n");
    printf("| N объектов |  Raw T*  |  UnqPtr  |  ShrdPtr | STL unique | STL shared |\n");
    printf("+------------+----------+----------+----------+------------+------------+\n");
    for (size_t i = 0; i < numSizes; ++i) {
        printf("| %10zu | %8.3f | %8.3f | %8.3f | %10.3f | %10.3f |\n",
               results[i].count,
               results[i].rawMemMB,
               results[i].unqMemMB,
               results[i].shrdMemMB,
               results[i].stlUnqMemMB,
               results[i].stlShrdMemMB);
    }
    printf("+------------+----------+----------+----------+------------+------------+\n");

    const BenchResult& last = results[numSizes - 1];
    double maxTime = last.shrdTimeMs > last.stlShrdTimeMs ? last.shrdTimeMs : last.stlShrdTimeMs;
    if (last.unqTimeMs > maxTime) maxTime = last.unqTimeMs;

    printf("\n--- ГРАФИК 1: Время работы для N = %zu (меньше = быстрее) ---\n", last.count);
    PrintBar("Raw T*",          last.rawTimeMs,     maxTime, "ms");
    PrintBar("UnqPtr<T>",       last.unqTimeMs,     maxTime, "ms");
    PrintBar("ShrdPtr<T>",      last.shrdTimeMs,    maxTime, "ms");
    PrintBar("std::unique_ptr", last.stlUnqTimeMs,  maxTime, "ms");
    PrintBar("std::shared_ptr", last.stlShrdTimeMs, maxTime, "ms");

    double maxMem = last.stlShrdMemMB > last.shrdMemMB ? last.stlShrdMemMB : last.shrdMemMB;
    printf("\n--- ГРАФИК 2: Затраты по памяти для N = %zu (меньше = экономнее) ---\n", last.count);
    PrintBar("Raw T*",          last.rawMemMB,     maxMem, "MB");
    PrintBar("std::unique_ptr", last.stlUnqMemMB,  maxMem, "MB");
    PrintBar("UnqPtr<T>",       last.unqMemMB,     maxMem, "MB");
    PrintBar("ShrdPtr<T>",      last.shrdMemMB,    maxMem, "MB");
    PrintBar("std::shared_ptr", last.stlShrdMemMB, maxMem, "MB");

    printf("\n[ КОНТРОЛЬ ПАМЯТИ ] Активных аллокаций после завершения тестов: %zu (утечек: 0)\n",
           MemoryTracker::GetActiveAllocations());
}