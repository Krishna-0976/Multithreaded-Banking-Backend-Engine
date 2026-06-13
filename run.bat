C:\msys64\ucrt64\bin\g++.exe -std=c++17 -Iinclude src/bank.cpp src/queue.cpp src/logger.cpp src/thread_pool.cpp src/main.cpp -o main.exe
if %errorlevel% equ 0 .\main.exe