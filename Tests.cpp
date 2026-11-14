#include "Tests.h"
#include <stdio.h>
#include <time.h>
#include <memory>
#include <type_traits>
#include <utility>
#include "UnqPtr.h"
#include "ShrdPtr.h"
#include "ArraySequence.h"
#include "Exceptions.h"

struct Vehicle {
    inline static int aliveCount = 0;
    int speed;
    explicit Vehicle(int s = 60) : speed(s) { aliveCount++; }
    Vehicle(const Vehicle& other) : speed(other.speed) { aliveCount++; }
    virtual ~Vehicle() { aliveCount--; }
    virtual int GetSpeed() const { return speed; }
};

struct Car : public Vehicle {
    int boost;
    explicit Car(int s = 100, int b = 50) : Vehicle(s), boost(b) {}
    Car(const Car& other) : Vehicle(other.speed), boost(other.boost) {}
    ~Car() override = default;
    int GetSpeed() const override { return speed + boost; }
};

struct Unrelated { int x = 0; };

// Compile-time проверки:
// 1. UnqPtr нельзя копировать (ни конструктором, ни присваиванием)
static_assert(!std::is_copy_constructible_v<UnqPtr<Vehicle>>);
static_assert(!std::is_copy_assignable_v<UnqPtr<Vehicle>>);
static_assert(!std::is_constructible_v<UnqPtr<Vehicle>, const UnqPtr<Car>&>);

// 2. Car приводится к Vehicle (при перемещении для UnqPtr и при копировании/перемещении для ShrdPtr)
static_assert(std::is_convertible_v<UnqPtr<Car>, UnqPtr<Vehicle>>);
static_assert(std::is_convertible_v<ShrdPtr<Car>, ShrdPtr<Vehicle>>);
static_assert(!std::is_convertible_v<UnqPtr<Unrelated>, UnqPtr<Vehicle>>);
static_assert(!std::is_convertible_v<ShrdPtr<Unrelated>, ShrdPtr<Vehicle>>);

static void Assert(bool cond, const char* msg) {
    if (!cond) {
        printf("[ FAIL ] %s\n", msg);
        throw Exception(msg);
    }
}

