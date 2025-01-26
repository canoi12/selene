cmake -DCMAKE_BUILD_TYPE=Release -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
if not exist "%CD%\dist\x64-windows-msvc\v143\bin" mkdir "%CD%\dist\x64-windows-msvc\v143\bin"
if not exist "%CD%\dist\x64-windows-msvc\v143\lib" mkdir "%CD%\dist\x64-windows-msvc\v143\lib"
copy "%CD%\build\Release\lua.exe" "%CD%\dist\x64-windows-msvc\v143\bin\lua.exe"
copy "%CD%\build\Release\luac.exe" "%CD%\dist\x64-windows-msvc\v143\bin\luac.exe"
copy "%CD%\build\Release\*.lib" "%CD%\dist\x64-windows-msvc\v143\lib\"
copy "%CD%\build\Release\*.dll" "%CD%\dist\x64-windows-msvc\v143\lib\"

rmdir /s /q build

cmake -DCMAKE_BUILD_TYPE=Release -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Release
if not exist "%CD%\dist\x86-windows-msvc\v143\bin" mkdir "%CD%\dist\x86-windows-msvc\v143\bin"
if not exist "%CD%\dist\x86-windows-msvc\v143\lib" mkdir "%CD%\dist\x86-windows-msvc\v143\lib"
copy "%CD%\build\Release\lua.exe" "%CD%\dist\x86-windows-msvc\v143\bin\lua.exe"
copy "%CD%\build\Release\luac.exe" "%CD%\dist\x86-windows-msvc\v143\bin\luac.exe"
copy "%CD%\build\Release\*.lib" "%CD%\dist\x86-windows-msvc\v143\lib\"
copy "%CD%\build\Release\*.dll" "%CD%\dist\x86-windows-msvc\v143\lib\"

rmdir /s /q build