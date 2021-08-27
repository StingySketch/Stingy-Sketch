#pragma once

#include "abstract.h"
#include<ext/pb_ds/priority_queue.hpp>
#include "StingyC.h"
using namespace std;
typedef __gnu_pbds::priority_queue<element,greater<element>,__gnu_pbds::pairing_heap_tag > Heap;
#define MP make_pair
struct skeheap :public Abstract{
    int K;
    StingyC ske;
    Heap heap;
    unordered_map<data_type,Heap::point_iterator>inheap;
    skeheap(int topk,int sz,int hs=3,uint32_t _hash_seed = 1000):K(topk),ske(sz-topk*32,hs,_hash_seed),heap(),inheap(),Abstract((char *)"Ske+Heap"){}
    void Insert(const data_type item){
        ske.Insert((char*)&item);
        double myval=ske.Query((char*)&item);
        if(inheap.count(item))
            heap.modify(inheap[item],element(item,myval));
        else{
            if(K>0&&((int)heap.size()<K||myval>heap.top().count)){
                if((int)heap.size()>=K){
                    element useless=heap.top();heap.pop();
                    inheap.erase(useless.item);
                }
                inheap[item]=heap.push(element(item,myval));
            }
        }
    }
    count_type Query(const data_type item){
        return ske.Query((char*)&item);
    }
    int Query_topk(vector<element>&ret,int topk){
        ret.clear();
        for(element x;heap.size()>0;heap.pop()){
            x=heap.top();
            ret.push_back(x);
        }
        return ret.size();
    }
};
