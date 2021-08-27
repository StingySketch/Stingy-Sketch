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
class SalsaC:public Sketch {
private:
	const int step=31;
	char  hash_size;
    unsigned char *counter;
    bool *flag;
	uint64_t w, d, hash_seed,
		bias_range, bias_mask,
		index_range, index_mask;
	uint64_t (*hash_func)(const void*, int32_t, uint32_t);
public:
	SalsaC(uint32_t _w, uint32_t _d, uint32_t _hash_seed = 1000) {
		d             	= _d;
		hash_size	= (d == 2) ? 0x20 : 0x40;
		hash_func	= (d == 2) ? MurmurHash32 : MurmurHash64B;
		bias_range	= Min((hash_size - ceil(log2(_w))) / (d - 1),0x08);
        w = _w/8*7;
        w=w/d;
        index_range	= Min(hash_size - bias_range * d, ceil(log2(w)) - bias_range);
        bias_mask	= (((uint64_t)1) << bias_range) - 1;
		index_mask	= (((uint64_t)1) << index_range) - 1;
		hash_seed	= _hash_seed;		
		counter		= new unsigned char[w*d]();
        flag        = new bool[w*d]();
	}
    void update(uint32_t pos,uint32_t r,unsigned int now,unsigned char*counter,bool*flag){
        uint32_t len=r-pos+1;
        uint32_t tpos=(pos|len)-len,tr=tpos+(len<<1)-1;
        if(tr>=w)return;
        while(pos>tpos){
            pos--;
            if(!flag[pos]){
                now+=Ask(pos,counter,flag);
            }
        }
        while(r<tr){
            r++;
            if(!flag[r]){
                now+=Ask(r,counter,flag);
            }
        }
        while(r>pos){
            counter[r]=(now&0xff);
            now>>=8;
            flag[r]=1;
            r--;
        }
        counter[r]=(now&0xff);
    }
    void Add(uint32_t pos,int Sign,unsigned char*counter,bool*flag){
        uint32_t l=pos,r=pos+1;
        while(flag[l]==1)l--;
        while(flag[r]==1)r++;r--;
        int now=(char)counter[l];
        pos=l;
        for(l++;l<=r;l++){
            now<<=8;
            now+=counter[l];
        }
        now+=(Sign?-1:1);
        int k=(r-pos+1)*8-1;
        k=-(1<<k);
        if(now<k){
            update(pos,r,now,counter,flag);
            return;
        }
        k=-k;
        if(now>=k){
            update(pos,r,now,counter,flag);
            return;
        }
        while(r>pos){
            counter[r]=(now&0xff);
            now>>=8;
            r--;
        }
        counter[r]=(now&0xff);
        return;
    }
    unsigned int Ask(uint32_t pos,unsigned char*counter,bool*flag){
        uint32_t l=pos,r=pos+1;
        while(flag[l]==1)l--;
        while(flag[r]==1)r++;r--;
        int re=(char)counter[l];
        for(l++;l<=r;l++){
            re<<=8;
            re+=counter[l];
        }
        return re;
    }
	void Insert(const char* str) {
        uint32_t h_s=hash_seed;
        for(uint32_t i=0;i<d;i++){
            h_s=h_s*hash_seed+i;
            uint32_t hash_value=hash_func(str, KEY_LEN, h_s);
            Add((hash_value/2)%w,hash_value&1,counter+(i*w),flag+(i*w));
        }
	}
	int Query(const char* str) {
        vector<int> query;
        query.clear();
        uint32_t h_s=hash_seed;
        for(uint32_t i=0;i<d;i++){
            h_s=h_s*hash_seed+i;
            uint32_t hash_value=hash_func(str, KEY_LEN, h_s);
            int k=Ask((hash_value/2)%w,counter+(i*w),flag+(i*w));
            if(hash_value&1)k=-k;
            query.push_back(k);
		}
        sort(query.begin(),query.end());
        return query[(query.size()-1)/2];
	}
};


