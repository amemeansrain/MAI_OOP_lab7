#ifndef DUNGEON_H
#define DUNGEON_H

#include "npc.h"
#include "factory.h"
#include "observer.h"
#include <vector>
#include <memory>
#include <fstream>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <functional>
#include <thread>

// Структура для задания боя
struct FightTask {
    std::shared_ptr<NPC> attacker;
    std::shared_ptr<NPC> defender;
};

// Класс для управления подземельем
class Dungeon {
private:
    std::vector<std::shared_ptr<NPC>> npcs;
    std::vector<std::shared_ptr<Observer>> observers;
    
    // Потокобезопасные структуры
    mutable std::shared_mutex npcsMutex;
    std::mutex fightQueueMutex;
    std::condition_variable fightCV;
    std::queue<FightTask> fightQueue;
    
    // Флаги управления потоками
    std::atomic<bool> running;
    std::atomic<int> fightCount;
    
    // Потоки
    std::thread movementThread;
    std::thread fightThread;
    std::thread mainThread;
    
    // Генератор случайных чисел для каждого потока
    std::random_device rd;
    
    // Вспомогательные методы
    void movementWorker();
    void fightWorker();
    void mainWorker();
    void processFight(FightTask& task);
    void printMap();

public:
    Dungeon();
    ~Dungeon();
    
    void addNPC(std::shared_ptr<NPC> npc);
    void addObserver(std::shared_ptr<Observer> observer);
    void printNPCs() const;
    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);
    void startGame();
    void stopGame();
    
    size_t getNPCCount() const;
    size_t getAliveCount() const;
    const std::vector<std::shared_ptr<NPC>>& getNPCs() const;
};

#endif