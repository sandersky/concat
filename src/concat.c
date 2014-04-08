#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
 * Write file to output stream
 *
 * @param[in] inputPath
 * @param[in] outStream
 */
void writeFile(char *inputPath, FILE *outStream);

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

void writeFile(char *inputPath, FILE *outStream) {
  FILE *file = fopen(inputPath, "r");

  if (file == NULL) {
    fprintf(stderr, "Failed to open file at %s\n", inputPath);
    exit(EXIT_FAILURE);
  }

  char c;
  int currentIndex = 0;
  char *filePath = malloc(sizeof(char) * 256);
  int filePathIndex = 0;
  int matchIndex = 0;
  int quotes = 0;
  int search = 1;

  while ((c = fgetc(file)) != EOF) {
    currentIndex++;

    fprintf(outStream, "%c", c);

    // If char is a new line, start new search and move to next char
    if (c == '\n') {
      search = 1;
      matchIndex = 0;
      continue;
    }

    // If we have already determined line isn't a require statement move to next char
    if (search == 0) {
      continue;
    }

    // If we are getting whitespace before '//=', 'require', or the file name then continue searching
    if ((matchIndex == 0 || matchIndex == 3 || matchIndex == 10) &&
      (c == '\t' || c == ' ')) {
      continue;
    }

    // If character is expected character in require string move on to next char
    if ((matchIndex == 0 && c == '/') ||
      (matchIndex == 1 && c == '/') ||
      (matchIndex == 2 && c == '=') ||
      (matchIndex == 3 && c == 'r') ||
      (matchIndex == 4 && c == 'e') ||
      (matchIndex == 5 && c == 'q') ||
      (matchIndex == 6 && c == 'u') ||
      (matchIndex == 7 && c == 'i') ||
      (matchIndex == 8 && c == 'r') ||
      (matchIndex == 9 && c == 'e')) {
      matchIndex++;
      continue;
    }

    // If we did not get expected character for require string stop search and move on to next char
    if (matchIndex >= 0 && matchIndex <= 9) {
      search = 0;
      continue;
    }

    // If single quotes before file path
    if (matchIndex == 10 && c == '\'') {
        quotes = 1;
        filePathIndex = 0;
        matchIndex++;
        continue;
    }

    // If double quotes before file path
    if (matchIndex == 10 && c == '"') {
      quotes = 2;
      filePathIndex = 0;
      matchIndex++;
      continue;
    }

    // If no quotes before expected non-whitespace char, stop search and move to next char
    if (matchIndex == 10) {
      search = 0;
      continue;
    }

    // If we have recieved full file path, load contents of file
    if (matchIndex == 11 && ((quotes == 1 && c == '\'') || (quotes == 2 && c == '"'))) {
      search = 0;
      filePath[filePathIndex] = '\0';

      // Make sure required file starts on a new line
      fprintf(outStream, "\n");

      filePath = realloc(filePath, sizeof(char) * filePathIndex);

      // Close input file handle while we read required file
      fclose(file);

      // Write required file where require statement is
      writeFile(filePath, outStream);

      // Now that we finishe reading required file reopen file handle and go to where we left off
      file = fopen(inputPath, "r");
      fseek(file, currentIndex, SEEK_CUR);

      continue;
    }

    if (matchIndex == 11) {
      filePath[filePathIndex++] = c;
      continue;
    }
  }

  free(filePath);

  fclose(file);
}

void writeFiles(char **inputPaths, int inputCount, FILE *outStream) {
  for (int i = 0; i < inputCount; i++) {
    writeFile(inputPaths[i], outStream);
  }
}