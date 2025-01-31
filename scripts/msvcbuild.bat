cmake -DCMAKE_BUILD_TYPE=Release -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
if not exist "%CD%\builds\x64-windows-msvc\v143\bin" mkdir "%CD%\builds\x64-windows-msvc\v143\bin"
if not exist "%CD%\builds\x64-windows-msvc\v143\lib" mkdir "%CD%\builds\x64-windows-msvc\v143\lib"
copy "%CD%\build\Release\Windows\bin\Release\*" "%CD%\builds\x64-windows-msvc\v143\bin\"
copy "%CD%\build\Release\Windows\lib\Release\*" "%CD%\builds\x64-windows-msvc\v143\lib\"
rmdir /s /q build

cmake -DCMAKE_BUILD_TYPE=Release -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Release
if not exist "%CD%\builds\x86-windows-msvc\v143\bin" mkdir "%CD%\builds\x86-windows-msvc\v143\bin"
if not exist "%CD%\builds\x86-windows-msvc\v143\lib" mkdir "%CD%\builds\x86-windows-msvc\v143\lib"
copy "%CD%\build\Release\Windows\bin\Release\*" "%CD%\builds\x86-windows-msvc\v143\bin\"
copy "%CD%\build\Release\Windows\lib\Release\*" "%CD%\builds\x86-windows-msvc\v143\lib\"
rmdir /s /q build