void RunAllTests() {
    printf("\n==================== ЗАПУСК ТЕСТОВ ====================\n");

    // 1. Тест UnqPtr (только эксклюзивное владение и перемещение)
    {
        UnqPtr<Vehicle> u1(new Vehicle(80));
        Assert(u1->GetSpeed() == 80 && Vehicle::aliveCount == 1, "UnqPtr: создание объекта");

        UnqPtr<Vehicle> uMoved = std::move(u1); // Перемещение конструктором
        Assert(u1.IsNull() && uMoved->GetSpeed() == 80 && Vehicle::aliveCount == 1, "UnqPtr: move-конструктор");

        UnqPtr<Vehicle> uMoved2;
        uMoved2 = std::move(uMoved); // Перемещение присваиванием
        Assert(uMoved.IsNull() && uMoved2->GetSpeed() == 80 && Vehicle::aliveCount == 1, "UnqPtr: move-присваивание");
    }
    Assert(Vehicle::aliveCount == 0, "UnqPtr: утечка памяти после выхода из области видимости");
    printf("[  OK  ] UnqPtr<T> (запрет копирования, move-семантика, очистка памяти)\n");

    // 2. Тест ShrdPtr
    {
        ShrdPtr<Vehicle> s1(new Vehicle(90));
        {
            ShrdPtr<Vehicle> s2 = s1;
            Assert(s1.UseCount() == 2 && s2.UseCount() == 2 && Vehicle::aliveCount == 1, "ShrdPtr: счетчик ссылок = 2");
        }
        Assert(s1.UseCount() == 1 && Vehicle::aliveCount == 1, "ShrdPtr: счетчик ссылок = 1 после удаления копии");
    }
    Assert(Vehicle::aliveCount == 0, "ShrdPtr: утечка памяти");
    printf("[  OK  ] ShrdPtr<T> (счетчик ссылок size_t* referenceCount, очистка памяти)\n");

    // 3. Тест подтипизации (Car <= Vehicle)
    {
        ShrdPtr<Car> carPtr(new Car(100, 40));
        ShrdPtr<Vehicle> basePtr = carPtr; // Копирование ShrdPtr с подтипизацией
        Assert(basePtr.UseCount() == 2 && basePtr->GetSpeed() == 140, "ShrdPtr: подтипизация Car -> Vehicle");

        UnqPtr<Car> unqCar(new Car(120, 30));
        UnqPtr<Vehicle> unqBase = std::move(unqCar); // Перемещение UnqPtr с подтипизацией
        Assert(unqCar.IsNull() && unqBase->GetSpeed() == 150, "UnqPtr: move-подтипизация Car -> Vehicle");
    }
    Assert(Vehicle::aliveCount == 0, "Subtyping: утечка памяти");
    printf("[  OK  ] Подтипизация (Car <= Vehicle + static_assert)\n");

    // 4. Тест ArraySequence (управляется UnqPtr<T[]>, хранит ShrdPtr<Vehicle>)
    {
        ArraySequence<ShrdPtr<Vehicle>> seq;
        seq.Append(ShrdPtr<Vehicle>(new Vehicle(50)));
        seq.Append(ShrdPtr<Car>(new Car(100, 60)));
        seq.Append(ShrdPtr<Car>(new Car(200, 80)));

        Assert(seq.GetLength() == 3, "ArraySequence: неверная длина");
        Assert(seq[0]->GetSpeed() == 50 && seq[1]->GetSpeed() == 160 && seq[2]->GetSpeed() == 280,
               "ArraySequence: полиморфные вызовы GetSpeed()");
        Assert(Vehicle::aliveCount == 3, "ArraySequence: должно жить 3 объекта");
    }
    Assert(Vehicle::aliveCount == 0, "ArraySequence: утечка памяти");
    printf("[  OK  ] ArraySequence<T> на базе UnqPtr<T[]> и ShrdPtr<T> (утечек: 0)\n");
}

// ==================== БЕНЧМАРКИ ====================
struct Row {
    size_t n;
    double rawMs, unqMs, shrdMs, stlUnqMs, stlShrdMs;
    double rawMB, unqMB, shrdMB, stlUnqMB, stlShrdMB;
};

static void DrawBar(const char* name, double val, double maxVal, const char* unit) {
    int width = (maxVal > 0.0001) ? static_cast<int>((val / maxVal) * 30.0) : 1;
    if (width < 1 && val > 0) width = 1;
    printf("  %-15s | ", name);
    for (int i = 0; i < width; ++i) printf("#");
    for (int i = width; i < 30; ++i) printf(" ");
    printf(" | %8.2f %s\n", val, unit);
}

