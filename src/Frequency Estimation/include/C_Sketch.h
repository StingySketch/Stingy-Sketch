#ifndef _CSKETCH_H
#define _CSKETCH_H
#include "params.h"
#include <string.h>
#include <iostream>
#include "MurmurHash.h"
#include <random>
#include <mmintrin.h>

class C_Sketch:public Sketch
{
public:
	int w, d;
	int index[MAX_HASH_NUM];
	int* counter[MAX_HASH_NUM];
	int MAX_CNT, MIN_CNT, hash_seed;

public:
	C_Sketch(int _w, int _d, int _hash_seed = 1000)
	{
		d = _d, w = _w/4/_d;
		hash_seed = _hash_seed;
		for (int i = 0; i < d; i++)
		{
			counter[i] = new int[w];
			memset(counter[i], 0, sizeof(int) * w);
		}

		MAX_CNT = (1 << (COUNTER_SIZE - 1)) - 1;
		MIN_CNT = (-(1 << (COUNTER_SIZE - 1)));
	}

	void Insert(const char* str)
	{
		int g = 0;
		for (int i = 0; i < d; i++)
		{
			index[i] = ((unsigned)MurmurHash32(str, KEY_LEN, hash_seed + i)) % w;
			g =((unsigned) MurmurHash32(str, KEY_LEN, hash_seed + i + d)) % 2;

			if (g == 0)
			{
				if (counter[i][index[i]] != MAX_CNT)
				{
					counter[i][index[i]]++;
				}
			}
			else
			{
				if (counter[i][index[i]] != MIN_CNT)
				{
					counter[i][index[i]]--;
				}
			}
		}
	}

	void Delete(const char* str) {
		int g = 0;
		for (int i = 0; i < d; i++)
		{
			index[i] = ((unsigned)MurmurHash32(str, KEY_LEN, hash_seed + i)) % w;
			g = ((unsigned)MurmurHash32(str, KEY_LEN, hash_seed + i + d)) % 2;

			if (g == 1)
			{
				if (counter[i][index[i]] != MAX_CNT)
				{
					counter[i][index[i]]++;
				}
			}
			else
			{
				if (counter[i][index[i]] != MIN_CNT)
				{
					counter[i][index[i]]--;
				}
			}
		}
	}

	int Query(const char* str)
	{
		int temp;
		int res[MAX_HASH_NUM];
		int g;
		for (int i = 0; i < d; i++)
		{
			index[i] = ((unsigned)MurmurHash32(str, KEY_LEN, hash_seed + i)) % w;
			temp = counter[i][index[i]];
			g = ((unsigned)MurmurHash32(str, KEY_LEN, hash_seed + i + d)) % 2;

			res[i] = (g == 0 ? temp : -temp);
		}

		sort(res, res + d);
		if (d % 2 == 0)
			return ((res[d / 2] + res[d / 2 - 1]) / 2);
		else
			return (res[d / 2]);
	}

	~C_Sketch()
	{
		for (int i = 0; i < d; i++)
			delete[]counter[i];
	}
};

#include "params.h"
#include <string.h>
#include <iostream>
#include "MurmurHash.h"
#include <random>
#include <mmintrin.h>
#include <algorithm>
#define Min(a,b) 	((a) < (b) ? (a) : (b))
#define THR		(0x0f)
using namespace std;
class C_Sketch_HS:public Sketch {
private:
	const int step=31;
	char  hash_size;
    int *counter;
    bool *flag;
	uint64_t w, d, hash_seed,
		bias_range, bias_mask,
		index_range, index_mask;
	uint64_t (*hash_func)(const void*, int32_t, uint32_t);
public:
	C_Sketch_HS(uint32_t _w, uint32_t _d, uint32_t _hash_seed = 1000) {
		d             	= _d;
		hash_size	= (d == 2) ? 0x20 : 0x40;
		hash_func	= (d == 2) ? MurmurHash32 : MurmurHash64B;
		bias_range	= Min((hash_size - ceil(log2(_w))) / (d - 1),0x08);
        _w=_w/4;
        _w/=d;
        w             	= (_w - (((d - 1) << bias_range) + 0x10)) >> bias_range << bias_range;
        index_range	= Min(hash_size - bias_range * d, ceil(log2(w)) - bias_range);
        bias_mask	= (((uint64_t)1) << bias_range) - 1;
		index_mask	= (((uint64_t)1) << index_range) - 1;
		hash_seed	= _hash_seed;		
		counter		= new int[d*((w + ((d - 1) << bias_range) + 0x10))](); 	
        
	}
    void Add(uint32_t pos,int Sign,int*counter){
        Sign?counter[pos]--:counter[pos]++;
        return;
    }
    unsigned int Ask(uint32_t pos,int*counter){
        return counter[pos];
    }
	void Insert(const char* str) {
		uint64_t hash_value = hash_func(str, KEY_LEN, hash_seed);
		uint32_t index = ((hash_value & index_mask) << bias_range);
		int32_t tmp = index - w;
		index = tmp < 0?index : tmp;
		hash_value >>= index_range;
		for(char i = 0;i < d;i++){
			int addr = index + (i << bias_range) + (hash_value & bias_mask);
			hash_value >>= bias_range;
			Add(addr,hash_value&1,counter+(i*((w + ((d - 1) << bias_range) + 0x10))));
		}
	}
    int Query(const char* str) {	
		uint64_t hash_value = hash_func(str, KEY_LEN, hash_seed);
		uint32_t index = ((hash_value & index_mask) << bias_range);
		int32_t tmp = index - w;
		index = tmp < 0?index : tmp;
		hash_value >>= index_range;
        vector<int> query;
        query.clear();
		for(char i = 0;i < d;i++){
			int addr = index + (i << bias_range) + (hash_value & bias_mask);
			hash_value >>= bias_range;

            int k=Ask(addr,counter+(i*((w + ((d - 1) << bias_range) + 0x10))));
            if(hash_value&1)k=-k;
            query.push_back(k);
		}
        sort(query.begin(),query.end());
        return query[(query.size()-1)/2];
	}
};


