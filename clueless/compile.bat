@echo off
C:\msys64\ucrt64\bin\g++.exe -m64 src\commandline_checks.cpp src\env_checks.cpp src\filesystem_checks.cpp src\main.cpp src\process_checks.cpp src\registry_checks.cpp src\utils.cpp src\window_checks.cpp -o clueless.exe -static-libgcc -static-libstdc++ -Wall -O2 -lpsapi -ladvapi32 2>&1
clueless.exe