void RunBenchmarks(bool include100M) {
    printf("\n==================== БЕНЧМАРКИ ====================\n");
    size_t counts[] = {100, 10000, 1000000, 10000000, 100000000};
    size_t total = include100M ? 5 : 4;
    Row rows[5];

    for (size_t idx = 0; idx < total; ++idx) {
        size_t n = counts[idx];
        printf("  N = %zu...\n", n);
        Row r = {};
        r.n = n;
        volatile long long sink = 0;

        clock_t t0 = clock();
        for (size_t i = 0; i < n; ++i) { int* p = new int(i); sink += *p; delete p; }
        r.rawMs = (double)(clock() - t0) / CLOCKS_PER_SEC * 1000.0;

        t0 = clock();
        for (size_t i = 0; i < n; ++i) { UnqPtr<int> p(new int(i)); sink += *p; }
        r.unqMs = (double)(clock() - t0) / CLOCKS_PER_SEC * 1000.0;

        t0 = clock();
        for (size_t i = 0; i < n; ++i) { ShrdPtr<int> p(new int(i)); sink += *p; }
        r.shrdMs = (double)(clock() - t0) / CLOCKS_PER_SEC * 1000.0;

        t0 = clock();
        for (size_t i = 0; i < n; ++i) { std::unique_ptr<int> p(new int(i)); sink += *p; }
        r.stlUnqMs = (double)(clock() - t0) / CLOCKS_PER_SEC * 1000.0;

        t0 = clock();
        for (size_t i = 0; i < n; ++i) { std::shared_ptr<int> p(new int(i)); sink += *p; }
        r.stlShrdMs = (double)(clock() - t0) / CLOCKS_PER_SEC * 1000.0;

        (void)sink;
        double toMB = 1.0 / (1024.0 * 1024.0);
        r.rawMB     = n * (sizeof(int*) + sizeof(int)) * toMB;
        r.unqMB     = n * (sizeof(UnqPtr<int>) + sizeof(int)) * toMB;
        r.shrdMB    = n * (sizeof(ShrdPtr<int>) + sizeof(int) + sizeof(size_t)) * toMB;
        r.stlUnqMB  = n * (sizeof(std::unique_ptr<int>) + sizeof(int)) * toMB;
        r.stlShrdMB = n * (sizeof(std::shared_ptr<int>) + sizeof(int) + 24) * toMB;
        rows[idx] = r;
    }

    printf("\nТаблица 1: Время выполнения (мс)\n");
    printf("+------------+----------+----------+----------+------------+------------+\n");
    printf("| N          |  Raw T*  |  UnqPtr  |  ShrdPtr | STL unique | STL shared |\n");
    printf("+------------+----------+----------+----------+------------+------------+\n");
    for (size_t i = 0; i < total; ++i) {
        printf("| %10zu | %8.2f | %8.2f | %8.2f | %10.2f | %10.2f |\n",
               rows[i].n, rows[i].rawMs, rows[i].unqMs, rows[i].shrdMs, rows[i].stlUnqMs, rows[i].stlShrdMs);
    }
    printf("+------------+----------+----------+----------+------------+------------+\n");

    printf("\nТаблица 2: Потребление памяти (МБ)\n");
    printf("+------------+----------+----------+----------+------------+------------+\n");
    printf("| N          |  Raw T*  |  UnqPtr  |  ShrdPtr | STL unique | STL shared |\n");
    printf("+------------+----------+----------+----------+------------+------------+\n");
    for (size_t i = 0; i < total; ++i) {
        printf("| %10zu | %8.3f | %8.3f | %8.3f | %10.3f | %10.3f |\n",
               rows[i].n, rows[i].rawMB, rows[i].unqMB, rows[i].shrdMB, rows[i].stlUnqMB, rows[i].stlShrdMB);
    }
    printf("+------------+----------+----------+----------+------------+------------+\n");

    Row last = rows[total - 1];
    double maxT = last.shrdMs > last.stlShrdMs ? last.shrdMs : last.stlShrdMs;
    printf("\nВремя (N = %zu):\n", last.n);
    DrawBar("Raw T*",          last.rawMs,     maxT, "ms");
    DrawBar("UnqPtr<T>",       last.unqMs,     maxT, "ms");
    DrawBar("ShrdPtr<T>",      last.shrdMs,    maxT, "ms");
    DrawBar("std::unique_ptr", last.stlUnqMs,  maxT, "ms");
    DrawBar("std::shared_ptr", last.stlShrdMs, maxT, "ms");

    double maxM = last.stlShrdMB > last.shrdMB ? last.stlShrdMB : last.shrdMB;
    printf("\nПамять (N = %zu):\n", last.n);
    DrawBar("Raw T*",          last.rawMB,     maxM, "MB");
    DrawBar("UnqPtr<T>",       last.unqMB,     maxM, "MB");
    DrawBar("std::unique_ptr", last.stlUnqMB,  maxM, "MB");
    DrawBar("ShrdPtr<T>",      last.shrdMB,    maxM, "MB");
    DrawBar("std::shared_ptr", last.stlShrdMB, maxM, "MB");
}