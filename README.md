# Stingy Sketch

## Introduction

Frequency estimation of items in highly skewed data streams is a fundamental and hot problem in recent years. The literature demonstrates that sketch is the most promising solution, but no existing sketch can achieve both great precision and fast speed with limited memory. To address this issue, we propose a new sketch framework called Stingy Sketch. Stingy Sketch uses Bit-pinching Counter Tree to allocate the appropriate number of counters for distinct items, and uses a pipelined prefetch technique Prophet Queue to accelerate without losing precision. More importantly, the two techniques are cooperative so that Stingy Sketch can improve accuracy and speed simultaneously. The extensive experimental results show that Stingy Sketch is at most 50%more accurate than the state of the art (SOTA) of accuracy-oriented sketches and is at most33%faster than the SOTA of speed-oriented sketches.

## Descriptions

We provide the source codes of Stingy Sketch and the algorithms we compared with in the task of frequency estimation in `./src/Frequency Estimation`. 

We also the source codes of Stingy Sketch and the algorithms we compared with in the task of unbiased top-𝑘 detection  in`./src/Top-k Detection`. 

We have uploaded the pdf of the technical report at `./StingySketch_AccurateAndFastFrequencyEstimation(TechnicalReport).pdf`.

