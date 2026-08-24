@echo off
echo ========================================================
echo  Compilando Vision Compensator (C++ Win32 GDI+ Nativo)
echo ========================================================

g++ -O3 -municode -mwindows main.cpp -o VisionCompensator.exe -lgdiplus -lgdi32 -luser32 -lcomctl32 -lcomdlg32

if %ERRORLEVEL% equ 0 (
    echo.
    echo [EXITO] VisionCompensator.exe generado correctamente!
    echo.
) else (
    echo.
    echo [ERROR] Ocurrio un error en la compilacion.
    echo.
)
