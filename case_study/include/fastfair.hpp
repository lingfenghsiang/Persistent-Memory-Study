#pragma once
#include <libpmemobj.h>
#include "../kv_wrap.hpp"

namespace fast_fair
{
    class fast_fair : public Index<uint64_t, uint64_t>
    {
    public:
        void Init(std::string, uint64_t);
        void CleanUp();
        void Touch(uint64_t key);
        int Insert(uint64_t key, uint64_t value);
        int Update(uint64_t key, uint64_t value);
        uint64_t Get(uint64_t key);
        int Delete(uint64_t key);
    };
}
