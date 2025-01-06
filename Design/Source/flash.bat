@echo off
set binary_name=%1

openocd.exe -d2 -f board/esp32s3-builtin.cfg -c "program_esp build/bootloader/bootloader.bin 0x0000 verify exit"
if %ERRORLEVEL% NEQ 0 goto error

openocd.exe -d2 -f board/esp32s3-builtin.cfg -c "program_esp build/partition_table/partition-table.bin 0x8000 verify exit"
if %ERRORLEVEL% NEQ 0 goto error

openocd.exe -d2 -f board/esp32s3-builtin.cfg -c "program_esp build/%binary_name% 0x10000 verify reset exit"
if %ERRORLEVEL% NEQ 0 goto error

echo:
echo [32mPROGRAMMING SUCCESSFUL!!![0m
exit /B 0

:error
echo:
echo [31mPROGRAMMING FAILED!!![0m
exit /B 1