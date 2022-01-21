# Source Codes for Flink Experiment

## Discriptions

- `stingyflink`: We implement Stingy Sketch in Java, where the data structure is located in `StingySketchStruct.java`, and the rest is for the implementation on Flink
- `StingyFlink.jar`: For convenience, we provide a successfully compiled jar package, you can get this package by compiling the source code in `stingyflink` with Maven
- `CAIDA.dat`: A fragment taken from the CAIDA dataset, which contains 1,000,000 lines, and formatted for Flink's needs. Specifically, each line of the input file should contain an Integer and a Long representing the ID and timestamp, respectively


## Requirements

- `Flink 1.13.1`

- `Hadoop 2.8.3`
## Run
1. First, you need several hosts and build HDFS on them, then upload the dataset to this file system
2. Then, you need to run the Flink cluster
3. Now you can run scripts on Flink. 

For example, assuming that the namenode running port of HDFS is xxx:9000 and the dataset is stored in the root directory of HDFS, you can run our program with the following command:
```
{FLINK_HOME}/bin/flink run StingyFlink.jar --input hdfs://xxx:9000/dataset
```
