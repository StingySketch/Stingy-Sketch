#include "abstract.h"
#include "sketch+heap.h"
#include "USS.h"
#include "WavingSketch.h"
Abstract* Choose_Topk(uint32_t w, uint32_t d, int topk,uint32_t hash_seed = 1000,int id=10){
	switch (id){
		case 0:return new skeheap(topk,w,d,hash_seed);
		case 1:return new WavingSketch<8>(w,hash_seed);
		case 2:return new USS(w);
	}
}
