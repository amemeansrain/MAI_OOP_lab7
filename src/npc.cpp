#include "../include/npc.h"
#include "../include/visitor.h"
#include <cmath>
#include <random>
#include <iostream>

// Реализация базового класса NPC
NPC::NPC(double x, double y, const std::string& name, int moveDist, int killDist) 
    : x(x), y(y), name(name), alive(true), moveDistance(moveDist), killDistance(killDist) {}

double NPC::distanceTo(const NPC& other) const {
    return std::sqrt(std::pow(x - other.x, 2) + std::pow(y - other.y, 2));
}

void NPC::move(std::mt19937& gen) {
    if (!alive) return;
    
    std::uniform_int_distribution<> moveDir(-moveDistance, moveDistance);
    double newX = x + moveDir(gen);
    double newY = y + moveDir(gen);
    
    // Проверка границ карты (0-100)
    if (newX >= 0 && newX <= 100 && newY >= 0 && newY <= 100) {
        x = newX;
        y = newY;
    }
}

void NPC::save(std::ostream& os) const {
    os << getType() << " " << x << " " << y << " " << name;
}

// Реализация Dragon
Dragon::Dragon(double x, double y, const std::string& name) 
    : NPC(x, y, name, 50, 30) {}  // Дракон: ход 50, убийство 30

std::string Dragon::getType() const { return "Dragon"; }

void Dragon::accept(Visitor& visitor) {
    (void)visitor; // Подавление предупреждения
}

bool Dragon::canAttack(NPC* other) {
    // Дракон может атаковать только быков
    if (auto bull = dynamic_cast<Bull*>(other)) {
        return true;
    }
    return false;
}

int Dragon::rollAttack(std::mt19937& gen) {
    std::uniform_int_distribution<> dist(1, 6);
    return dist(gen);
}

int Dragon::rollDefense(std::mt19937& gen) {
    std::uniform_int_distribution<> dist(1, 6);
    return dist(gen);
}

// Реализация Bull
Bull::Bull(double x, double y, const std::string& name) 
    : NPC(x, y, name, 30, 10) {}  // Бык: ход 30, убийство 10

std::string Bull::getType() const { return "Bull"; }

void Bull::accept(Visitor& visitor) {
    (void)visitor;
}

bool Bull::canAttack(NPC* other) {
    // Бык может атаковать только жаб
    if (auto toad = dynamic_cast<Toad*>(other)) {
        return true;
    }
    return false;
}

int Bull::rollAttack(std::mt19937& gen) {
    std::uniform_int_distribution<> dist(1, 6);
    return dist(gen);
}

int Bull::rollDefense(std::mt19937& gen) {
    std::uniform_int_distribution<> dist(1, 6);
    return dist(gen);
}

// Реализация Toad
Toad::Toad(double x, double y, const std::string& name) 
    : NPC(x, y, name, 1, 10) {}  // Жаба: ход 1, убийство 10

std::string Toad::getType() const { return "Toad"; }

void Toad::accept(Visitor& visitor) {
    (void)visitor;
}

bool Toad::canAttack(NPC* other) {
    // Жабы никого не могут атаковать
    (void)other;
    return false;
}

int Toad::rollAttack(std::mt19937& gen) {
    std::uniform_int_distribution<> dist(1, 6);
    return dist(gen);
}

int Toad::rollDefense(std::mt19937& gen) {
    std::uniform_int_distribution<> dist(1, 6);
    return dist(gen);
}