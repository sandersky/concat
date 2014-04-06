#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Make buffer size 8KB's
#define BUFFER_SIZE 8192
#define DEBUG 1

void freeStringArray(char **array, int count);
char getOutStream(char *outputPath, FILE **outStream);
char parseArgs(int argc, char *argv[], char **inputPaths, int *inputCount, char **outputPath);
void writeFiles(char **inputPaths, int inputCount, FILE *outStream);

int main(int argc, char *argv[]) {
  int inputCount;
  char **inputPaths = malloc(sizeof(char*) * argc);
  char *outputPath = NULL;

  #ifdef DEBUG
    printf("Parse args...\n");
  #endif

  if (parseArgs(argc, argv, inputPaths, &inputCount, &outputPath) == (char)-1) {
    freeStringArray(inputPaths, inputCount);

    if (outputPath != NULL) {
      free(outputPath);
    }

    exit(EXIT_FAILURE);
  }

  #ifdef DEBUG
    printf("Get output stream...\n");
  #endif

  FILE *outStream;
  char result = getOutStream(outputPath, &outStream);

  // We no longer need the output path so free it from memory if it was set
  if (outputPath != NULL) {
    free(outputPath);
  }

  // If it failed to open output file for writing to
  if (result == (signed char)-1) {
    freeStringArray(inputPaths, inputCount);
    exit(EXIT_FAILURE);
  }

  #ifdef DEBUG
    printf("Write files...\n");
  #endif

  writeFiles(inputPaths, inputCount, outStream);

  if (outStream != stdout) {
    fclose(outStream);
  }

  freeStringArray(inputPaths, inputCount);

  exit(EXIT_SUCCESS);
}

void freeStringArray(char **array, int count) {
  for (int i = 0; i < count; i++) {
    free(array[i]);
  }

  free(array);
}

char getOutStream(char *outputPath, FILE **outStream) {
  if (outputPath != NULL) {
    for (int i = strlen(outputPath) - 1; i >= 0; i--) {
      if (outputPath[i] == '/') {
        char directoryPath[i];
        memcpy(directoryPath, outputPath, i);
        directoryPath[i] = '\0';
        mkdir(directoryPath, 0700);
        break;
      }
    }

    *outStream = fopen(outputPath, "w");

    if (outStream < 0) {
      printf("Failed to open output file %s\n", outputPath);
      return -1;
    }
  } else {
    *outStream = stdout;
  }

  return 0;
}

char parseArgs(int argc, char *argv[], char **inputPaths, int *inputCount, char **outputPath) {
  *inputCount = 0;

  // Iterate arguments (skip first argument since it is this program)
  for (int i = 1; i < argc; i++) {
    // Check if argument starts with a '-' indicating it is an option
    if (argv[i][0] == '-') {
      if (argv[i][1] == 'o') {
        if (++i >= argc) {
          fprintf(stderr, "You must provide an output path when using -o command line option.\n");
          return -1;
        }
        int length = strlen(argv[i]);
        *outputPath = malloc(sizeof(char) * (length + 1));
        strcpy(*outputPath, argv[i]);
      } else {
        fprintf(stderr, "%s is not a valid command line option.\n", argv[i]);
        return -1;
      }
    } else {
      inputPaths[*inputCount] = malloc(sizeof(char) * (strlen(argv[i]) + 1));
      strcpy(inputPaths[*inputCount], argv[i]);
      (*inputCount)++;
    }
  }

  return 0;
}

void writeFiles(char **inputPaths, int inputCount, FILE *outStream) {
  char buffer[BUFFER_SIZE];

  for (int i = 0; i < inputCount; i++) {
    FILE *file = fopen(inputPaths[i], "r");

    if (file < 0) {
      fprintf(stderr, "Failed to open file at %s", inputPaths[i]);
      exit(EXIT_FAILURE);
    }

    while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
      fprintf(outStream, "%s", buffer);
    }

    fclose(file);
  }
}