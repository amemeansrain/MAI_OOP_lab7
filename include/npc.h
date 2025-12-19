#ifndef NPC_H
#define NPC_H

#include <string>
#include <iostream>
#include <memory>
#include <random>
#include <mutex>

class Visitor;

// Абстрактный класс NPC
class NPC {
protected:
    double x, y;
    std::string name;
    bool alive;
    int moveDistance;  // Расстояние хода за один шаг
    int killDistance;  // Расстояние для атаки

public:
    NPC(double x, double y, const std::string& name, int moveDist, int killDist);
    virtual ~NPC() = default;

    double getX() const { return x; }
    double getY() const { return y; }
    const std::string& getName() const { return name; }
    bool isAlive() const { return alive; }
    void die() { alive = false; }
    int getMoveDistance() const { return moveDistance; }
    int getKillDistance() const { return killDistance; }
    
    virtual std::string getType() const = 0;
    virtual std::string getTypeSymbol() const = 0; // Символ для отображения на карте

    double distanceTo(const NPC& other) const;
    void move(std::mt19937& gen);
    virtual void save(std::ostream& os) const;
    virtual void accept(Visitor& visitor) = 0;
    
    // Методы для боя
    virtual bool canAttack(NPC* other) = 0;
    virtual int rollAttack(std::mt19937& gen) = 0;
    virtual int rollDefense(std::mt19937& gen) = 0;
};

// Конкретные классы NPC
class Dragon : public NPC {
public:
    Dragon(double x, double y, const std::string& name);
    std::string getType() const override;
    std::string getTypeSymbol() const override { return "D"; }
    void accept(Visitor& visitor) override;
    
    bool canAttack(NPC* other) override;
    int rollAttack(std::mt19937& gen) override;
    int rollDefense(std::mt19937& gen) override;
};

class Bull : public NPC {
public:
    Bull(double x, double y, const std::string& name);
    std::string getType() const override;
    std::string getTypeSymbol() const override { return "B"; }
    void accept(Visitor& visitor) override;
    
    bool canAttack(NPC* other) override;
    int rollAttack(std::mt19937& gen) override;
    int rollDefense(std::mt19937& gen) override;
};

class Toad : public NPC {
public:
    Toad(double x, double y, const std::string& name);
    std::string getType() const override;
    std::string getTypeSymbol() const override { return "T"; }
    void accept(Visitor& visitor) override;
    
    bool canAttack(NPC* other) override;
    int rollAttack(std::mt19937& gen) override;
    int rollDefense(std::mt19937& gen) override;
};

#endif