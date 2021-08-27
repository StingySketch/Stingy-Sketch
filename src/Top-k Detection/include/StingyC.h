#pragma once
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
class StingyC {
private:
	const int step=31;
	char  hash_size, *counter;
	uint64_t w, d, hash_seed,
		bias_range, bias_mask,
		index_range, index_mask;
	uint64_t (*hash_func)(const void*, int32_t, uint32_t);
public:
	StingyC(uint32_t _w, uint32_t _d, uint32_t _hash_seed = 1000) {
		d             	= _d;
		hash_size	= (d == 2) ? 0x20 : 0x40;
		hash_func	= (d == 2) ? MurmurHash32 : MurmurHash64B;
		bias_range	= Min((hash_size - ceil(log2(_w))) / (d - 1),0x08);
        w             	= (_w - (((d - 1) << bias_range) + 0x10)) >> bias_range << bias_range;
        // w=8;
        index_range	= Min(hash_size - bias_range * d, ceil(log2(w)) - bias_range);
        bias_mask	= (((uint64_t)1) << bias_range) - 1;
		index_mask	= (((uint64_t)1) << index_range) - 1;
		hash_seed	= _hash_seed;		
		counter		= new char[(w + ((d - 1) << bias_range) + 0x10)](); 	
	}
	void kick(uint32_t pos,int K){//传入是低层pos,保证这个pos已经清空
        // return;
        // if(w==8)printf("kick %d %d ",pos,K);
        // Clear(pos,1);
        if(K==0)return;
        while((counter[pos]&0x3f)==0x20)
            pos=(step+pos)%w;
        // if(w==8)printf("to %d\n",pos);
        int now=Ask(pos);
        // if(w==8)printf("now=%d\n",now);
        bool sn=now<0,sk=K<0;
        if(sn==sk)
            Jin(pos,abs(K));
        else{
            now+=K;
            if(now==0){
                Clear(pos,0);
                return;
            }
            sk=now<0;
            if(sk==sn){
                Tui(pos,abs(K));
            }else{
                Clear(pos,sk);
                Jin(pos,abs(now));
            }
        }
    }
    void kicknode(uint32_t node,uint32_t plus){
        // return;
        int re=(counter[node]>>6)&3;
        counter[node]&=0x3f;
        uint32_t lowbit;
        while(!(node&1)){
            re=re*3;
            lowbit=node&(-node);
            node|=lowbit>>1;
            if(((counter[node]>>6)&3)==0)node^=lowbit;
            re+=(counter[node]>>6)&3;
            counter[node]&=0x3f;
        }
        if((counter[node]&0x1f)==0)node^=1;
        re=re*31;
        re+=counter[node]&0x1f;
        re+=plus;
        if(counter[node]&0x20)re=-re;
        counter[node]&=0xc0;
        counter[node]|=0x20;
        kick(node,re);
    }
    void Jin(uint32_t pos,uint32_t K){
        if((counter[pos]&0x1f)+K<32){
            counter[pos]+=K;
            return;
        }
        uint32_t k=(K-1)%31+1,base=31,cop=pos;
        K=(K-k)/31;
        if((counter[pos]&0x1f)+k<32)
            counter[pos]+=k;
        else{
            counter[pos]+=k-31;
            K++;
        }
        // uint32_t backup=counter[pos]&0x3f;
        // counter[pos]+=0x20-backup;
        //暂时搞成-0防止影响

        uint32_t fa,bro;
        fa=pos|1;bro=pos^1;
        if((counter[bro]&0x3f)!=0x20){
            uint32_t val=counter[bro];
            counter[bro]&=0xc0;
            counter[bro]|=0x20;
            kick(bro,val&0x20?-(val&0x1f):val&0x1f);
        }
        pos|=1;
        uint32_t lowbit=pos&(-pos);
        fa=(pos^lowbit)|(lowbit<<1);
        while(K&&((counter[fa]>>6)&3)!=0){
            base*=3;
            k=(K-1)%3+1;
            K=(K-k)/31;
            if(((counter[pos]>>6)&3)+k<4)
                counter[pos]+=k<<6;
            else
                counter[pos]+=(k-3)<<6,K++;
            pos=fa;
            lowbit=pos&(-pos);
            fa=(pos^lowbit)|(lowbit<<1);
        }
        //现在fa是00,pos还要加K
        base*=3;
        k=(K-1)%3+1;
        K=(K-k)/31;
        if(((counter[pos]>>6)&3)+k<4)
            counter[pos]+=k<<6;
        else
            counter[pos]+=(k-3)<<6,K++;
        bro=pos^(lowbit<<1);
        pos=fa;
        lowbit=pos&(-pos);
        fa=(pos^lowbit)|(lowbit<<1);
        while(K){
            if(counter[bro]&(3<<6))
                kicknode(bro,0);
            if(counter[fa]&(3<<6)){
                // counter[cop]+=backup-0x20;
                kicknode(pos,K*base);
                return;
            }
            base*=3;
            k=(K-1)%3+1;
            K=(K-k)/31;
            if(((counter[pos]>>6)&3)+k<4)
                counter[pos]+=k<<6;
            else
                counter[pos]+=(k-3)<<6,K++;
            bro=pos^(lowbit<<1);
            pos=fa;
            lowbit=pos&(-pos);
            fa=(pos^lowbit)|(lowbit<<1);
        }
        // counter[cop]+=backup-0x20;
        return;
    }
    void Jin(uint32_t pos){//只加1
        counter[pos]++;
        if(counter[pos]&0x1f)return;
        counter[pos]-=0x1f;
        uint32_t fa,bro;
        fa=pos|1;bro=pos^1;
        pos|=1;
        while((counter[pos]&(3<<6))==(3<<6)){
            counter[pos]-=2<<6;
            uint32_t lowbit=pos&(-pos);
            bro=pos^(lowbit<<1);
            pos=(pos^lowbit)|(lowbit<<1);
        }
        //最后还要在pos+1，并判断踢人
        counter[pos]+=1<<6;
        uint32_t lowbit=pos&(-pos);
        fa=(pos^lowbit)|(lowbit<<1);
        if(((counter[pos]&(3<<6))==(1<<6))&&(counter[fa]&(3<<6))){
            kicknode(pos,0);
            return;
        }
        if(((pos&1)==0)){
            if((counter[bro]&(3<<6))){//这里的bro是上一个pos的bro，即当前pos的另一个孩子
                kicknode(bro,0);
            }
        }else{
            if((counter[bro]&0x3f)!=0x20){
                uint32_t val=counter[bro];
                counter[bro]&=0xc0;
                counter[bro]|=0x20;
                kick(bro,val&0x20?-(val&0x1f):val&0x1f);
            }
        }
        return;
    }
    void Clear(uint32_t pos,uint32_t sign){
        //把pos这个位置变成0，+0还是-0取决于sign
        sign<<=5;
        counter[pos]&=0xc0;
        counter[pos]|=sign;
        pos|=1;
        while((counter[pos]&(0xc0))){
            counter[pos]&=0x3f;
            uint32_t lowbit=pos&(-pos);
            pos=(pos^lowbit)|(lowbit<<1);
        }
    }
    bool JieWei(uint32_t node,int K){
        if(!K)return 1;
        if(((counter[node]>>6)&3)==0)return 0;
        //先全借位，会出现最高的一些原来相等的位被多借了，则会在更高一位是-1，返回0，取消借位
        int k=(K-1)%3+1;
        K=(K-k)/3;
        uint32_t fa,lowbit=node&(-node);
        fa=(node^lowbit)|(lowbit<<1);
        if(((counter[node]>>6)&3)>k){
            counter[node]-=k<<6;
            JieWei(fa,K);
            return 1;
        }else{
            counter[node]+=(3-k)<<6,K++;
            bool re=JieWei(fa,K);
            if(!re)counter[node]-=3<<6;
            return re;
        }
    }
    void Tui(uint32_t pos,int K){
        //调用时要保证pos的值>K
        int k=(K-1)%31+1;
        K=(K-k)/31;
        if((counter[pos]&0x1f)>k)
            counter[pos]-=k;//这里进去的JIEWei里可能值=K
        else
            counter[pos]+=31-k,K++;
            //pos的值>K,底层必借,且上面的仍保证值>K
        JieWei(pos|1,K);
    }
    void Tui(uint32_t pos){//只减1
        if((counter[pos]&0x1f)>1){
            counter[pos]--;
            return;
        }
        uint32_t fa=pos|1;
        if((counter[fa]&(3<<6))==0){
            counter[pos]&=(3<<6);//现在就等于1，减一变回+0，返回
            return;
        }
        counter[pos]+=0x1e;//现在是1，借位变成0x1f
        pos=fa;
        uint32_t lowbit=pos&(-pos);
        fa=(pos^lowbit)|(lowbit<<1);
        while((counter[pos]&(3<<6))==(1<<6)){//需要借位
            if((counter[fa]&(3<<6))==0)break;
            counter[pos]+=2<<6;
            pos=fa;
            lowbit=pos&(-pos);
            fa=(pos^lowbit)|(lowbit<<1);
        }
        counter[pos]-=(1<<6);
    }
    void Add(uint32_t pos,int Sign){
        if((counter[pos]&0x3f)==0){
            counter[pos]+=1+(Sign<<5);
            return;
        }
        uint32_t sign=counter[pos]&0x20;
        ((bool)Sign^(bool)sign)?Tui(pos):Jin(pos);
        return;
    }
    int Ask(uint32_t pos){
        int sign=counter[pos]&0x20,ret=counter[pos]&0x1f,base=31;
        pos|=1;
        while((counter[pos]&(0xc0))){
            ret+=base*((counter[pos]>>6)&3);
            base*=3;
            uint32_t lowbit=pos&(-pos);
            pos=(pos^lowbit)|(lowbit<<1);
        }
        return sign?-ret:ret;
    }
	void Want_Add(uint32_t pos,int Sign){
        pos%=w;
        while((counter[pos]&0x3f)==0x20)
            pos=(step+pos)%w;
		Add(pos,Sign);
        // if(w==8)printf("add %u %u\n",pos,Sign);
	}
	int Want_Ask(uint32_t pos){
        pos%=w;
        while((counter[pos]&0x3f)==0x20)
            pos=(step+pos)%w;
        // if(w==8)printf("ask %u\n",pos);
		return Ask(pos);
	}
	void Insert(const char* str) {
		uint64_t hash_value = hash_func(str, KEY_LEN, hash_seed);
		uint32_t index = ((hash_value & index_mask) << bias_range);
		int32_t tmp = index - w;
		index = tmp < 0?index : tmp;
		// index%=w;// ?
		hash_value >>= index_range;
		for(char i = 0;i < d;i++){
			int addr = index + (i << bias_range) + (hash_value & bias_mask);
			hash_value >>= bias_range;
			Want_Add(addr,hash_value&1);
			// if((++counter[addr]) & 0x3f)continue;
			// counter[addr] -= 0x40;
			// Carry(addr|1);
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

            int k=Want_Ask(addr);
            if(hash_value&1)k=-k;
            query.push_back(k);
		}
        sort(query.begin(),query.end());
        return query[(query.size()-1)/2];
	}
	
