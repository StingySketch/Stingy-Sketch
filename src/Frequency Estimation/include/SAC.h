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
class SAC:public Sketch {
private:
	char  hash_size;
    unsigned short*counter;
	uint64_t w, d, hash_seed,
		bias_range, bias_mask,
		index_range, index_mask;
	uint64_t (*hash_func)(const void*, int32_t, uint32_t);
public:
	SAC(uint32_t _w, uint32_t _d, uint32_t _hash_seed = 1000) {
		d             	= _d;
		hash_size	= (d == 2) ? 0x20 : 0x40;
		hash_func	= (d == 2) ? MurmurHash32 : MurmurHash64B;
		bias_range	= Min((hash_size - ceil(log2(_w))) / (d - 1),0x08);
        w = _w/2;
        w=w/d;
        index_range	= Min(hash_size - bias_range * d, ceil(log2(w)) - bias_range);
        bias_mask	= (((uint64_t)1) << bias_range) - 1;
		index_mask	= (((uint64_t)1) << index_range) - 1;
		hash_seed	= _hash_seed;		
		counter		= new unsigned short[w*d]();
	}
    void Add(uint32_t pos,unsigned short*counter){
        // printf("add %d\n",pos);
        int sta=1<<15,base=1;
        unsigned short&now=counter[pos];
        while(now&sta){
            sta>>=1;
            base<<=1;
        }
        base=base*base;
        base--;
        if((rand()&base)==0)now++;
        return;
    }
    unsigned int Ask(uint32_t pos,unsigned short*counter){
        int sta=1<<15,base=1,re=0;
        pos=counter[pos];
        while(pos&sta){
            re+=(sta-1)*base*base;
            sta>>=1;
            base<<=1;
        }
        re+=((sta-1)&pos)*base*base;
        return re;
    }
	void Insert(const char* str) {
        uint32_t h_s=hash_seed;
        for(uint32_t i=0;i<d;i++){
            h_s=h_s*hash_seed+i;
            uint32_t hash_value=hash_func(str, KEY_LEN, h_s);
            Add(hash_value%w,counter+(i*w));
        }
	}
	int Query(const char* str) {
		unsigned int query=UINT32_MAX,k;
        uint32_t h_s=hash_seed;
		for(uint32_t i=0;i<d;i++){
            h_s=h_s*hash_seed+i;
            uint32_t hash_value=hash_func(str, KEY_LEN, h_s);
            k=Ask(hash_value%w,counter+(i*w));
            query=Min(k,query);
		}
		return query;
	}
    void check(){
        for(int i=0;i<w;i++)
        printf("%u ",counter[i]);
        puts("");
    }
};


class SAC_HS:public Sketch {
private:
	char  hash_size;
    unsigned short*counter;
	uint64_t w, d, hash_seed,
		bias_range, bias_mask,
		index_range, index_mask;
	uint64_t (*hash_func)(const void*, int32_t, uint32_t);
public:
	SAC_HS(uint32_t _w, uint32_t _d, uint32_t _hash_seed = 1000) {
		d             	= _d;
		hash_size	= (d == 2) ? 0x20 : 0x40;
		hash_func	= (d == 2) ? MurmurHash32 : MurmurHash64B;
		bias_range	= Min((hash_size - ceil(log2(_w))) / (d - 1),0x08);
        _w/=2;
        _w/=d;
        w             	= (_w - (((d - 1) << bias_range) + 0x10)) >> bias_range << bias_range;
        index_range	= Min(hash_size - bias_range * d, ceil(log2(w)) - bias_range);
        bias_mask	= (((uint64_t)1) << bias_range) - 1;
		index_mask	= (((uint64_t)1) << index_range) - 1;
		hash_seed	= _hash_seed;		
		counter		= new unsigned short[d*((w + ((d - 1) << bias_range) + 0x10))](); 	
	}
    void Add(uint32_t pos,unsigned short*counter){
        // printf("add %d\n",pos);
        int sta=1<<15,base=1;
        unsigned short&now=counter[pos];
        while(now&sta){
            sta>>=1;
            base<<=1;
        }
        base=base*base;
        base--;
        if((rand()&base)==0)now++;
        return;
    }
    unsigned int Ask(uint32_t pos,unsigned short*counter){
        int sta=1<<15,base=1,re=0;
        pos=counter[pos];
        while(pos&sta){
            re+=(sta-1)*base*base;
            sta>>=1;
            base<<=1;
        }
        re+=((sta-1)&pos)*base*base;
        return re;
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


class SAC_PQ:public Sketch {
private:
	char  clk,hash_size;
    unsigned short*counter;
	uint64_t w, d, hash_seed,*prefch[THR+1],
		bias_range, bias_mask,
		index_range, index_mask;
	uint64_t (*hash_func)(const void*, int32_t, uint32_t);
public:
	SAC_PQ(uint32_t _w, uint32_t _d, uint32_t _hash_seed = 1000) {
		clk		= 0;
		d             	= _d;
		hash_size	= (d == 2) ? 0x20 : 0x40;
		hash_func	= (d == 2) ? MurmurHash32 : MurmurHash64B;
		bias_range	= Min(((hash_size - ceil(log2(_w))) / (d - 1)),0x08);
		bias_mask	= (((uint64_t)1) << bias_range) - 1;
		_w/=2;
        _w/=d;
		w             	= (_w - (((d - 1) << bias_range) + 0x10)) >> bias_range << bias_range;
		index_range	= Min(hash_size - bias_range * d, ceil(log2(w)) - bias_range);
		index_mask	= (((uint64_t)1) << index_range) - 1;
		hash_seed	= _hash_seed;		
		counter		= new unsigned short[d*((w + ((d - 1) << bias_range) + 0x10))](); 	
		for(char i = 0;i <= THR;i++){
			prefch[i]=new uint64_t [d];
			for(char j = 0;j < d;j++) prefch[i][j] = w + ((d - 1) << bias_range) + 1;	
		}
	}
    void Add(uint32_t pos,unsigned short*counter){
        // printf("add %d\n",pos);
        int sta=1<<15,base=1;
        unsigned short&now=counter[pos];
        while(now&sta){
            sta>>=1;
            base<<=1;
        }
        base=base*base;
        base--;
        if((rand()&base)==0)now++;
        return;
    }
    unsigned int Ask(uint32_t pos,unsigned short*counter){
        int sta=1<<15,base=1,re=0;
        pos=counter[pos];
        while(pos&sta){
            re+=(sta-1)*base*base;
            sta>>=1;
            base<<=1;
        }
        re+=((sta-1)&pos)*base*base;
        return re;
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
