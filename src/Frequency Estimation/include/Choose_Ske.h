#include "Sketch.h"
#include "SAC.h"
#include "SalsaCM.h"
#include "SalsaCount.h"
#include "CM_Sketch.h"
#include "CU_Sketch.h"
#include "C_Sketch.h"
#include "PCM_Sketch.h"
#include "A_Sketch.h"
#include "StingyCM.h"
#include "StingyCU.h"
#include "StingyCM_Base.h"
#include "StingyCU_Base.h"
Sketch* Choose_Sketch(uint32_t w, uint32_t d, uint32_t hash_seed = 1000,int id=10){
	switch (id){
		case 0:return new StingyCM(w,d,hash_seed);
		case 1:return new StingyCU(w,d,hash_seed);
		case 2:return new StingyCM_Base(w,d,hash_seed);
		case 3:return new StingyCU_Base(w,d,hash_seed);
		
		case 10:return new CM_Sketch(w,d,hash_seed);
		case 11:return new CM_Sketch_HS(w,d,hash_seed);
		case 12:return new CM_Sketch_PQ(w,d,hash_seed);
		case 20:return new CU_Sketch(w,d,hash_seed);
		case 21:return new CU_Sketch_HS(w,d,hash_seed);
		case 22:return new CU_Sketch_PQ(w,d,hash_seed);
		case 30:return new C_Sketch(w,d,hash_seed);
		case 31:return new C_Sketch_HS(w,d,hash_seed);
		case 32:return new C_Sketch_PQ(w,d,hash_seed);
		case 40:return new SAC(w,d,hash_seed);
		case 41:return new SAC_HS(w,d,hash_seed);
		case 42:return new SAC_PQ(w,d,hash_seed);
		case 50:return new SalsaCM(w,d,hash_seed);
		case 51:return new SalsaCM_HS(w,d,hash_seed);
		case 52:return new SalsaCM_PQ(w,d,hash_seed);
		case 60:return new SalsaC(w,d,hash_seed);
		case 61:return new SalsaC_HS(w,d,hash_seed);
		case 62:return new SalsaC_PQ(w,d,hash_seed);
		case 70:return new A_Sketch(w,d,hash_seed);
		case 80:return new PCM_Sketch(w,d,hash_seed);


		case 101:return new CM_Sketch(w,d,hash_seed);
		case 102:return new CM_Sketch_HS(w,d,hash_seed);
		case 103:return new CM_Sketch_PQ(w,d,hash_seed);
		case 104:return new CU_Sketch(w,d,hash_seed);
		case 105:return new CU_Sketch_HS(w,d,hash_seed);
		case 106:return new CU_Sketch_PQ(w,d,hash_seed);
		case 107:return new C_Sketch(w,d,hash_seed);
		case 108:return new C_Sketch_HS(w,d,hash_seed);
		case 109:return new C_Sketch_PQ(w,d,hash_seed);
		case 110:return new SAC(w,d,hash_seed);
		case 111:return new SAC_HS(w,d,hash_seed);
		case 112:return new SAC_PQ(w,d,hash_seed);
		case 113:return new SalsaCM(w,d,hash_seed);
		case 114:return new SalsaCM_HS(w,d,hash_seed);
		case 115:return new SalsaCM_PQ(w,d,hash_seed);
		case 116:return new SalsaC(w,d,hash_seed);
		case 117:return new SalsaC_HS(w,d,hash_seed);
		case 118:return new SalsaC_PQ(w,d,hash_seed);
		case 119:return new A_Sketch(w,d,hash_seed);
		case 120:return new PCM_Sketch(w,d,hash_seed);
	}
	return NULL;
}
