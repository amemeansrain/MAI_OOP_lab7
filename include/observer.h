#ifndef OBSERVER_H
#define OBSERVER_H

#include <string>
#include <fstream>
#include <memory>
#include <mutex>
#include <iostream>

// Observer интерфейс
class Observer {
public:
    virtual ~Observer() = default;
    virtual void onFight(const std::string& attacker, const std::string& defender, bool defenderDied) = 0;
    virtual void onMove(const std::string& npcName, double x, double y) = 0;
    virtual void onDie(const std::string& npcName) = 0;
};

// Конкретные Observer'ы
class ConsoleObserver : public Observer {
    std::mutex coutMutex;
public:
    void onFight(const std::string& attacker, const std::string& defender, bool defenderDied) override;
    void onMove(const std::string& npcName, double x, double y) override;
    void onDie(const std::string& npcName) override;
};

class FileObserver : public Observer {
    std::ofstream file;
    std::mutex fileMutex;
public:
    FileObserver();
    void onFight(const std::string& attacker, const std::string& defender, bool defenderDied) override;
    void onMove(const std::string& npcName, double x, double y) override;
    void onDie(const std::string& npcName) override;
};

#endif