class SalsaC_HS:public Sketch {
private:
	const int step=31;
	char  hash_size;
    unsigned char *counter;
    bool *flag;
	uint64_t w, d, hash_seed,
		bias_range, bias_mask,
		index_range, index_mask;
	uint64_t (*hash_func)(const void*, int32_t, uint32_t);
public:
	SalsaC_HS(uint32_t _w, uint32_t _d, uint32_t _hash_seed = 1000) {
		d             	= _d;
		hash_size	= (d == 2) ? 0x20 : 0x40;
		hash_func	= (d == 2) ? MurmurHash32 : MurmurHash64B;
		bias_range	= Min((hash_size - ceil(log2(_w))) / (d - 1),0x08);
        _w=_w*7/8;
        _w/=d;
        w             	= (_w - (((d - 1) << bias_range) + 0x10)) >> bias_range << bias_range;
        index_range	= Min(hash_size - bias_range * d, ceil(log2(w)) - bias_range);
        bias_mask	= (((uint64_t)1) << bias_range) - 1;
		index_mask	= (((uint64_t)1) << index_range) - 1;
		hash_seed	= _hash_seed;		
		counter		= new unsigned char[d*((w + ((d - 1) << bias_range) + 0x10))](); 	
        flag		= new bool[d*((w + ((d - 1) << bias_range) + 0x10))](); 	
        
	}
    void update(uint32_t pos,uint32_t r,unsigned int now,unsigned char*counter,bool*flag){
        uint32_t len=r-pos+1;
        uint32_t tpos=(pos|len)-len,tr=tpos+(len<<1)-1;
        if(tr>=w)return;
        while(pos>tpos){
            pos--;
            if(!flag[pos]){
                now+=Ask(pos,counter,flag);
            }
        }
        while(r<tr){
            r++;
            if(!flag[r]){
                now+=Ask(r,counter,flag);
            }
        }
        while(r>pos){
            counter[r]=(now&0xff);
            now>>=8;
            flag[r]=1;
            r--;
        }
        counter[r]=(now&0xff);
    }
    void Add(uint32_t pos,int Sign,unsigned char*counter,bool*flag){
        uint32_t l=pos,r=pos+1;
        while(flag[l]==1)l--;
        while(flag[r]==1)r++;r--;
        int now=(char)counter[l];
        pos=l;
        for(l++;l<=r;l++){
            now<<=8;
            now+=counter[l];
        }
        now+=(Sign?-1:1);
        int k=(r-pos+1)*8-1;
        k=-(1<<k);
        if(now<k){
            update(pos,r,now,counter,flag);
            return;
        }
        k=-k;
        if(now>=k){
            update(pos,r,now,counter,flag);
            return;
        }
        while(r>pos){
            counter[r]=(now&0xff);
            now>>=8;
            r--;
        }
        counter[r]=(now&0xff);
        return;
    }
    unsigned int Ask(uint32_t pos,unsigned char*counter,bool*flag){
        uint32_t l=pos,r=pos+1;
        while(flag[l]==1)l--;
        while(flag[r]==1)r++;r--;
        int re=(char)counter[l];
        for(l++;l<=r;l++){
            re<<=8;
            re+=counter[l];
        }
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
			Add(addr,hash_value&1,counter+(i*((w + ((d - 1) << bias_range) + 0x10))),flag+(i*((w + ((d - 1) << bias_range) + 0x10))));
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

            int k=Ask(addr,counter+(i*((w + ((d - 1) << bias_range) + 0x10))),flag+(i*((w + ((d - 1) << bias_range) + 0x10))));
            if(hash_value&1)k=-k;
            query.push_back(k);
		}
        sort(query.begin(),query.end());
        return query[(query.size()-1)/2];
	}
};


