@call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
@cls

@ECHO OFF
@ECHO --------------------------------------------------------------------------------
@ECHO 			DOOM 3 MODDING KIT
@ECHO			   NINJA PROJECT GENERATION FOR x64
@ECHO				     [RELEASE]
@ECHO --------------------------------------------------------------------------------
pause

cd ..
mkdir buildlog > NUL 2>&1
cd buildlog
mkdir x64_ninja > NUL 2>&1
cd ..
del /s /q buildx64_ninja > NUL 2>&1
mkdir buildx64_ninja > NUL 2>&1
cd buildx64_ninja
@ECHO Generating x64 files
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ../neo > ../buildlog/x64_ninja/log.txt
@ECHO Compiling project
ninja > ../buildlog/x64_ninja/build.txt
if ERRORLEVEL == 1 goto ERROR
pause
exit

:ERROR
@ECHO ERROR Generating x64 files
cd ..
rmdir /Q /S buildx64_ninja
pause
exit