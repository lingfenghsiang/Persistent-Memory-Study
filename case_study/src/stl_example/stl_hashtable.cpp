#include "stl_hashtable.hpp"

void StlHashTable::Touch(uint64_t key){
    return;
};
int StlHashTable::Insert(uint64_t key, uint64_t value){
    std::lock_guard<std::mutex> guard(lock);
    map.emplace(key, value);
    return 0;
};
int StlHashTable::Update(uint64_t key, uint64_t value){
    map[key] = value;
    return 0;
};
uint64_t StlHashTable::Get(uint64_t key){
    return map[key];
};
int StlHashTable::Delete(uint64_t key){
    map.erase(key);
    return 0;
};
