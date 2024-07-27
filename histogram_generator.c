#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

#define ALPHASIZE 26 // Define the size of the alphabet (26 letters)
#define FILENAMELENGTH 250 // Define the maximum file name length

// Function declarations
void cleanupResources(int numberOfChildren);
void writeHistogramToFile(int alphabetCount[], int childPID);
void sigchld_handler(int signalNumber);
void processFileAndCountLetters(int pipeIndex, char *filePath);
void exitCleaning(int pipeIndex);
void exitCleaningWithFile(int pipeIndex, FILE *inputFile);
void exitCleaningWithBuffer(int pipeIndex, FILE *inputFile, char *buffer);
void processContent(char *content, long size, int *count);
void finalizeProcess(int pipeIndex, FILE *inputFile, char *buffer);

int *pids; // Array to hold the PIDs of child processes
int processState = 0; // Variable to track the number of completed processes
int **myPipes; // Array to hold the pipes for inter-process communication

int main(int argc, char *argv[]) {
    // Check if there are input files provided
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> <file2> ... <fileN>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int numberOfChildren = argc - 1; // Number of child processes to create
    printf("Processing %d files...\n", numberOfChildren);

    // Setup signal handling for child process termination
    signal(SIGCHLD, sigchld_handler);
    pids = malloc(numberOfChildren * sizeof(pid_t)); // Allocate memory for child PIDs
    myPipes = malloc(numberOfChildren * sizeof(int *)); // Allocate memory for pipes
    
    // Check if memory allocation was successful
    if (!pids || !myPipes) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    // Initialize pipes for each child process
    for (int i = 0; i < numberOfChildren; ++i) {
        myPipes[i] = malloc(2 * sizeof(int));
        if (!myPipes[i] || pipe(myPipes[i]) == -1) {
            perror("Failed to initialize pipe");
            exit(EXIT_FAILURE);
        }
    }

    int i = 1; // Start processing from the first file

    // Create child processes to handle each file
    if(argc > 1) {
        do {
            int child_index = i - 1;
            int child_pid = fork();

            if(child_pid == 0){
                // Child process: process the file and count letters
                processFileAndCountLetters(child_index, argv[i]);
                exit(0); // Child exits after processing
            }
            else if(child_pid < 0){
                // Handle fork error
                perror("Error during fork");
                exit(EXIT_FAILURE);
            }
            else{
                // Parent process: store child PID
                pids[child_index] = child_pid;
            }

            i++; // Move to the next file
        } while(i < argc);
    }

    // Parent process waits for all child processes to complete
    while (processState < numberOfChildren) {
        sleep(1);
    }

    // Cleanup allocated resources
    cleanupResources(numberOfChildren);

    return EXIT_SUCCESS;
}

void cleanupResources(int numberOfChildren) {
    // Close and free the pipes, and free the PIDs array
    for (int i = 0; i < numberOfChildren; ++i) {
        close(myPipes[i][0]);
        free(myPipes[i]);
    }
    free(myPipes);
    free(pids);
}

void writeHistogramToFile(int alphabetCount[], int childPID) {
    char outputFile[FILENAMELENGTH];
    sprintf(outputFile, "file%d.hist", childPID);

    FILE *filePtr = fopen(outputFile, "w");
    if (!filePtr) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Write the alphabet counts to the file
    for (int j = 0; j < ALPHASIZE; ++j) {
        fprintf(filePtr, "%c %d\n", 'a' + j, alphabetCount[j]);
    }
    fclose(filePtr);
}

void sigchld_handler(int signalNumber) {
    int childPID, exitStatus, indexForChild, numBytesRead;

    // Handle all terminated child processes
    for (;;) {
        childPID = waitpid(-1, &exitStatus, WNOHANG);
        if (childPID <= 0)
            break;

        printf("Signal received from child %d\n", childPID);
        indexForChild = childPID % pids[0];

        if (WIFEXITED(exitStatus)) {
            close(myPipes[indexForChild][1]); // Close write end

            int histogramData[ALPHASIZE] = {0};
            numBytesRead = read(myPipes[indexForChild][0], histogramData, sizeof(histogramData));

            if (numBytesRead > 0) {
                writeHistogramToFile(histogramData, childPID);
            } else {
                printf("No data received from child.\n");
            }
            close(myPipes[indexForChild][0]);
        }
        else if (WIFSIGNALED(exitStatus)) {
            printf("Child %d exited with status %d (%s).\n", childPID, WEXITSTATUS(exitStatus), strsignal(WTERMSIG(exitStatus)));
        }
        processState++;
    }

    signal(SIGCHLD, sigchld_handler); // Re-register the signal handler
}

void finalizeProcess(int pipeIndex, FILE *inputFile, char *buffer) {
    fclose(inputFile);
    free(buffer);
    printf("Child process going for a nap...\n");
    sleep(10 + 3 * pipeIndex); // Sleep for a variable time
    printf("Child process waking up and terminating.\n");
    close(myPipes[pipeIndex][1]);
    exit(0);
}

void exitCleaningWithBuffer(int pipeIndex, FILE *inputFile, char *buffer) {
    fclose(inputFile);
    free(buffer);
    exitCleaning(pipeIndex);
}

void processFileAndCountLetters(int pipeIndex, char *filePath) {
    if(strcmp(filePath, "SIG") == 0){
        kill(getpid(), SIGINT); // Send SIGINT signal to itself
    }

    FILE *inputFile = fopen(filePath, "r");
    if (!inputFile) {
        perror("Error opening file");
        exitCleaning(pipeIndex);
    }

    fseek(inputFile, 0, SEEK_END);
    long fileSize = ftell(inputFile);
    rewind(inputFile);

    char *fileContent = malloc(fileSize + 1);
    if (!fileContent) {
        fprintf(stderr, "Failed to allocate memory for file content.\n");
        exitCleaningWithFile(pipeIndex, inputFile);
    }

    if (fread(fileContent, 1, fileSize, inputFile) != fileSize) {
        fprintf(stderr, "Failed to read the entire file.\n");
        exitCleaningWithBuffer(pipeIndex, inputFile, fileContent);
    }

    int alphabetCount[ALPHASIZE] = {0};
    processContent(fileContent, fileSize, alphabetCount);
    write(myPipes[pipeIndex][1], alphabetCount, sizeof(alphabetCount));

    finalizeProcess(pipeIndex, inputFile, fileContent);
}

void exitCleaning(int pipeIndex) {
    close(myPipes[pipeIndex][1]);
    exit(1);
}

void exitCleaningWithFile(int pipeIndex, FILE *inputFile) {
    fclose(inputFile);
    exitCleaning(pipeIndex);
}

void processContent(char *content, long size, int *count) {
    for (int i = 0; i < size; ++i) {
        if (isalpha(content[i])) {
            char letter = tolower(content[i]);
            count[letter - 'a']++;
        }
    }
}