class SalsaC_PQ:public Sketch {
private:
	const int step=31;
	char  clk,hash_size;
    unsigned char *counter;
    bool *flag;
	uint64_t w, d, hash_seed,*prefch[THR+1],
		bias_range, bias_mask,
		index_range, index_mask;
	uint64_t (*hash_func)(const void*, int32_t, uint32_t);
public:
	SalsaC_PQ(uint32_t _w, uint32_t _d, uint32_t _hash_seed = 1000) {
        clk		= 0;
		d             	= _d;
		hash_size	= (d == 2) ? 0x20 : 0x40;
		hash_func	= (d == 2) ? MurmurHash32 : MurmurHash64B;
		bias_range	= Min(((hash_size - ceil(log2(_w))) / (d - 1)),0x08);
		bias_mask	= (((uint64_t)1) << bias_range) - 1;
        _w=_w*7/8;
        _w/=d;
		w             	= (_w - (((d - 1) << bias_range) + 0x10)) >> bias_range << bias_range;
		index_range	= Min(hash_size - bias_range * d, ceil(log2(w)) - bias_range);
		index_mask	= (((uint64_t)1) << index_range) - 1;
		hash_seed	= _hash_seed;		
		counter		= new unsigned char[d*((w + ((d - 1) << bias_range) + 0x10))](); 	
        flag		= new bool[d*((w + ((d - 1) << bias_range) + 0x10))](); 	
        for(char i = 0;i <= THR;i++){
			prefch[i]=new uint64_t [d];
			for(char j = 0;j < d;j++) prefch[i][j] = (w + ((d - 1) << bias_range) + 1)<<1;	
		}
	}
    void update(uint32_t pos,uint32_t r,unsigned int now,unsigned char*counter,bool*flag){
        uint32_t len=r-pos+1;
        uint32_t tpos=(pos|len)-len,tr=tpos+(len<<1)-1;
        if(tr>=w)return;
        while(pos>tpos){
            pos--;
            if(!flag[pos]){
                now+=Ask(pos,counter,flag);
            }
        }
        while(r<tr){
            r++;
            if(!flag[r]){
                now+=Ask(r,counter,flag);
            }
        }
        while(r>pos){
            counter[r]=(now&0xff);
            now>>=8;
            flag[r]=1;
            r--;
        }
        counter[r]=(now&0xff);
    }
    void Add(uint32_t pos,int Sign,unsigned char*counter,bool*flag){
        uint32_t l=pos,r=pos+1;
        while(flag[l]==1)l--;
        while(flag[r]==1)r++;r--;
        int now=(char)counter[l];
        pos=l;
        for(l++;l<=r;l++){
            now<<=8;
            now+=counter[l];
        }
        now+=(Sign?-1:1);
        int k=(r-pos+1)*8-1;
        k=-(1<<k);
        if(now<k){
            update(pos,r,now,counter,flag);
            return;
        }
        k=-k;
        if(now>=k){
            update(pos,r,now,counter,flag);
            return;
        }
        while(r>pos){
            counter[r]=(now&0xff);
            now>>=8;
            r--;
        }
        counter[r]=(now&0xff);
        return;
    }
    unsigned int Ask(uint32_t pos,unsigned char*counter,bool*flag){
        uint32_t l=pos,r=pos+1;
        while(flag[l]==1)l--;
        while(flag[r]==1)r++;r--;
        int re=(char)counter[l];
        for(l++;l<=r;l++){
            re<<=8;
            re+=counter[l];
        }
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

            prefch[preclk][i]<<=1;
            prefch[preclk][i]|=hash_value&1;
			Add(prefch[clk][i]>>1,prefch[clk][i]&1,counter+(i*((w + ((d - 1) << bias_range) + 0x10))),flag+(i*((w + ((d - 1) << bias_range) + 0x10))));
        }
	}
    int Query(const char* str) {	
		char preclk = clk;
		do{
			clk = (clk + 1) & THR;
			for(char i = 0;i < d;i++){
				Add(prefch[clk][i]>>1,prefch[clk][i]&1,counter+(i*((w + ((d - 1) << bias_range) + 0x10))),flag+(i*((w + ((d - 1) << bias_range) + 0x10))));
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

            int k=Ask(addr,counter+(i*((w + ((d - 1) << bias_range) + 0x10))),flag+(i*((w + ((d - 1) << bias_range) + 0x10))));
            if(hash_value&1)k=-k;
            query.push_back(k);
		}
        sort(query.begin(),query.end());
        return query[(query.size()-1)/2];
	}
};