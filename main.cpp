#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#undef GetMessage
#endif

#include "UnqPtr.h"
#include "ShrdPtr.h"
#include "ArraySequence.h"
#include "Tests.h"
#include "Exceptions.h"

int ReadInt(const char* prompt) {
    printf("%s", prompt);
    int val = 0;
    while (scanf("%d", &val) != 1) {
        while (getchar() != '\n');
        printf("Повторите ввод: ");
    }
    return val;
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    ArraySequence<ShrdPtr<int>> demoSeq;
    int choice = -1;

    while (choice != 0) {
        printf("1. Запустить функциональные тесты (UnqPtr, ShrdPtr, Subtyping, ArraySequence)\n");
        printf("2. Запустить нагрузочные тесты и графики (до 10^7 объектов)\n");
        printf("3. Запустить нагрузочные тесты включая 10^8 объектов\n");
        printf("4. Добавить число в ArraySequence<ShrdPtr<int>> и вывести массив\n");
        printf("0. Выход\n");

        choice = ReadInt("Выберите пункт: ");
        try {
            switch (choice) {
                case 1:
                    RunAllTests();
                    break;
                case 2:
                    RunBenchmarks(false);
                    break;
                case 3:
                    RunBenchmarks(true);
                    break;
                case 4: {
                    int v = ReadInt("Введите значение: ");
                    demoSeq.Append(ShrdPtr<int>(new int(v)));
                    printf("Содержимое ArraySequence (len=%zu): [ ", demoSeq.GetLength());
                    for (size_t i = 0; i < demoSeq.GetLength(); ++i) {
                        if (i > 0) printf(", ");
                        printf("%d (refs=%zu)", *demoSeq[i], demoSeq[i].UseCount());
                    }
                    printf(" ]\n");
                    break;
                }
                case 0:
                    break;
                default:
                    printf("Неверный пункт.\n");
            }
        } catch (const Exception& e) {
            printf("[ ОШИБКА ]: %s\n", e.GetMessage());
        }
    }
    return 0;
}