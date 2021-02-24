
@echo off

Setlocal enableextensions enabledelayedexpansion

set /a total=0

for /R %%A in (*.cpp *.h *.hpp *c) do (

	ECHO.%%A | FIND /I "imgui">Nul && ( Echo.skipping %%A ) || (

	set /a compt=0

	for /f "tokens=*" %%I in ('type "%%A"') do (set /a compt+=1)

	echo %%~dpnxA : !compt!

	set /a total=!total!+!compt!
	)

)

echo. & echo Total : !total!

Endlocal

pause

exit