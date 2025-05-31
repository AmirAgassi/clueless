@echo off
g++ src\*.cpp -o clueless.exe -static-libgcc -static-libstdc++ -Wall -O2 -lpsapi -ladvapi32