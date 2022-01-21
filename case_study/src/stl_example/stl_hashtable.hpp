#pragma once
#include <unordered_map>
#include <mutex>

#include "../../kv_wrap.hpp"

class StlHashTable : public Index<uint64_t, uint64_t>
{
public:
    void Touch(uint64_t key);
    int Insert(uint64_t key, uint64_t value);
    int Update(uint64_t key, uint64_t value);
    uint64_t Get(uint64_t key);
    int Delete(uint64_t key);
    std::unordered_map<uint64_t, uint64_t> map;
    std::mutex lock;

};
