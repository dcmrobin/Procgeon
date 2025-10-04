@echo off
REM === Change this path to where your ELF file is built ===
set ELF=C:\Users\matsk\AppData\Local\arduino\sketches\A577313DF34CC8EFBD9CA7E8DD64B8F5\Main.ino.elf

REM === Change this to your toolchain path if needed ===
set TOOLCHAIN=C:\Users\matsk\AppData\Local\Arduino15\packages\teensy\tools\teensy-compile\11.3.1\arm\bin

:loop
set /p ADDR=Enter address (or 'q' to quit): 
if "%ADDR%"=="q" goto :eof

"%TOOLCHAIN%\arm-none-eabi-addr2line.exe" -e "%ELF%" -f -C %ADDR%
echo.
goto loop
