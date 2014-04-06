#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 8192

void parseArgs(int argc, char *argv[], char **inputPaths, int *inputCount, char **outputPath);
void writeFiles(char **inputPaths, int inputCount, FILE *outStream);

int main(int argc, char *argv[]) {
  int inputCount;
  char **inputPaths = malloc(sizeof(char*) * argc);
  char *outputPath = NULL;

  parseArgs(argc, argv, inputPaths, &inputCount, &outputPath);

  FILE *outStream;
  if (outputPath != NULL) {
    outStream = fopen(outputPath, "w");
    if (outStream < 0) {
      printf("Failed to open output file %s\n", outputPath);
      free(outputPath);
      exit(EXIT_FAILURE);
    }
    free(outputPath);
  } else {
    outStream = stdout;
  }

  writeFiles(inputPaths, inputCount, outStream);

  if (outStream != stdout) {
    fclose(outStream);
  }

  for (int i = 0; i < inputCount; i++) {
    free(inputPaths[i]);
  }

  free(inputPaths);

  exit(EXIT_SUCCESS);
}

void parseArgs(int argc, char *argv[], char **inputPaths, int *inputCount, char **outputPath) {
  *inputCount = 0;

  // Iterate arguments (skip first argument since it is this program)
  for (int i = 1; i < argc; i++) {
    // Check if argument starts with a '-' indicating it is an option
    if (argv[i][0] == '-') {
      if (argv[i][1] == 'o') {
        if (++i >= argc) {
          fprintf(stderr, "You must provide an output path when using -o command line option.\n");
          exit(EXIT_FAILURE);
        }
        int length = strlen(argv[i]);
        *outputPath = malloc(sizeof(char) * (length + 1));
        strcpy(*outputPath, argv[i]);
      } else {
        fprintf(stderr, "%s is not a valid command line option.\n", argv[i]);
        exit(EXIT_FAILURE);
      }
    } else {
      inputPaths[*inputCount] = malloc(sizeof(char) * (strlen(argv[i]) + 1));
      strcpy(inputPaths[*inputCount], argv[i]);
      (*inputCount)++;
    }
  }
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