# Parallel-Histogram-Generator
A parallel program that generates English alphabet histograms from multiple input files. It forks child processes for each file, calculates character counts, and writes the results to new files. Parent-child communication is managed using pipes and signals, ensuring efficient and concurrent processing.

# Overview
This project is a parallel program that takes a list of file names as input and produces a new set of files containing the English alphabet histograms for each input file. The program is designed to handle multiple files concurrently using a parent-child process structure with inter-process communication through pipes and signals.

# Features
-Parallel Processing: Utilizes multiple child processes to handle files concurrently.

-Signal Handling: Implements SIGCHLD signal handling to manage child process termination.

-Dynamic Memory Allocation: Reads files dynamically and allocates memory as needed.

-Error Handling: Provides robust error handling for file operations and process management.

# Requirements
Compiler: GCC with support for POSIX.1-2008

Libraries: Standard C libraries including unistd.h, signal.h, and string.h.

# Installation
1.) git clone https://github.com/yourusername/Parallel-Histogram-Generator.git

2.) cd Parallel-Histogram-Generator

3.) make

# Usage
1.) ./histogram_generator file1.txt file2.txt

2.) The program generates output files in the format filePID.hist, where PID is the process ID of the corresponding child.

# Command Line Arguments
The program accepts multiple file names as command line arguments.

If a file name is SIG, the corresponding child process will receive a SIGINT signal instead of a file name.

Example: ./histogram_generator file1.txt SIG file2.txt

# Output Format
The output histogram files will have the following format, with one letter count per line:

a 12

b 0

c 17

d 3

...

# Makefile targets:
histogram_generator: Compiles the main executable.

clean: Removes the executable and any temporary files.

# Known Issues
Ensure that the input files exist and are readable.

The program must be run in an environment supporting POSIX signals and process control.