class C_Sketch_PQ:public Sketch {
private:
	const int step=31;
	char  clk,hash_size;
    int *counter;
	uint64_t w, d, hash_seed,*prefch[THR+1],
		bias_range, bias_mask,
		index_range, index_mask;
	uint64_t (*hash_func)(const void*, int32_t, uint32_t);
public:
	C_Sketch_PQ(uint32_t _w, uint32_t _d, uint32_t _hash_seed = 1000) {
        clk		= 0;
		d             	= _d;
		hash_size	= (d == 2) ? 0x20 : 0x40;
		hash_func	= (d == 2) ? MurmurHash32 : MurmurHash64B;
		bias_range	= Min(((hash_size - ceil(log2(_w))) / (d - 1)),0x08);
		bias_mask	= (((uint64_t)1) << bias_range) - 1;
        _w=_w/4;
        _w/=d;
		w             	= (_w - (((d - 1) << bias_range) + 0x10)) >> bias_range << bias_range;
		index_range	= Min(hash_size - bias_range * d, ceil(log2(w)) - bias_range);
		index_mask	= (((uint64_t)1) << index_range) - 1;
		hash_seed	= _hash_seed;		
		counter		= new int[d*((w + ((d - 1) << bias_range) + 0x10))](); 	
        for(char i = 0;i <= THR;i++){
			prefch[i]=new uint64_t [d];
			for(char j = 0;j < d;j++) prefch[i][j] = (w + ((d - 1) << bias_range) + 1)<<1;	
		}
	}
    void Add(uint32_t pos,int Sign,int*counter){
        Sign?counter[pos]--:counter[pos]++;
        return;
    }
    unsigned int Ask(uint32_t pos,int*counter){
        return counter[pos];
    }
	void Insert(const char* str) {
		char preclk = clk;
		clk = (clk + 1) & THR;
		uint64_t hash_value = hash_func(str, KEY_LEN, hash_seed);
		uint32_t index = ((hash_value & index_mask) << bias_range);
		int32_t tmp = index - w;
		index = tmp < 0?index : tmp;
		hash_value >>= index_range;
		for(char i = 0;i < d;i++){
			prefch[preclk][i] = index+ (i << bias_range) + (hash_value & bias_mask);
			__builtin_prefetch(counter+(i*((w + ((d - 1) << bias_range) + 0x10))) + prefch[preclk][i]);
			hash_value >>= bias_range;

            prefch[preclk][i]<<=1;
            prefch[preclk][i]|=hash_value&1;
			Add(prefch[clk][i]>>1,prefch[clk][i]&1,counter+(i*((w + ((d - 1) << bias_range) + 0x10))));
        }
	}
    int Query(const char* str) {	
		char preclk = clk;
		do{
			clk = (clk + 1) & THR;
			for(char i = 0;i < d;i++){
				Add(prefch[clk][i]>>1,prefch[clk][i]&1,counter+(i*((w + ((d - 1) << bias_range) + 0x10))));
        		prefch[clk][i] = (w + ((d - 1) << bias_range) + 1)<<1;
			}
		}while(clk==preclk);
		uint64_t hash_value = hash_func(str, KEY_LEN, hash_seed);
		uint32_t index = ((hash_value & index_mask) << bias_range);
		int32_t tmp = index - w;
		index = tmp < 0?index : tmp;
		hash_value >>= index_range;
        vector<int> query;
        query.clear();
		for(char i = 0;i < d;i++){
			int addr = index + (i << bias_range) + (hash_value & bias_mask);
			hash_value >>= bias_range;

            int k=Ask(addr,counter+(i*((w + ((d - 1) << bias_range) + 0x10))));
            if(hash_value&1)k=-k;
            query.push_back(k);
		}
        sort(query.begin(),query.end());
        return query[(query.size()-1)/2];
	}
};




#endif
