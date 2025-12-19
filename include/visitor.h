#ifndef VISITOR_H
#define VISITOR_H

#include "npc.h"
#include "observer.h"
#include <vector>
#include <memory>

// Visitor для боевых взаимодействий
class Visitor {
private:
    std::vector<std::shared_ptr<Observer>> observers;

public:
    void addObserver(std::shared_ptr<Observer> observer);
    void notifyObservers(const std::string& attacker, const std::string& defender, bool defenderDied);
    
    // Правила для варианта 20
    void visit(Dragon& dragon, NPC& other);
    void visit(Bull& bull, NPC& other);
    void visit(Toad& toad, NPC& other);
};

#endif