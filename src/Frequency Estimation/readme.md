# Source Codes for Frequency Estimation

## Discriptions

- `Sketch.h`: The base class for Stingy Sketch and related algorithms for frequency estimation
- `MurmurHash.h`: The hash function, from https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp

The contents of other sources are consistent with their names. The class name with PQ is the version that contains Prophet Queue. And the class name with HS is the version that contains Bounded Hash Split. You can get a pointer to the Sketch class through the function provided in `Choose_Ske.h`. All Sketch classes have interfaces Insert and Query. The number of bytes used, the number of hash functions, and the hash seed can be specified in the constructor of the Sketch class. 

## Run

We have prepared a sample of the CAIDA data set. To run, first run `make`, and a executable file `main` will be generate. Then `./main`, the result will be shown on the screen.

