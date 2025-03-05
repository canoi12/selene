@echo "DIR: %CD%"
cd /d "%CD%\source\src"
call msvcbuild.bat;

cd "%CD%\..\.."

set "arch=x86"

if "%1"=="x64" (
    set "arch=x64"
)

echo "%arch%"

if not exist "%CD%\dist\%arch%-windows-msvc\v143\bin" mkdir "%CD%\dist\%arch%-windows-msvc\v143\bin"
if not exist "%CD%\dist\%arch%-windows-msvc\v143\lib" mkdir "%CD%\dist\%arch%-windows-msvc\v143\lib"
copy "%CD%\source\src\luajit.exe" "%CD%\dist\%arch%-windows-msvc\v143\bin\luajit.exe"
copy "%CD%\source\src\luajit.lib" "%CD%\dist\%arch%-windows-msvc\v143\lib\"
copy "%CD%\source\src\lua51.lib" "%CD%\dist\%arch%-windows-msvc\v143\lib\"
copy "%CD%\source\src\lua51.dll" "%CD%\dist\%arch%-windows-msvc\v143\bin\"
copy "%CD%\source\src\lua51.exp" "%CD%\dist\%arch%-windows-msvc\v143\lib\"