# Parallel-Histogram-Generator
A parallel program that generates English alphabet histograms from multiple input files. It forks child processes for each file, calculates character counts, and writes the results to new files. Parent-child communication is managed using pipes and signals, ensuring efficient and concurrent processing.
