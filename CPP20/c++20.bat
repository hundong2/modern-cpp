cls

@echo [101mg++ %* -std=c++2a -o gout.exe [0m
@g++ %* -std=c++2a -o gout.exe

@IF %ERRORLEVEL% EQU 0 (
	echo [33;4m[ g++ compiling success !! run gout.exe ][0m
  	gout.exe
) ELSE (
	echo [33;4m[ Error !!, g++ fail to compile. ][0m
)
