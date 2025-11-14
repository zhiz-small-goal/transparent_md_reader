@echo off
chcp 65001 >nul
cls
echo ========================================
echo   Transparent Markdown Reader - Build & Run
echo ========================================
echo.

REM Kill running process if exists
taskkill /IM md_reader.exe /F >nul 2>&1
timeout /t 1 /nobreak >nul

REM Compile SQLite (C)
echo [1/4] Compile SQLite (sqlite3.c)...
gcc -c sqlite3.c -o sqlite3.o -O2 -DSQLITE_THREADSAFE=1 -DSQLITE_OMIT_LOAD_EXTENSION -w
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Build sqlite3.c failed!
    pause
    exit /b 1
)

REM Compile main app (C++)
echo [2/4] Compile main.cpp...
g++ -c main.cpp -o main.o -O2 -w
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Build main.cpp failed!
    del sqlite3.o >nul 2>&1
    pause
    exit /b 1
)

REM Link executable
echo [3/4] Link md_reader.exe...
g++ main.o sqlite3.o -o md_reader.exe -lgdi32 -luser32 -lshell32 -lcomdlg32 -lcomctl32 -lgdiplus -mwindows
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Link failed!
    del main.o sqlite3.o >nul 2>&1
    pause
    exit /b 1
)

REM Clean objects
del main.o sqlite3.o >nul 2>&1

echo.
echo [4/4] Build success. Launching...
echo.
start md_reader.exe

echo Done. You can close this window after app starts.
pause

