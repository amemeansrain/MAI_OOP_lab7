#include "../include/dungeon.h"
#include "../include/factory.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <algorithm>

Dungeon::Dungeon() : running(false), fightCount(0) {
    npcs.reserve(100);
}

Dungeon::~Dungeon() {
    stopGame();
}

void Dungeon::addNPC(std::shared_ptr<NPC> npc) {
    std::unique_lock lock(npcsMutex);
    if (npc->getX() >= 0 && npc->getX() <= 100 && npc->getY() >= 0 && npc->getY() <= 100) {
        npcs.push_back(npc);
    }
}

void Dungeon::addObserver(std::shared_ptr<Observer> observer) {
    observers.push_back(observer);
}

void Dungeon::printNPCs() const {
    std::shared_lock lock(npcsMutex);
    std::cout << "\n=== СПИСОК ВЫЖИВШИХ NPC ===\n";
    int aliveCount = 0;
    for (const auto& npc : npcs) {
        if (npc->isAlive()) {
            std::cout << "Тип: " << std::setw(8) << std::left << npc->getType() 
                      << " Имя: " << std::setw(15) << std::left << npc->getName() 
                      << " Координаты: (" << std::setw(3) << (int)npc->getX() 
                      << ", " << std::setw(3) << (int)npc->getY() << ")\n";
            aliveCount++;
        }
    }
    std::cout << "Всего выживших: " << aliveCount << "\n";
}

void Dungeon::saveToFile(const std::string& filename) const {
    std::shared_lock lock(npcsMutex);
    std::ofstream file(filename);
    for (const auto& npc : npcs) {
        if (npc->isAlive()) {
            npc->save(file);
            file << "\n";
        }
    }
}

void Dungeon::loadFromFile(const std::string& filename) {
    std::unique_lock lock(npcsMutex);
    npcs.clear();
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        auto npc = NPCFactory::loadFromStream(iss);
        if (npc) {
            npcs.push_back(npc);
        }
    }
}

size_t Dungeon::getNPCCount() const {
    std::shared_lock lock(npcsMutex);
    return npcs.size();
}

size_t Dungeon::getAliveCount() const {
    std::shared_lock lock(npcsMutex);
    size_t count = 0;
    for (const auto& npc : npcs) {
        if (npc->isAlive()) count++;
    }
    return count;
}

const std::vector<std::shared_ptr<NPC>>& Dungeon::getNPCs() const {
    return npcs;
}

void Dungeon::processFight(FightTask& task) {
    auto attacker = task.attacker;
    auto defender = task.defender;
    
    if (!attacker || !defender || !attacker->isAlive() || !defender->isAlive()) {
        return;
    }
    
    // Проверяем, может ли атакующий атаковать защитника
    if (attacker->canAttack(defender.get())) {
        std::mt19937 gen(rd());
        
        int attackPower = attacker->rollAttack(gen);
        int defensePower = defender->rollDefense(gen);
        
        // Отладочный вывод для проверки правил
        std::cout << "[АТАКА] " << attacker->getName() << " (" << attacker->getType() 
                  << ") атакует " << defender->getName() << " (" << defender->getType() 
                  << "): атака=" << attackPower << ", защита=" << defensePower << std::endl;
        
        if (attackPower > defensePower) {
            // Убийство
            defender->die();
            
            // Уведомляем наблюдателей
            for (auto& observer : observers) {
                observer->onFight(attacker->getName(), defender->getName(), true);
                observer->onDie(defender->getName());
            }
        } else {
            // Защита успешна
            for (auto& observer : observers) {
                observer->onFight(attacker->getName(), defender->getName(), false);
            }
        }
        
        fightCount++;
    }
}

void Dungeon::movementWorker() {
    std::mt19937 gen(rd());
    
    while (running) {
        // Делаем паузу между движениями
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        {
            std::shared_lock lock(npcsMutex);
            if (npcs.empty() || !running) continue;
            
            // Создаем копию индексов живых NPC
            std::vector<size_t> aliveIndices;
            for (size_t i = 0; i < npcs.size(); ++i) {
                if (npcs[i]->isAlive()) {
                    aliveIndices.push_back(i);
                }
            }
            
            // Перемешиваем индексы для случайного порядка движения
            std::shuffle(aliveIndices.begin(), aliveIndices.end(), gen);
            
            for (size_t idx : aliveIndices) {
                if (!running) break;
                
                // Двигаем NPC
                npcs[idx]->move(gen);
                
                // Проверяем возможность боя с другими NPC
                // Проверяем только часть NPC чтобы не перегружать
                for (size_t j = 0; j < std::min(npcs.size(), (size_t)10); ++j) {
                    if (idx == j || !npcs[j]->isAlive()) continue;
                    
                    double distance = npcs[idx]->distanceTo(*npcs[j]);
                    if (distance <= npcs[idx]->getKillDistance()) {
                        // Создаем задачу для потока боев
                        std::lock_guard<std::mutex> fightLock(fightQueueMutex);
                        fightQueue.push({npcs[idx], npcs[j]});
                        fightCV.notify_one();
                    }
                }
            }
        }
    }
}

