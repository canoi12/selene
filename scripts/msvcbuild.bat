set OUT_DIR=builds
set BUILD_TYPE=Release
set VS_GENERATOR="Visual Studio 17 2022"
set DIST_DIR=dist/windows
call :build_x64
call :build_x86
:: call :dist
if %1 == "dist" (
    call :dist
)
goto :eof

:build_x64
    cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -B build -G %VS_GENERATOR% -A x64
    cmake --build build --config %BUILD_TYPE%
    if not exist "%CD%\%OUT_DIR%\x64-windows-msvc\v143\bin" mkdir "%CD%\%OUT_DIR%\x64-windows-msvc\v143\bin"
    if not exist "%CD%\%OUT_DIR%\x64-windows-msvc\v143\lib" mkdir "%CD%\%OUT_DIR%\x64-windows-msvc\v143\lib"
    copy "%CD%\build\%BUILD_TYPE%\Windows\bin\%BUILD_TYPE%\*" "%CD%\%OUT_DIR%\x64-windows-msvc\v143\bin\"
    copy "%CD%\build\%BUILD_TYPE%\Windows\lib\%BUILD_TYPE%\*" "%CD%\%OUT_DIR%\x64-windows-msvc\v143\lib\"
    rmdir /s /q build
    goto :eof

:build_x86
    cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -B build -G %VS_GENERATOR% -A Win32
    cmake --build build --config %BUILD_TYPE%
    if not exist "%CD%\%OUT_DIR%\x86-windows-msvc\v143\bin" mkdir "%CD%\%OUT_DIR%\x86-windows-msvc\v143\bin"
    if not exist "%CD%\%OUT_DIR%\x86-windows-msvc\v143\lib" mkdir "%CD%\%OUT_DIR%\x86-windows-msvc\v143\lib"
    copy "%CD%\build\%BUILD_TYPE%\Windows\bin\%BUILD_TYPE%\*" "%CD%\%OUT_DIR%\x86-windows-msvc\v143\bin\"
    copy "%CD%\build\%BUILD_TYPE%\Windows\lib\%BUILD_TYPE%\*" "%CD%\%OUT_DIR%\x86-windows-msvc\v143\lib\"
    rmdir /s /q build
    goto :eof

:dist
    if not exist %DIST_DIR% (
        mkdir "%DIST_DIR%"
        mkdir "%DIST_DIR%/x86"
        mkdir "%DIST_DIR%/x64"
        copy "%CD%\%OUT_DIR%\x86-windows-msvc\v143\bin\selene.exe" "%DIST_DIR%/x86/selene.exe"
        copy "%CD%\%OUT_DIR%\x86-windows-msvc\v143\bin\SDL2.dll" "%DIST_DIR%/x86/SDL2.dll"
        copy "%CD%\%OUT_DIR%\x86-windows-msvc\v143\bin\lua5*" "%DIST_DIR%/x86/lua5*"
        
        copy "%CD%\%OUT_DIR%\x64-windows-msvc\v143\bin\selene.exe" "%DIST_DIR%/x64/selene.exe"
        copy "%CD%\%OUT_DIR%\x64-windows-msvc\v143\bin\SDL2.dll" "%DIST_DIR%/x64/SDL2.dll"
        copy "%CD%\%OUT_DIR%\x64-windows-msvc\v143\bin\lua5*" "%DIST_DIR%/x64/lua5*"
    )
    goto :eof