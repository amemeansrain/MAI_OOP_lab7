#include <iostream>
#include "include/dungeon.h"
#include "include/factory.h"
#include "include/observer.h"

int main() {
    try {
        Dungeon dungeon;
        
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
        
        // Ждем завершения игры
        std::this_thread::sleep_for(std::chrono::seconds(31));
        
        // Сохраняем результаты
        dungeon.saveToFile("dungeon_final.txt");
        std::cout << "\nРезультаты сохранены в файлы 'dungeon_final.txt' и 'log.txt'\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}