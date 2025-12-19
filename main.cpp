#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>
#include "include/dungeon.h"
#include "include/factory.h"
#include "include/observer.h"

// Глобальная переменная для обработки сигналов
Dungeon* globalDungeon = nullptr;

// Обработчик сигналов
void signalHandler(int signal) {
    if (globalDungeon && signal == SIGINT) {
        std::cout << "\nПолучен сигнал прерывания. Завершаем игру..." << std::endl;
        globalDungeon->stopGame();
    }
}

int main() {
    try {
        Dungeon dungeon;
        globalDungeon = &dungeon;
        
        // Устанавливаем обработчик сигналов
        std::signal(SIGINT, signalHandler);
        
        // Добавляем Observer'ы
        auto consoleObserver = std::make_shared<ConsoleObserver>();
        auto fileObserver = std::make_shared<FileObserver>();
        dungeon.addObserver(consoleObserver);
        dungeon.addObserver(fileObserver);
        
        std::cout << "=== RPG DUNGEON SIMULATOR ===\n";
        std::cout << "Вариант 20: Дракон, Бык, Жаба\n";
        std::cout << "Правила:\n";
        std::cout << "- Дракон может атаковать быков\n";
        std::cout << "- Бык может атаковать жаб\n";
        std::cout << "- Жабы никого не атакуют\n";
        std::cout << "- Каждый ход NPC бросают кубик для атаки/защиты\n";
        std::cout << "==============================\n\n";
        
        // Запускаем игру
        dungeon.startGame();
        
        // Ждем 30 секунд
        for (int i = 0; i < 30; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (!dungeon.getAliveCount()) {
                std::cout << "\nВсе NPC погибли! Завершаем игру досрочно." << std::endl;
                break;
            }
        }
        
        // Останавливаем игру
        dungeon.stopGame();
        
        // Ждем немного чтобы все потоки завершились
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Сохраняем результаты
        dungeon.saveToFile("dungeon_final.txt");
        std::cout << "\nРезультаты сохранены в файлы 'dungeon_final.txt' и 'log.txt'\n";
        
        // Финальный вывод статистики
        dungeon.printNPCs();
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}