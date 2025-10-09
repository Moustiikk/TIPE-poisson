@echo off
setlocal enabledelayedexpansion

REM ================================================================
REM === CONFIGURATION DU CHEMIN SDL3 (SDK officiel MinGW 64-bit) ===
REM ================================================================
set SDL_PATH=C:\libs\SDL3

REM === Dossiers sources ===
set DEPS_DIR=dependencies
set BUILD_DIR=build

REM === Préparation du dossier de build ===
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM === Options de compilation ===
set CFLAGS=-Wall -Wextra -O2 -g -I"%DEPS_DIR%" -I"%SDL_PATH%\include"
set LDFLAGS=-L"%SDL_PATH%\lib" -lSDL3

echo ===== Compilation des fichiers de %DEPS_DIR% =====
set OBJS=
for %%f in ("%DEPS_DIR%\*.c") do (
    echo [CC] %%~nxf
    gcc -c "%%f" %CFLAGS% -o "%BUILD_DIR%\%%~nf.o"
    if errorlevel 1 (
        echo.
        echo [ERREUR] Echec de la compilation de %%~nxf
        pause
        exit /b 1
    )
    set OBJS=!OBJS! "%BUILD_DIR%\%%~nf.o"
)

echo ===== Compilation de main.c =====
gcc -c "main.c" %CFLAGS% -o "%BUILD_DIR%\main.o"
if errorlevel 1 (
    echo.
    echo [ERREUR] Echec de la compilation de main.c
    pause
    exit /b 1
)

echo ===== Edition des liens =====
gcc "%BUILD_DIR%\main.o" %OBJS% %LDFLAGS% -o "fish_sim.exe"
if errorlevel 1 (
    echo.
    echo [ERREUR] Echec de l’édition des liens
    pause
    exit /b 1
)

REM === Copie de la bonne DLL officielle ===
copy /Y "%SDL_PATH%\bin\SDL3.dll" . >nul

REM === Facultatif: runtimes MinGW si ton exe plante sans eux ===
REM set MSYS2=C:\msys64
REM copy /Y "%MSYS2%\mingw64\bin\libgcc_s_seh-1.dll" . >nul
REM copy /Y "%MSYS2%\mingw64\bin\libstdc++-6.dll" . >nul
REM copy /Y "%MSYS2%\mingw64\bin\libwinpthread-1.dll" . >nul

echo.
echo [OK] Compilation et linkage terminés !
echo.
pause
exit /b 0
