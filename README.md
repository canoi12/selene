<p align="center">
<img src="selene_icon.png" width="256" alt="icon"/>
</p>

# selene

Selene is an executable with embedded Lua modules for game making programming (sdl2, opengl, linmath, json, asset loaders).

## Running

After build or download a release file, you can execute the file `selene`, or `selene.exe` on Windows.
Notice that on Windows you will also need to put the `SDL2.dll` in the same folder.

The executable searches for a `main.lua` file, and it uses the lua `require` to do this job, so it will search the main file following the `package.path` hierarchy.
From running a project that is on another directory, you can use the flag `-d` and provide the path (both absolute and relative works), `selene -d examples/sdl2`.

### main.lua

As I said, the entry file is a `main.lua`. The selene structure use two simple callbacks, `step` in the main loop and `quit` when closing. You can set them using `selene.set_step(function)` or `selene.set_quit(function)`.

```lua
-- main.lua
local i = 1
selene.set_step(function()
    print(i, 'aloooooooooooooooooooooo')
    i = i + 1
    if i > 50 then
        selene.set_running(false) -- do not forget to call this function to exit, otherwise the application will continue running the main loop
    end
end)
selene.set_quit(function()
    print('exiting')
end)
```

### SDL2 example

```lua
-- main.lua
if not sdl.init(sdl.INIT_VIDEO) then
    error('Failed to init SDL2 ' .. sdl.get_error())
end
local win = sdl.create_window('SDL2 example', sdl.WINDOWPOS_CENTERED, sdl.WINDOWPOS_CENTERED, 640, 380, sdl.WINDOW_SHOWN)
if not win then
    error("Failed to create SDL2 window " .. sdl.get_error())
end
local renderer = sdl.create_renderer(win)
local event = sdl.create_event()
selene.set_step(function()
    while event:poll() do
        local t = event:get_type()
        if t == sdl.QUIT then selene.set_running(false) end
        if t == sdl.WINDOWEVENT then
            if t == sdl.WINDOWEVENT_CLOSE then selene.set_running(false) end
        end
    end
    renderer:set_color(75, 125, 125, 255)
    renderer:clear()
    renderer:present()
end)
selene.set_quit(function()
    renderer:destroy()
    win:destroy()
    sdl.quit()
end)
```

## Building

First you need to install [premake5](https://premake.github.io/) on your system.

### Windows

For Windows, you will also need to download the [SDL2](https://libsdl.org/) package for development. Making that, simply run on a terminal:

```
premake5 vs2019 --sdl-dir={source to SDL folder}
```

It will generate the Visual Studio project files in the `build/` folder. After that, you can open the `build\selene.sln` solution in Visual Studio, or start the `Developer Command Prompt for VS20**` and run:

```
msbuild build\selene.sln -p:Configuration=[Release|Debug];Platform=[win32|win64]
```

### Linux

For linux the process is very similar. You will need to install the `SDL2` packages for development too, the package is present in the majority of the Linux distros.

In Debian you can install SDL2 with:

```
sudo apt install libsdl2-dev
```

And then run premake:

```
premake5 gmake2
```

It will generate the Makefiles in the `build/` folder, then just run:

```
make -C build/ config=[debug_linux,release_linux]
```

*Building for MinGW is a very similar process*

### Emscripten

You need the module [premake-emscripten](https://github.com/tritao/premake-emscripten) for premake5. Put in the same directory of the premake5 binary and rename the module folder to `emscripten`. After that, execute this in the selene source directory:

`premake5 gmake2 --emscripten`

And run the Makefile with:

`make -C build config=[release_emscripten|debug_emscripten]`

Notice that you will need to setup the EMSDK too, https://emscripten.org/.

#### Packaging for Emscripten

In the release distribution of selene for Emscripten, the index.html searches for a `game.js` file.
It can be generated using the file packaging tools of the EMSDK itself. I create a script for premake5 called `gen_gamejs.lua` that can help with that.
One you have the EMSDK variable properly setted on your system, it should work. You can use like this:

`premake5 build --file=gen_gamejs.lua --project-dir=[path to your project] --out-dir=build/bin/emscripten/Release/`

### Android

TODO
