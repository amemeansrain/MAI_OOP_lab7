#include "../include/factory.h"
#include "../include/npc.h"
#include <random>
#include <string>
#include <vector>

using namespace std;

// Генерирует случайное имя для NPC
string NPCFactory::generateRandomName() {
    static const vector<string> prefixes = {"Синдзи", "Сюнсуй", "Кенпачи", "Бьякуя", "Тоширо"};
    static const vector<string> suffixes = {"Хирако", "Кьераку", "Зараки", "Кучики", "Хицугая"};
    
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> prefixDist(0, static_cast<int>(prefixes.size()) - 1);
    uniform_int_distribution<> suffixDist(0, static_cast<int>(suffixes.size()) - 1);
    
    return prefixes[prefixDist(gen)] + "_" + suffixes[suffixDist(gen)];
}

// Создает NPC указанного типа
shared_ptr<NPC> NPCFactory::createNPC(const string& type, double x, double y, const string& name) {
    if (type == "Dragon") {
        return make_shared<Dragon>(x, y, name);
    }
    if (type == "Bull") {
        return make_shared<Bull>(x, y, name);
    }
    if (type == "Toad") {
        return make_shared<Toad>(x, y, name);
    }
    
    // Возвращаем пустой shared_ptr
    return shared_ptr<NPC>();
}

// Создает случайного NPC в указанных координатах
shared_ptr<NPC> NPCFactory::createRandomNPC(double x, double y) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> typeDist(0, 2);
    
    string name = generateRandomName();
    
    switch (typeDist(gen)) {
        case 0:
            return make_shared<Dragon>(x, y, name);
        case 1:
            return make_shared<Bull>(x, y, name);
        case 2:
            return make_shared<Toad>(x, y, name);
        default:
            return shared_ptr<NPC>();
    }
}

// Загружает NPC из потока
shared_ptr<NPC> NPCFactory::loadFromStream(istream& is) {
    string type, name;
    double x, y;
    
    if (is >> type >> x >> y) {
        // Читаем остаток строки как имя
        getline(is, name);
        
        // Убираем начальные пробелы
        size_t start = name.find_first_not_of(" \t");
        if (start != string::npos) {
            name = name.substr(start);
        } else {
            name = "Unnamed_" + generateRandomName();
        }
        
        return createNPC(type, x, y, name);
    }
    
    return shared_ptr<NPC>();
}