void Dungeon::fightWorker() {
    while (running) {
        FightTask task;
        bool hasTask = false;
        
        {
            std::unique_lock<std::mutex> lock(fightQueueMutex);
            
            // Ждем задачи или остановки с таймаутом
            if (fightCV.wait_for(lock, std::chrono::milliseconds(100), 
                [this]() { return !fightQueue.empty() || !running; })) {
                
                if (!running) break;
                
                if (!fightQueue.empty()) {
                    task = fightQueue.front();
                    fightQueue.pop();
                    hasTask = true;
                }
            }
        }
        
        if (hasTask) {
            processFight(task);
        }
    }
}

void Dungeon::printMap() {
    const int MAP_SIZE = 100;
    const int CELL_SIZE = 10;
    
    std::vector<std::vector<std::string>> map(MAP_SIZE/CELL_SIZE, 
                                             std::vector<std::string>(MAP_SIZE/CELL_SIZE, "."));
    
    int dragonCount = 0, bullCount = 0, toadCount = 0;
    
    {
        std::shared_lock lock(npcsMutex);
        
        for (const auto& npc : npcs) {
            if (npc->isAlive()) {
                int x = static_cast<int>(npc->getX()) / CELL_SIZE;
                int y = static_cast<int>(npc->getY()) / CELL_SIZE;
                
                if (x >= 0 && x < MAP_SIZE/CELL_SIZE && y >= 0 && y < MAP_SIZE/CELL_SIZE) {
                    map[y][x] = npc->getTypeSymbol();
                    
                    if (npc->getType() == "Dragon") dragonCount++;
                    else if (npc->getType() == "Bull") bullCount++;
                    else if (npc->getType() == "Toad") toadCount++;
                }
            }
        }
    }
    
    // Блокируем cout для чистого вывода
    static std::mutex coutMutex;
    std::lock_guard<std::mutex> coutLock(coutMutex);
    
    std::cout << "\n=== КАРТА ПОДЗЕМЕЛЬЯ ===" << std::endl;
    std::cout << "Легенда: D - Дракон, B - Бык, T - Жаба, . - пусто" << std::endl;
    std::cout << "Драконы: " << dragonCount << ", Быки: " << bullCount << ", Жабы: " << toadCount << std::endl;
    
    for (int y = 0; y < MAP_SIZE/CELL_SIZE; ++y) {
        for (int x = 0; x < MAP_SIZE/CELL_SIZE; ++x) {
            std::cout << std::setw(2) << map[y][x];
        }
        std::cout << std::endl;
    }
    
    std::cout << "Всего живых: " << getAliveCount() 
              << " из " << getNPCCount()
              << ", Боев проведено: " << fightCount.load() << std::endl;
}

void Dungeon::mainWorker() {
    auto startTime = std::chrono::steady_clock::now();
    int lastMapTime = -2; // Чтобы первая карта отобразилась сразу
    
    while (running) {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
        
        if (elapsed.count() >= 30) {
            stopGame();
            break;
        }
        
        // Печатаем карту раз в 3 секунды
        if (elapsed.count() / 3 != lastMapTime) {
            lastMapTime = elapsed.count() / 3;
            printMap();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Финальный вывод
    if (running) {
        std::cout << "\n=== ИГРА ОКОНЧЕНА (прошло 30 секунд) ===" << std::endl;
        printNPCs();
    }
}

void Dungeon::startGame() {
    if (running) return;
    
    running = true;
    
    // Создаем 50 NPC в случайных местах
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> posDist(0, 100);
    
    std::cout << "Создаю NPC..." << std::endl;
    for (int i = 0; i < 50; ++i) {
        double x = posDist(gen);
        double y = posDist(gen);
        auto npc = NPCFactory::createRandomNPC(x, y);
        if (npc) {
            addNPC(npc);
        }
    }
    
    std::cout << "Создано " << getNPCCount() << " NPC. Начинаем игру!" << std::endl;
    std::cout << "Игра продлится 30 секунд..." << std::endl;
    
    // Запускаем потоки
    movementThread = std::thread(&Dungeon::movementWorker, this);
    fightThread = std::thread(&Dungeon::fightWorker, this);
    mainThread = std::thread(&Dungeon::mainWorker, this);
}

void Dungeon::stopGame() {
    if (!running) return;
    
    running = false;
    
    // Оповещаем все потоки
    fightCV.notify_all();
    
    // Ждем завершения потоков с таймаутом
    if (movementThread.joinable()) {
        if (movementThread.get_id() != std::this_thread::get_id()) {
            movementThread.join();
        }
    }
    
    if (fightThread.joinable()) {
        if (fightThread.get_id() != std::this_thread::get_id()) {
            fightThread.join();
        }
    }
    
    if (mainThread.joinable()) {
        if (mainThread.get_id() != std::this_thread::get_id()) {
            mainThread.join();
        }
    }
}