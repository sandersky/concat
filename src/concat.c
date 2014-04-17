#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FAILURE -1
#define SUCCESS 0

typedef struct {
    int inputCount;
    char **inputPaths;
    char *outputPath;
} CONFIG;

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
int getOutStream(char *outputPath, FILE **outStream);

/**
 * Parse command line arguments
 *
 * @param[in] argc argument count
 * @param[in] argv argument values
 * @param[in|out] config object that stores configuration
 * @return error -1 if error otherwise 0
 */
int parseArgs(int argc, char *argv[], CONFIG *config);

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
    CONFIG config;
    config.inputPaths = malloc(sizeof(char*) * argc);
    config.outputPath = NULL;

    if (parseArgs(argc, argv, &config) == FAILURE) {
        freeStringArray(config.inputPaths, config.inputCount);

        if (config.outputPath != NULL) {
            free(config.outputPath);
        }

        exit(EXIT_FAILURE);
    }

    FILE *outStream;
    char result = getOutStream(config.outputPath, &outStream);

    // If output path was set then free it since it is no longer needed
    if (config.outputPath != NULL) {
        free(config.outputPath);
    }

    // If getOutStream() failed to open output file for writing to
    if (result == FAILURE) {
        freeStringArray(config.inputPaths, config.inputCount);
        exit(EXIT_FAILURE);
    }

    writeFiles(config.inputPaths, config.inputCount, outStream);

    // If output stream is a file, make sure we close our file handle
    if (outStream != stdout) {
        fclose(outStream);
    }

    freeStringArray(config.inputPaths, config.inputCount);
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

int getOutStream(char *outputPath, FILE **outStream) {
    // If an output path was provided
    if (outputPath != NULL) {
        // Look for '/' to determine if output path includes directories
        for (int i = strlen(outputPath) - 1; i >= 0; i--) {
            // If output path includes directories
            if (outputPath[i] == '/') {
                // Get directory portion of output path
                char directoryPath[i];
                memcpy(directoryPath, outputPath, i);
                directoryPath[i] = '\0';

                // Make directory in the event it doesn't already exist
                mkdir(directoryPath, 0700);

                // Leave for loop since we already determined directory portion of output path
                break;
            }
        }

        // Open output path and make output stream
        *outStream = fopen(outputPath, "w");

        // If program failed to open output path print error and return
        if (outStream < 0) {
            printf("Failed to open output file %s\n", outputPath);
            return FAILURE;
        }

    // If an output path wasn't specified use STDOUT as output stream
    } else {
        *outStream = stdout;
    }

    return SUCCESS;
}

int parseArgs(int argc, char *argv[], CONFIG *config) {
    config->inputCount = 0;

    // Iterate arguments (skip first argument since it is this program)
    for (int i = 1; i < argc; i++) {
        // If argument is output flag
        if (strcmp(argv[i], "-o") == 0) {
            // If the output flag was specified without an output path
            if (++i >= argc) {
                fprintf(stderr, "You must provide an output path when using -o command line option.\n");
                return FAILURE;
            }

            // Allocate space for output path and set output path
            int length = strlen(argv[i]);
            config->outputPath = malloc(sizeof(char) * (length + 1));

            // Copy output path argument to out parameter
            strcpy(config->outputPath, argv[i]);

        // If argument is unknown option
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "%s is not a valid command line option.\n", argv[i]);
            return FAILURE;

        // If argument isn't a flag assume it is an input path
        } else {
            // Allocate space for input path
            config->inputPaths[config->inputCount] = malloc(sizeof(char) * (strlen(argv[i]) + 1));

            // Copy input path argument to input paths array
            strcpy(config->inputPaths[config->inputCount], argv[i]);

            // Keep track of how many input paths are allocated
            config->inputCount++;
        }
    }

    return SUCCESS;
}

void writeFile(char *inputPath, FILE *outStream) {
    FILE *file = fopen(inputPath, "r");

    if (file == NULL) {
        fprintf(stderr, "Failed to open file at %s\n", inputPath);
        exit(EXIT_FAILURE);
    }

    char c;
    int currentIndex = 0;

    // Allocate space for a file name of length 64
    int fileLength = 64;
    char *filePath = malloc(sizeof(char) * fileLength);
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

        // If we are currently retrieving the file path for a required file
        if (matchIndex == 11) {
            // If file path is larger than currently allocated space, allocate space for another 64 chars
            if (currentIndex > fileLength) {
                fileLength += 64;
                filePath = realloc(filePath, sizeof(char) * fileLength);
            }

            // Add char to file path
            filePath[filePathIndex++] = c;
            continue;
        }
    }

    free(filePath);

    fclose(file);
}

void writeFiles(char **inputPaths, int inputCount, FILE *outStream) {
    // Iterate input files and write each one
    for (int i = 0; i < inputCount; i++) {
        writeFile(inputPaths[i], outStream);
    }
}