#include "../include/visitor.h"

void Visitor::addObserver(std::shared_ptr<Observer> observer) {
    observers.push_back(observer);
}

void Visitor::notifyObservers(const std::string& attacker, const std::string& defender, bool defenderDied) {
    for (auto& observer : observers) {
        observer->onFight(attacker, defender, defenderDied);
    }
}

// Правила для варианта 20:
// Дракон ест быков
// Бык толчет жаб
// Жабы спасаются как могут (никого не убивают)

void Visitor::visit(Dragon& dragon, NPC& other) {
    if (auto bull = dynamic_cast<Bull*>(&other)) {
        notifyObservers(dragon.getName(), bull->getName(), true);
    }
}

void Visitor::visit(Bull& bull, NPC& other) {
    if (auto toad = dynamic_cast<Toad*>(&other)) {
        notifyObservers(bull.getName(), toad->getName(), true);
    }
}

void Visitor::visit(Toad& toad, NPC& other) {
    // Жабы никого не убивают
    notifyObservers(toad.getName(), other.getName(), false);
}