#include "MurmurHash.h"
# include "params.h"
# include <iostream>
# include <string.h>
using namespace std;
class CM_Sketch:public Sketch{
public:
	int w, d;
	int* counter[MAX_HASH_NUM];
	int COUNTER_SIZE_MAX_CNT = (1 << (COUNTER_SIZE - 1)) - 1;
	int hash_seed;
	int index[MAX_HASH_NUM];    //index of each d

public:
	CM_Sketch(int _w, int _d, int _hash_seed = 1000){
		w = _w/4/_d, d = _d;
		hash_seed=_hash_seed;
		for (int i = 0; i < d; i++)counter[i] = new int[w]();
	}
	void Insert(const char* str){
		int temp = 0, min_value = COUNTER_SIZE_MAX_CNT;
		for (int i = 0; i < d; i++){
			index[i] = MurmurHash32(str, KEY_LEN, hash_seed + i) % w;
			counter[i][index[i]]++;
		}
	}
	int Query(const char* str){
		int temp = 0, min_value = COUNTER_SIZE_MAX_CNT;
		for (int i = 0; i < d; i++){
			index[i] =  MurmurHash32(str, KEY_LEN, hash_seed + i) % w;
			temp = counter[i][index[i]];
			min_value = temp < min_value ? temp : min_value;
		}
		return min_value;
	}

	void Delete(const char* str){
		for (int i = 0; i < d; i++){
			index[i] =  MurmurHash32(str, KEY_LEN, hash_seed) % w;
			counter[i][index[i]] --;
		}
	}

	~CM_Sketch()
	{
		for (int i = 0; i < d; i++){
			delete[]counter[i];
		}
	}
};


class CM_Sketch_HS:public Sketch {
private:
	char  hash_size;
    unsigned int*counter;
	uint64_t w, d, hash_seed,
		bias_range, bias_mask,
		index_range, index_mask;
	uint64_t (*hash_func)(const void*, int32_t, uint32_t);
public:
	CM_Sketch_HS(uint32_t _w, uint32_t _d, uint32_t _hash_seed = 1000) {
		d             	= _d;
		hash_size	= (d == 2) ? 0x20 : 0x40;
		hash_func	= (d == 2) ? MurmurHash32 : MurmurHash64B;
		bias_range	= Min((hash_size - ceil(log2(_w))) / (d - 1),0x08);
        _w/=4;
        _w/=d;
        w             	= (_w - (((d - 1) << bias_range) + 0x10)) >> bias_range << bias_range;
        index_range	= Min(hash_size - bias_range * d, ceil(log2(w)) - bias_range);
        bias_mask	= (((uint64_t)1) << bias_range) - 1;
		index_mask	= (((uint64_t)1) << index_range) - 1;
		hash_seed	= _hash_seed;		
		counter		= new unsigned int[d*((w + ((d - 1) << bias_range) + 0x10))](); 	
	}
    void Add(uint32_t pos,unsigned int*counter){
        // printf("add %d\n",pos);
        counter[pos]++;
        return;
    }
    unsigned int Ask(uint32_t pos,unsigned int*counter){
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
			Add(addr,counter+(i*((w + ((d - 1) << bias_range) + 0x10))));
		}
	}
    int Query(const char* str) {
		uint32_t query = UINT32_MAX;	
		uint64_t hash_value = hash_func(str, KEY_LEN, hash_seed);
		uint32_t index = ((hash_value & index_mask) << bias_range);
		int32_t tmp = index - w;
		index = tmp < 0?index : tmp;
		hash_value >>= index_range;
		for(char i = 0;i < d;i++){
			int addr = index + (i << bias_range) + (hash_value & bias_mask);
			hash_value >>= bias_range;
			query = Min(Ask(addr,counter+(i*((w + ((d - 1) << bias_range) + 0x10)))), query);
		}
		return query;
	}
};


class CM_Sketch_PQ:public Sketch {
private:
	char  clk,hash_size;
    unsigned int*counter;
	uint64_t w, d, hash_seed,*prefch[THR+1],
		bias_range, bias_mask,
		index_range, index_mask;
	uint64_t (*hash_func)(const void*, int32_t, uint32_t);
public:
	CM_Sketch_PQ(uint32_t _w, uint32_t _d, uint32_t _hash_seed = 1000) {
		clk		= 0;
		d             	= _d;
		hash_size	= (d == 2) ? 0x20 : 0x40;
		hash_func	= (d == 2) ? MurmurHash32 : MurmurHash64B;
		bias_range	= Min(((hash_size - ceil(log2(_w))) / (d - 1)),0x08);
		bias_mask	= (((uint64_t)1) << bias_range) - 1;
		_w/=4;
        _w/=d;
		w             	= (_w - (((d - 1) << bias_range) + 0x10)) >> bias_range << bias_range;
		index_range	= Min(hash_size - bias_range * d, ceil(log2(w)) - bias_range);
		index_mask	= (((uint64_t)1) << index_range) - 1;
		hash_seed	= _hash_seed;		
		counter		= new unsigned int[d*((w + ((d - 1) << bias_range) + 0x10))](); 	
		for(char i = 0;i <= THR;i++){
			prefch[i]=new uint64_t [d];
			for(char j = 0;j < d;j++) prefch[i][j] = w + ((d - 1) << bias_range) + 1;	
		}
	}
    void Add(uint32_t pos,unsigned int*counter){
        // printf("add %d\n",pos);
        counter[pos]++;
        return;
    }
    unsigned int Ask(uint32_t pos,unsigned int*counter){
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
			Add(prefch[clk][i],counter+(i*((w + ((d - 1) << bias_range) + 0x10))));
		}
	}
    int Query(const char* str) {
		char preclk = clk;
		do{
			clk = (clk + 1) & THR;
			for(char i = 0;i < d;i++){
				Add(prefch[clk][i],counter+(i*((w + ((d - 1) << bias_range) + 0x10))));
				prefch[clk][i] = w + ((d - 1) << bias_range) + 1;
			}
		}while(clk==preclk);
		uint32_t query = UINT32_MAX;	
		uint64_t hash_value = hash_func(str, KEY_LEN, hash_seed);
		uint32_t index = ((hash_value & index_mask) << bias_range);
		int32_t tmp = index - w;
		index = tmp < 0?index : tmp;
		hash_value >>= index_range;
		for(char i = 0;i < d;i++){
			int addr = index + (i << bias_range) + (hash_value & bias_mask);
			hash_value >>= bias_range;
			query = Min(Ask(addr,counter+(i*((w + ((d - 1) << bias_range) + 0x10)))), query);
		}
		return query;
	}
};
