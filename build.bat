@echo off
echo ========================================================
echo  Compilando Vision Compensator (C++ Win32 GDI+ Nativo)
echo ========================================================

g++ -O3 -std=c++17 -municode -mwindows main.cpp d3d_overlay.cpp -o VisionCompensator.exe -ld3d11 -ldxgi -lgdiplus -lgdi32 -luser32 -lcomctl32 -lcomdlg32

if %ERRORLEVEL% equ 0 (
    echo.
    echo [EXITO] VisionCompensator.exe generado correctamente!
    echo.
) else (
    echo.
    echo [ERROR] Ocurrio un error en la compilacion.
    echo.
)
