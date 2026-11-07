#include <stdio.h>
#include <iostream>
#include <cstddef>
#ifdef _WIN32
#include <windows.h>
#undef GetMessage
#endif

#include "UnqPtr.h"
#include "ShrdPtr.h"
#include "ArraySequence.h"
#include "Tests.h"
#include "Exceptions.h"
#include "MemoryTracker.h"

int ReadInt(const char* prompt) {
    printf("%s", prompt);
    int value = 0;
    while (scanf("%d", &value) != 1) {
        while (getchar() != '\n');
        printf("Ошибка ввода. Введите целое число: ");
    }
    return value;
}

void SequenceInteractiveMenu() {
    UnqPtr<Sequence<int>> seq(new ArraySequence<int>());
    seq->Append(10);
    seq->Append(20);
    seq->Append(30);

    int op = -1;
    while (op != 0) {
        printf("\nArraySequence (управляется через UnqPtr<Sequence<int>>, len=%zu): [ ", seq->GetLength());
        for (size_t i = 0; i < seq->GetLength(); ++i) {
            if (i > 0) printf(", ");
            printf("%d", seq->Get(i));
        }
        printf(" ]\n");
        printf("1. Append (добавить в конец)\n");
        printf("2. Prepend (добавить в начало)\n");
        printf("3. InsertAt (вставить по индексу)\n");
        printf("4. Изменить элемент по индексу (operator[])\n");
        printf("5. Пересоздать пустой ArraySequence (проверка автоматической очистки)\n");
        printf("0. Назад в главное меню\n");

        op = ReadInt("Выберите действие: ");
        try {
            switch (op) {
                case 1:
                    seq->Append(ReadInt("Введите число: "));
                    break;
                case 2:
                    seq->Prepend(ReadInt("Введите число: "));
                    break;
                case 3: {
                    int idx = ReadInt("Введите индекс: ");
                    int val = ReadInt("Введите число: ");
                    seq->InsertAt(val, static_cast<size_t>(idx));
                    break;
                }
                case 4: {
                    int idx = ReadInt("Введите индекс: ");
                    int val = ReadInt("Введите новое значение: ");
                    (*seq)[static_cast<size_t>(idx)] = val;
                    break;
                }
                case 5:
                    seq.Reset(new ArraySequence<int>());
                    printf("Старый контейнер автоматически удалён, создан новый!\n");
                    break;
                case 0:
                    break;
                default:
                    printf("Неверный пункт меню.\n");
            }
        } catch (const Exception& e) {
            printf("\n[ ИСКЛЮЧЕНИЕ ]: %s\n", e.GetMessage());
        }
    }
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    int choice = -1;
    while (choice != 0) {
        printf(" Контроль памяти: активных аллокаций = %zu | всего создано = %zu\n",
               MemoryTracker::GetActiveAllocations(),
               MemoryTracker::GetTotalAllocations());
        printf("----------------------------------------------------------------------\n");
        printf("1. Запустить функциональные тесты (UnqPtr, ShrdPtr, Subtyping, ArraySequence)\n");
        printf("2. Запустить нагрузочные тесты и графики (N от 10^2 до 10^7)\n");
        printf("3. Запустить экстремальный нагрузочный тест (включая N = 10^8)\n");
        printf("4. Интерактивная работа с контейнером ArraySequence<int>\n");
        printf("0. Выход\n");

        choice = ReadInt("Ваш выбор: ");
        try {
            switch (choice) {
                case 1:
                    RunAllFunctionalTests();
                    break;
                case 2:
                    RunLoadBenchmarks(false);
                    break;
                case 3:
                    RunLoadBenchmarks(true);
                    break;
                case 4:
                    SequenceInteractiveMenu();
                    break;
                case 0:
                    printf("Завершение работы...\n");
                    break;
                default:
                    printf("Неверный выбор.\n");
            }
        } catch (const Exception& e) {
            printf("\n[ КРИТИЧЕСКАЯ ОШИБКА ]: %s\n", e.GetMessage());
        }
    }

    return 0;
}