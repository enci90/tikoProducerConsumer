# Producer Consumer test
Purpose of the program
 	Compute the sum of the first 1024 integer numbers
	Constraints:
	- Use a circular buffer of 256 elements
	- 2 threads must be used: one writing in the circular buffer, one consuming and doing the sum

## Build
Build of the project is done by using scripts under windows using cygwin.
Script to build is located under /scripts/ folder.
./scripts/build.bat requires environment variables dependent on local path, which is possible to set using setenv. 
modify setenv file accordingly to your paths.
    WORKING_DIR is root of the project 
    CYGWIN_DIR is path for cygwin

Makefile is a future work. It is under developent, partly working but not fully tested.

## Debug 
Debug is done using gdb in vscode.
It is possible to configure debug using .vscode/launch.json configuration file

## Run
Production code is under dist folder, at  /dist/ProdConsTest.exe 
