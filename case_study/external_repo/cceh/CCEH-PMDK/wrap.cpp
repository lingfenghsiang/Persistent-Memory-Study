// #include <fstream>
#include <unistd.h>

#include "src/CCEH.h"
#include "../../../include/cceh_pmdk.hpp"

PMEMobjpool *pop;
TOID(CCEH) HashTable;
void ccehpmdk::cceh::Init(std::string pool_dir, uint64_t pool_size) 
{
    
    bool exists = false;
    HashTable = OID_NULL;
    uint64_t initial_size = 1024 * 16;
    auto pool_path = pool_dir + "cceh_pmempool";
    if (access(pool_path.c_str(), 0) != 0)
    {
        pop = pmemobj_create(pool_path.c_str(), "CCEH", pool_size, 0666);
        if (!pop)
        {
            perror("pmemoj_create");
            exit(1);
        }
        HashTable = POBJ_ROOT(pop, CCEH);
        D_RW(HashTable)->initCCEH(pop, initial_size);
    }
    else
    {
        pop = pmemobj_open(pool_path.c_str(), "CCEH");
        if (pop == NULL)
        {
            perror("pmemobj_open");
            exit(1);
        }
        HashTable = POBJ_ROOT(pop, CCEH);
        if (D_RO(HashTable)->crashed)
        {
            D_RW(HashTable)->Recovery(pop);
        }
        exists = true;
    }
};
void ccehpmdk::cceh::CleanUp(){

};
void ccehpmdk::cceh::Touch(uint64_t key){
    D_RW(HashTable)->Touch(pop, key);
};
int ccehpmdk::cceh::Insert(uint64_t key, uint64_t value)
{
    D_RW(HashTable)->Insert(pop, key, reinterpret_cast<Value_t>(value));
    return 0;
};
int ccehpmdk::cceh::Update(uint64_t key, uint64_t value){

};
uint64_t ccehpmdk::cceh::Get(uint64_t key)
{
    auto ret = D_RW(HashTable)->Get(key);
    return (uint64_t)ret;
};
int ccehpmdk::cceh::Delete(uint64_t key){

};