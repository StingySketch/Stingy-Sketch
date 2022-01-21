package stingyflink;

import com.google.common.base.Charsets;
import com.google.common.hash.Hashing;

public class StingySketchStruct implements java.io.Serializable {
    public static final int d = 4;
    public static final int w = 1048576*8;
    public static final int wdivd = w/d;

    public byte[] counter = new byte[w];

    public static long hash(long key,int seed){
        int m = 0x5bd1e995;
        int r = 24;
        int h1 = 4^seed;
        int h2 = 0;
        int data = (int)key;
        int k1 = data++;
        k1 *= m; k1 ^= k1 >>> r; k1 *= m;
        h1 *= m; h1 ^= k1;
        h2 *= m;
        h1 ^= h2 >>> 18; h1 *= m;
        h2 ^= h1 >>> 22; h2 *= m;
        h1 ^= h2 >>> 17; h1 *= m;
        h2 ^= h1 >>> 19; h2 *= m;
        if(h1<0)h1+=Integer.MAX_VALUE+1;
        if(h2<0)h2+=Integer.MAX_VALUE+1;
        long h = (long)h1;
        h = (h << 31) | h2;
        return h;
    }
    void Carry(int loc) {
		counter[loc] += 0x40;
		if((counter[loc] & 0xc0)!=0) return;
		counter[loc] += 0x40;
		int tmp=loc & (-loc);
		Carry((loc | (tmp << 1)) ^ tmp);
	}
    public void insert(int id){
        long[] hash=new long[4];
        hash[0]=hash((long)id,23);
        hash[1]=hash((long)id,27);
        hash[2]=hash((long)id,37);
        hash[3]=hash((long)id,43);
        // System.out.println(hash[0]);
        // System.out.println(hash[1]);
        // hash[0]=id;
        // hash[1]=id;
        for(int i = 0;i < d;i++){
			int addr = (int)(i*wdivd+hash[i]%wdivd);
            counter[addr]++;
			if((counter[addr] & 0x3f)!=0)continue;
			counter[addr] -= 0x40;
			Carry(addr|1);
		}
    }
    int Calculate(int loc) {
		int query = counter[loc] & 0xc0;
		if (query == 0) return 0;
		int tmp = loc & (-loc);
		return (query >>> 6) + Calculate((loc | (tmp << 1)) ^ tmp) * 0x03;
	}
    public int query(int id){
        long[] hash=new long[4];
        hash[0]=hash((long)id,23);
        hash[1]=hash((long)id,27);
        hash[2]=hash((long)id,37);
        hash[3]=hash((long)id,43);
        int re=Integer.MAX_VALUE;
        for(int i = 0;i < d;i++){
			int addr = (int)(i*wdivd+hash[i]%wdivd);
            int th=(counter[addr] & 0x3f)+ (Calculate(addr | 1) << 6);
            if(th<re)re=th;
		}
        return re;
    }
    // public static void main(String[] args) {
    //     StingySketchStruct sketch=new StingySketchStruct();
    //     for(int i=1;i<=64*4+5;i++)
    //     sketch.insert(1);
    //     // for(int i=1;i<=64*4+3;i++)
    //     // sketch.insert(0);
    //     for(int i=0;i<=7;i++)
    //     System.out.println(sketch.counter[i]);
    //     // for(int i=0;i<=4;i++)
    //     // System.out.println(sketch.query(i));

    //     System.out.println("alb");
    // }
}
