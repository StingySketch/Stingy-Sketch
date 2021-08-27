#pragma once

#include "abstract.h"

#define factor 1

template < uint32_t slot_num >
class WavingSketch : public Abstract {
public:
	struct Bucket {
		data_type items[slot_num];
		count_type counters[slot_num];
		count_type incast;

		void Insert(const data_type item) {
			uint32_t choice = hashfunc(item, 17) & 1;
			count_type min_num = INT_MAX;
			uint32_t min_pos = -1;

			for (uint32_t i = 0; i < slot_num; ++i) {
				if (counters[i] == 0) {
					items[i] = item;
					counters[i] = -1;
					return;
				}
				else if (items[i] == item) {
					if (counters[i] < 0)
						counters[i]--;
					else {
						counters[i]++;
						incast += COUNT[choice];
					}
					return;
				}

				count_type counter_val = std::abs(counters[i]);
				if (counter_val < min_num) {
					min_num = counter_val;
					min_pos = i;
				}
			}

			if (incast * COUNT[choice] >= int(min_num * factor)) {
				if (counters[min_pos] < 0) {
					uint32_t min_choice = hashfunc(items[min_pos], 17) & 1;
					incast -= COUNT[min_choice] * counters[min_pos];
				}
				items[min_pos] = item;
				counters[min_pos] = min_num + 1;
			}
			incast += COUNT[choice];
		}

		count_type Query(const data_type item) {
			uint32_t choice = hashfunc(item, 17) & 1;
			count_type re=incast;
			for (uint32_t i = 0; i < slot_num; ++i) {
				if (counters[i]<0) {
					uint32_t ch=hashfunc(items[i], 17) & 1;
					re-=counters[i]*COUNT[ch];
				}
			}
			return  re * COUNT[choice];
		}
	};

	WavingSketch(uint32_t _BUCKET_NUM,uint32_t _hash_seed = 1000)
	    : Abstract((char *)"WavingSketch"),BUCKET_NUM(_BUCKET_NUM/(8*slot_num+4)) {
		buckets = new Bucket[BUCKET_NUM];
		memset(buckets, 0, BUCKET_NUM * sizeof(Bucket));
		seed=_hash_seed;
	}
	~WavingSketch() { delete[] buckets; }

	void Insert(const data_type item) {
		uint32_t bucket_pos = hashfunc(item,seed) % BUCKET_NUM;
		buckets[bucket_pos].Insert(item);
	}

	count_type Query(const data_type item) {
		uint32_t bucket_pos = hashfunc(item,seed) % BUCKET_NUM;
		return buckets[bucket_pos].Query(item);
	}
	int Query_topk(vector<element>&ret,int topk){
        priority_queue<element,vector<element>,greater<element> >heap;
        while(heap.size()>0)heap.pop();
		for(int k=0;k<BUCKET_NUM;k++){
			for (uint32_t i = 0; i < slot_num; ++i){
				if(buckets[k].counters[i]!=0){
            		heap.push(element(buckets[k].items[i],abs(buckets[k].counters[i])));
					if(heap.size()>topk)heap.pop();
				}
			}
		}

        ret.clear();
        for(element x;heap.size()>0;heap.pop()){
            x=heap.top();
            ret.push_back(x);
        }
        return ret.size();
    }
private:
	Bucket *buckets;
	const uint32_t BUCKET_NUM;
	uint32_t seed;
};