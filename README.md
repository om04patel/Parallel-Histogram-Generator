# Parallel-Histogram-Generator
A parallel program that generates English alphabet histograms from multiple input files. It forks child processes for each file, calculates character counts, and writes the results to new files. Parent-child communication is managed using pipes and signals, ensuring efficient and concurrent processing.

Overview

This project is a parallel program that takes a list of file names as input and produces a new set of files containing the English alphabet histograms for each input file. The program is designed to handle multiple files concurrently using a parent-child process structure with inter-process communication through pipes and signals.