    void check(){
        // printf("%d\n",w);
        for(uint32_t i=0;i<=w;i++){
            printf("%d:%d ",i,(int)((counter[i]>>6)&3));
        }
        puts("");
        for(uint32_t i=0;i<=w;i++){
            printf("%d:%d ",i,counter[i]&0x3f);
        }
        puts("");
        for(uint32_t i=0;i<=w;i++){
            printf("%d:%d ",i,(int)(counter[i]&0x20?-(counter[i]&0x1f):counter[i]&0x1f));
        }
        puts("");
    }
/*	void DownCarry(uint32_t loc) {
		counter[loc] -= 0x40;
		if(counter[loc] & 0xc0) return;
		counter[loc] -= 0x40;
		uint32_t tmp = loc & (-loc);
		DownCarry((loc | (tmp << 1)) ^ tmp);
	}
	void Delete(const char* str) {
		uint64_t hash_value = hash_func(str, KEY_LEN, hash_seed);
		uint32_t index = ((hash_value & index_mask) << bias_range) % w;
		hash_value >>= index_range;
		for(char i = 0;i < d;i++){
			int addr = index + (i << bias_range) + (hash_value & bias_mask);
			hash_value >>= bias_range;
			if((counter[addr]--) & 0x3f)continue;
			counter[addr] += 0x40;
			DownCarry(addr|1);
		}
	}
*/
	~StingyC(){
		 delete[] counter;
	}
};
