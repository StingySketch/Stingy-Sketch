# Source Codes for Unbiased Top-𝑘 Detection

## Discriptions

- `abstract.h`: The base class for Stingy Sketch and related algorithms for frequency estimation
- `MurmurHash.h`: The hash function, from https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp

The contents of other sources are consistent with their names. You can get a pointer to the Sketch class through the function provided in `Choose_Topk.h`. All classes have interfaces Insert, Query and Query_topk. The number of bytes used, the number of hash functions, and the hash seed can be specified in the constructor of the Sketch class. 

## Run

We have prepared a sample of the CAIDA data set. To run, first run `make`, and a executable file `main` will be generate. Then `./main`, the result will be shown on the screen.

