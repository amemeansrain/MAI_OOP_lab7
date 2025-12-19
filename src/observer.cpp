#include "../include/observer.h"

// Реализация ConsoleObserver
void ConsoleObserver::onFight(const std::string& attacker, const std::string& defender, bool defenderDied) {
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << "[БОЙ] " << attacker << " атакует " << defender;
    if (defenderDied) {
        std::cout << " и убивает!";
    } else {
        std::cout << ", но " << defender << " выживает!";
    }
    std::cout << std::endl;
}

void ConsoleObserver::onMove(const std::string& npcName, double x, double y) {
    // Не выводим перемещения чтобы не засорять консоль
    // std::lock_guard<std::mutex> lock(coutMutex);
    // std::cout << "[ДВИЖЕНИЕ] " << npcName << " переместился в (" << x << ", " << y << ")" << std::endl;
}

void ConsoleObserver::onDie(const std::string& npcName) {
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << "[СМЕРТЬ] " << npcName << " погиб!" << std::endl;
}

// Реализация FileObserver
FileObserver::FileObserver() : file("log.txt", std::ios::app) {}

void FileObserver::onFight(const std::string& attacker, const std::string& defender, bool defenderDied) {
    std::lock_guard<std::mutex> lock(fileMutex);
    file << "[БОЙ] " << attacker << " атакует " << defender;
    if (defenderDied) {
        file << " и убивает!";
    } else {
        file << ", но " << defender << " выживает!";
    }
    file << std::endl;
}

void FileObserver::onMove(const std::string& npcName, double x, double y) {
    std::lock_guard<std::mutex> lock(fileMutex);
    file << "[ДВИЖЕНИЕ] " << npcName << " переместился в (" << x << ", " << y << ")" << std::endl;
}

void FileObserver::onDie(const std::string& npcName) {
    std::lock_guard<std::mutex> lock(fileMutex);
    file << "[СМЕРТЬ] " << npcName << " погиб!" << std::endl;
}