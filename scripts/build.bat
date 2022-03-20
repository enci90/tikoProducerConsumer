
@echo on
%CYGWIN_DIR%\\bash --login -i -c "cd %WORKING_DIR%; gcc ./src/main.c ./src/circular_buffer.c -I./include/ -I.. -lpthread -ggdb -O0 -o ./dist/ProdConsTest.exe"

rem %CYGWIN_DIR%\\bash --login -i -c "cd %WORKING_DIR%/; make"