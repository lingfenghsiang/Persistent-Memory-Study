#pragma once
#include "global.hpp"

template <typename T1, typename T2>
class Index{
    public:
    virtual void Touch(T1 key)=0;
    virtual int Insert(T1 key, T2 value)=0;
    virtual int Update(T1 key, T2 value)=0;
    virtual T2 Get(T1 key)=0;
    virtual int Delete(T1 key)=0;
};