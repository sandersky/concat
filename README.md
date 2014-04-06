concat
======

This is a simple command line tool for concatenating multiple files.

Build
-----
In terminal go to the root directory of this codebase and run the following command:
```
make concat
```
This will create the executable `concat` in a `build` directory.

Usage
-----
### Options
####-o
This command line option allows you to specify a file to write the concatenated output to. The `-o` argument is to be followed with the file path of the output file.
### Examples
Concatenate two files with output written to STDOUT:
```
./build/concat file1.txt file2.txt
```
Concatenate two files and write output to a file by redirecting STDOUT:
```
./build/concat file1.txt file2.txt > out.txt
```
Concatenate two files and write output to a file by using command line argument to specify output file:
```
./build/concat -o out.txt file1.txt file2.txt
```
