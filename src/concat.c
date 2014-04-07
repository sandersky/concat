#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Size of buffer used when reading files
#define BUFFER_SIZE 8192

/**
 * Free an array of strings from memory
 *
 * @param[in] array arry of strings to free
 * @param[in] count number of strings in array
 */
void freeStringArray(char **array, int count);

/**
 * Get a file handle for an output stream based on a file path
 *
 * @param[in] outputPath
 * @param[out]outputStream
 * @return error -1 if error otherwise 0
 */
char getOutStream(char *outputPath, FILE **outStream);

/**
 * Parse command line arguments
 *
 * @param[in] argc argument count
 * @param[in] argv argument values
 * @param[out] inputPaths array of input file paths
 * @param[out] inputCount number of input file paths
 * @param[out] outputPath file path to write output to
 * @return error -1 if error otherwise 0
 */
char parseArgs(int argc, char *argv[], char **inputPaths, int *inputCount, char **outputPath);

/**
 * Write concatenation of input files to an output file
 *
 * @param[in] inputPaths
 * @param[in] inputCount
 * @param[in] outStream
 */
void writeFiles(char **inputPaths, int inputCount, FILE *outStream);

/**
 * Entry point of program
 *
 * @param[in] argc argument count
 * @param[in] argv argument values
 */
int main(int argc, char *argv[]) {
  int inputCount;
  char **inputPaths = malloc(sizeof(char*) * argc);
  char *outputPath = NULL;

  if (parseArgs(argc, argv, inputPaths, &inputCount, &outputPath) == (char)-1) {
    freeStringArray(inputPaths, inputCount);

    if (outputPath != NULL) {
      free(outputPath);
    }

    exit(EXIT_FAILURE);
  }

  FILE *outStream;
  char result = getOutStream(outputPath, &outStream);

  // If output path was set then free it since it is no longer needed
  if (outputPath != NULL) {
    free(outputPath);
  }

  // If getOutStream() failed to open output file for writing to
  if (result == (signed char)-1) {
    freeStringArray(inputPaths, inputCount);
    exit(EXIT_FAILURE);
  }

  writeFiles(inputPaths, inputCount, outStream);

  // If output stream is a file, make sure we close our file handle
  if (outStream != stdout) {
    fclose(outStream);
  }

  freeStringArray(inputPaths, inputCount);
  exit(EXIT_SUCCESS);
}

void freeStringArray(char **array, int count) {
  // Free each string in array
  for (int i = 0; i < count; i++) {
    free(array[i]);
  }

  // Free array itself
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
    if (strcmp(argv[i], "-o") == 0) {
      if (++i >= argc) {
        fprintf(stderr, "You must provide an output path when using -o command line option.\n");
        return -1;
      }
      int length = strlen(argv[i]);
      *outputPath = malloc(sizeof(char) * (length + 1));
      strcpy(*outputPath, argv[i]);
    } else if (argv[i][0] == '-') {
      fprintf(stderr, "%s is not a valid command line option.\n", argv[i]);
      return -1;
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