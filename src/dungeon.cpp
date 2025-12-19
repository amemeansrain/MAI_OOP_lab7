#include "dungeon.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <iomanip>

Dungeon::Dungeon() : running(false), fightCount(0) {}

Dungeon::~Dungeon() {
    stopGame();
}

void Dungeon::addNPC(std::shared_ptr<NPC> npc) {
    std::unique_lock lock(npcsMutex);
    npcs.push_back(npc);
}

void Dungeon::addObserver(std::shared_ptr<Observer> observer) {
    observers.push_back(observer);
}

void Dungeon::printNPCs() const {
    std::shared_lock lock(npcsMutex);
    std::cout << "Список NPC в подземелье:\n";
    for (const auto& npc : npcs) {
        if (npc->isAlive()) {
            std::cout << "Тип: " << npc->getType() 
                      << ", Имя: " << npc->getName() 
                      << ", Координаты: (" << npc->getX() << ", " << npc->getY() << ")"
                      << ", Жив: " << (npc->isAlive() ? "Да" : "Нет") << "\n";
        }
    }
}

void Dungeon::saveToFile(const std::string& filename) const {
    std::shared_lock lock(npcsMutex);
    std::ofstream file(filename);
    for (const auto& npc : npcs) {
        npc->save(file);
        file << "\n";
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

void Dungeon::movementWorker() {
    std::mt19937 gen(rd());
    
    while (running) {
        {
            std::shared_lock lock(npcsMutex);
            
            for (size_t i = 0; i < npcs.size() && running; ++i) {
                if (!npcs[i]->isAlive()) continue;
                
                // Двигаем NPC
                auto oldX = npcs[i]->getX();
                auto oldY = npcs[i]->getY();
                npcs[i]->move(gen);
                
                // Уведомляем наблюдателей о движении
                for (auto& observer : observers) {
                    observer->onMove(npcs[i]->getName(), npcs[i]->getX(), npcs[i]->getY());
                }
                
                // Проверяем возможность боя с другими NPC
                for (size_t j = 0; j < npcs.size(); ++j) {
                    if (i == j || !npcs[j]->isAlive()) continue;
                    
                    double distance = npcs[i]->distanceTo(*npcs[j]);
                    if (distance <= npcs[i]->getKillDistance()) {
                        // Создаем задачу для потока боев
                        std::lock_guard<std::mutex> fightLock(fightQueueMutex);
                        fightQueue.push({npcs[i], npcs[j]});
                        fightCV.notify_one();
                    }
                }
            }
        }
        
        // Пауза между движениями
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Dungeon::processFight(FightTask& task) {
    auto& attacker = task.attacker;
    auto& defender = task.defender;
    
    if (!attacker->isAlive() || !defender->isAlive()) return;
    
    // Проверяем, может ли атакующий атаковать защитника
    if (attacker->canAttack(defender.get())) {
        std::mt19937 gen(rd());
        
        int attackPower = attacker->rollAttack(gen);
        int defensePower = defender->rollDefense(gen);
        
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

void Dungeon::fightWorker() {
    while (running) {
        std::unique_lock<std::mutex> lock(fightQueueMutex);
        
        // Ждем задач или остановки
        fightCV.wait(lock, [this]() { 
            return !fightQueue.empty() || !running; 
        });
        
        if (!running) break;
        
        // Берем задачу из очереди
        if (!fightQueue.empty()) {
            FightTask task = fightQueue.front();
            fightQueue.pop();
            lock.unlock();
            
            processFight(task);
        }
    }
}

void Dungeon::printMap() {
    const int MAP_SIZE = 100;
    const int CELL_SIZE = 10; // Каждая клетка = 10x10 единиц
    
    std::vector<std::vector<std::string>> map(MAP_SIZE/CELL_SIZE, 
                                             std::vector<std::string>(MAP_SIZE/CELL_SIZE, "."));
    
    {
        std::shared_lock lock(npcsMutex);
        
        for (const auto& npc : npcs) {
            if (npc->isAlive()) {
                int x = static_cast<int>(npc->getX()) / CELL_SIZE;
                int y = static_cast<int>(npc->getY()) / CELL_SIZE;
                
                if (x >= 0 && x < MAP_SIZE/CELL_SIZE && y >= 0 && y < MAP_SIZE/CELL_SIZE) {
                    map[y][x] = npc->getTypeSymbol();
                }
            }
        }
    }
    
    std::lock_guard<std::mutex> coutLock(std::mutex()); // Блокируем cout
    std::cout << "\n=== КАРТА ПОДЗЕМЕЛЬЯ ===\n";
    std::cout << "Легенда: D - Дракон, B - Бык, T - Жаба, . - пусто\n";
    
    for (int y = 0; y < MAP_SIZE/CELL_SIZE; ++y) {
        for (int x = 0; x < MAP_SIZE/CELL_SIZE; ++x) {
            std::cout << std::setw(2) << map[y][x];
        }
        std::cout << std::endl;
    }
    
    std::cout << "Живых NPC: " << getAliveCount() 
              << ", Всего NPC: " << getNPCCount()
              << ", Боев проведено: " << fightCount.load() << std::endl;
}

void Dungeon::mainWorker() {
    auto startTime = std::chrono::steady_clock::now();
    
    while (running) {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);
        
        if (elapsed.count() >= 30) {
            stopGame();
            break;
        }
        
        // Печатаем карту раз в секунду
        printMap();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    // Финальный вывод
    std::cout << "\n=== ИГРА ОКОНЧЕНА (прошло 30 секунд) ===\n";
    std::cout << "ВЫЖИВШИЕ:\n";
    printNPCs();
}

void Dungeon::startGame() {
    if (running) return;
    
    running = true;
    
    // Создаем 50 NPC в случайных местах
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> posDist(0, 100);
    
    for (int i = 0; i < 50; ++i) {
        double x = posDist(gen);
        double y = posDist(gen);
        auto npc = NPCFactory::createRandomNPC(x, y);
        addNPC(npc);
    }
    
    std::cout << "Создано 50 NPC. Начинаем игру!\n";
    std::cout << "Игра продлится 30 секунд...\n";
    
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
    
    // Ждем завершения потоков
    if (movementThread.joinable()) movementThread.join();
    if (fightThread.joinable()) fightThread.join();
    if (mainThread.joinable()) mainThread.join();
}