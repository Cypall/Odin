@echo off
echo Starting Odin Server Monitor At %time% %date%
echo.

echo Loading Account Server...
start "Account Server" /min /abovenormal Odin-Account.exe

echo Loading Character Server...
start "Character Server" /min /abovenormal Odin-Character.exe

:ZoneLoad
echo Loading Zone Server...
start /wait /high Odin-Zone.exe

echo Zone Server close At %time% %date%
echo.
goto ZoneLoad
exit
