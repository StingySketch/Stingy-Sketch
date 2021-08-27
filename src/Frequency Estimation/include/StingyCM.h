#include "params.h"
#include <string.h>
#include <iostream>
#include "MurmurHash.h"
#include "params.h"
#include <random>
#include <mmintrin.h>
#define Min(a,b) 	((a) < (b) ? (a) : (b))
#define THR		(0x0F)
using namespace std;
class StingyCM:public Sketch {
private:
   char  hash_size, *counter;
	uint64_t w, d, hash_seed, *prefch[THR+1],
		bias_range, bias_mask,
		index_range, index_mask,clk;
	uint64_t (*hash_func)(const void*, int32_t, uint32_t);
	double index_fix;
public:
	StingyCM(uint32_t _w, uint32_t _d, uint32_t _hash_seed = 1000) {
		clk		= 0;
		d             	= _d;
		hash_size	= (d == 2) ? 0x20 : 0x40;
		hash_func	= (d == 2) ? MurmurHash32 : MurmurHash64B;
		bias_range	= ((hash_size - ceil(log2(_w))) / (d - 1));
		bias_mask	= (((uint64_t)1) << bias_range) - 1;
		w             	= (_w - (((d - 1) << bias_range) + 0x10)) >> bias_range << bias_range;
		index_range	= Min(hash_size - bias_range * d, ceil(log2(w)) - bias_range);
		index_mask	= (((uint64_t)1) << index_range) - 1;
		hash_seed	= _hash_seed;		
		counter		= new char[w + ((d - 1) << bias_range) + 0x10](); 
		for(int i = 0;i <= THR;i++){
			prefch[i]=new uint64_t [d];
			for(char j = 0;j < d;j++) prefch[i][j] = w + ((d - 1) << bias_range) + 0x8;	
		}	
	}
	void Carry(uint32_t loc) {
		counter[loc] += 0x40;
		if(counter[loc] & 0xc0) return;
		counter[loc] += 0x40;
		uint32_t tmp = loc & (-loc);
		Carry((loc | (tmp << 1)) ^ tmp);
	}
	void Insert(const char* str) {
		int preclk = clk;
		clk = (clk + 1) & THR;
		uint64_t hash_value = hash_func(str, KEY_LEN, hash_seed);
		uint32_t index = ((hash_value & index_mask) << bias_range);
		int32_t tmp = index - w;
		index = tmp < 0?index : tmp;
		hash_value >>= index_range;
		for(char i = 0;i < d;i++){
			prefch[preclk][i] = index+ (i << bias_range) + (hash_value & bias_mask);
			__builtin_prefetch(counter + prefch[preclk][i]);
			hash_value >>= bias_range;
			if((++counter[prefch[clk][i]]) & 0x3f)continue;
			counter[prefch[clk][i]] -= 0x40;
			Carry(prefch[clk][i]|1);
		}
	}
	int Calculate(uint32_t loc) {
		uint32_t query = counter[loc] & 0xc0;
		if (query == 0) return 0;
		uint32_t tmp = loc & (-loc);
		return (query >> 6) + Calculate((loc | (tmp << 1)) ^ tmp) * 0x03;
	}
	int Query(const char* str) {
		uint32_t query = UINT32_MAX;	
		uint64_t hash_value = hash_func(str, KEY_LEN, hash_seed);
		uint32_t index = ((hash_value & index_mask) << bias_range);
		int32_t tmp = index - w;
		index = tmp < 0?index : tmp;
		hash_value >>= index_range;
		for(char i = 0;i < d;i++){
			uint32_t addr = index + (i << bias_range) + (hash_value & bias_mask);
			hash_value >>= bias_range;
			query = Min((counter[addr] & 0x3f) + (Calculate(addr | 1) << 6), query);
		}
		return query;
	}

	~StingyCM(){
		 delete[] counter;
	}
};
