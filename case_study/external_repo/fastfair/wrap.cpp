
#include "fastfair.h"
#include "../../include/fastfair.hpp"

// fastfair::fastfair
VMEM *vmp;
btree *bt;
void fast_fair::fast_fair::Init(std::string path_name, uint64_t pool_size)
{
    if ((vmp = vmem_create(path_name.c_str(),
                           pool_size)) == NULL)
    {
        perror("vmem_create");
        exit(1);
    }
    bt = new btree();
};
void fast_fair::fast_fair::CleanUp(){};
void fast_fair::fast_fair::Touch(uint64_t key){};
int fast_fair::fast_fair::Insert(uint64_t key, uint64_t value)
{
    bt->btree_insert((entry_key_t)key, (char *)value);
    return 0;
};
int fast_fair::fast_fair::Update(uint64_t key, uint64_t value)
{
    bt->btree_insert((entry_key_t)&key, (char *)&value);
    return 0;
};
uint64_t fast_fair::fast_fair::Get(uint64_t key)
{
    return (uint64_t)bt->btree_search((entry_key_t)key);
};
int fast_fair::fast_fair::Delete(uint64_t key){return 0;};