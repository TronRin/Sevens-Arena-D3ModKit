@call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
@cls

@ECHO OFF
@ECHO --------------------------------------------------------------------------------
@ECHO 			DOOM 3 MODDING KIT
@ECHO			   NINJA PROJECT GENERATION FOR x86
@ECHO				     [DEBUG]
@ECHO --------------------------------------------------------------------------------
pause
cd ..
mkdir buildlog > NUL 2>&1
cd buildlog
mkdir x86_ninja > NUL 2>&1
cd ..
del /s /q buildx86_ninja > NUL 2>&1
mkdir buildx86_ninja > NUL 2>&1
cd buildx86_ninja
@ECHO Generating x86 files
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Debug ../neo > ../buildlog/x86_ninja/log.txt
@ECHO Compiling project
ninja > ../buildlog/x86_ninja/build.txt
if ERRORLEVEL == 1 goto ERROR
pause
exit

:ERROR
@ECHO ERROR Generating x86 files
cd ..
rmdir /Q /S buildx86_ninja
pause
exit