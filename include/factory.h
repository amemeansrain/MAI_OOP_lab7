#ifndef FACTORY_H
#define FACTORY_H

#include <memory>
#include <sstream>
#include <string>

// Forward declaration
class NPC;

// Factory для создания NPC
class NPCFactory {
private:
    static std::string generateRandomName();
    
public:
    static std::shared_ptr<NPC> createNPC(const std::string& type, double x, double y, const std::string& name);
    static std::shared_ptr<NPC> createRandomNPC(double x, double y);
    static std::shared_ptr<NPC> loadFromStream(std::istream& is);
};

#endif