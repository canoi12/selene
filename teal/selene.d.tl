global record pointer
    userdata
end

local record Data
    userdata
    create: function(size: integer) : Data

    realloc: function(Data, size: integer)
    free: function(Data)
    getSize: function(Data) : integer
    getPointer: function(Data) : pointer
    getString: function(Data) : string

    readBytes: function(Data, offset: integer, count: integer | nil) : integer...
    readInts: function(Data, offset: integer, count: integer | nil) : integer...
    readFloats: function(Data, offset: integer, count: integer | nil) : number...

    writeBytes: function(Data, offset: integer, ...: integer) : integer
    writeInts: function(Data, offset: integer, ...: integer) : integer
    writeFloats: function(Data, offset: integer, ...: number) : integer
    writeString: function(Data, offset: integer, value: string) : integer
end

local record audio
    record Decoder
        userdata
        load: function(path: string) : Decoder

        close: function(Decoder)
        clone: function(Decoder) : Decoder
        seek: function(Decoder, index: integer)
        readS16: function(Decoder, out: Data, len: integer | nil) : integer
        readF32: function(Decoder, out: Data, len: integer | nil) : integer
        getChunk: function(Decoder, out: Data, len: integer | nil) : integer
    
        getSampleRate: function(Decoder) : integer
        getChannels: function(Decoder) : integer
        getBitDepth: function(Decoder) : integer
        getFrameCount: function(Decoder) : integer
    end
end

local record font
    record Glyph
        ax: number
        ay: number

        bl: number
        bt: number
        bw: number
        bh: number

        tx: number
    end
    getDefault: function() : Data, integer, integer, {Glyph}
    loadTTF: function(path: string, size: integer) : Data, integer, integer, {Glyph}
end

local record fs
    record File
        userdata
        open: function(path: string) : File

        seek: function(File, n: integer)
        tell: function(File) : integer
        read: function(File, data_or_size: Data | integer) : Data
        write: function(File, path: string)
        append: function(File, path: string)
        getSize: function(File) : integer
        close: function(File)
    end

    exists: function(path: string) : boolean
    read: function(path: string) : Data
    write: function(path: string, text: string) : boolean
    createDir: function(path: string)
    removeDir: function(path: string)
    load: function(path: string) : any
end

local record gl
    record Buffer
        userdata
        create: function() : Buffer
        bind: function(target: integer, buffer: Buffer | nil)
        data: function(target: integer, size: integer, usage: integer, data: Data | nil)
        subData: function(target: integer, start: integer, size: integer, data: Data)
    
        destroy: function(Buffer)
    end

    record Framebuffer
        userdata
        create: function() : Framebuffer
        bind: function(target: integer, buffer: Framebuffer | nil)
        attachTexture2D: function(target: integer, attachment: integer, tex_target: integer, texture: Texture)
    
        destroy: function(Framebuffer)
    end

    record Program
        userdata
        create: function() : Program
        use: function(prog: Program | nil)

        destroy: function(Program)
        attachShader: function(Program, ...: Shader)
        link: function(Program)
        getAttribLocation: function(Program, string) : integer
        getUniformLocation: function(Program, string) : integer
    end

    record Shader
        userdata
        create: function(integer) : Shader

        destroy: function(Shader)
        source: function(Shader, source: string)
        compile: function(Shader)
    end

    record Texture
        userdata
        create: function() : Texture
        bind: function(target: integer, texture: Texture)
        image2D: function(target: integer, internal: integer, width: integer, height: integer, format: integer, type_: integer, data: Data | nil)
        subImage2D: function(target: integer, xoffset: integer, yoffset: integer, width: integer, height: integer, format: integer, type_: integer, data: Data | nil)
        parameteri: function(target: integer, pname: integer, param: integer)

        destroy: function(Texture)
    end

    record VertexArray
        userdata
        create: function() : VertexArray

        unbind: function()
        enable: function(attrib: integer)
        disable: function(attrib: integer)
        attribPointer: function(attrib: integer, size: integer, type_: integer, normalized: boolean, stride: integer, ptr: integer)
    
        bind: function(VertexArray)
        destroy: function(VertexArray)
    end

    loadGlad: function(proc_fn: pointer)
    viewport: function(x: integer, y: integer, w: integer, h: integer)
    clearDepth: function(depth: number | nil)
    clearColor: function(...: number)
    clear: function(...: integer)
    enable: function(...: integer)
    disable: function(...: integer)
    scissor: function(x: integer, y: integer, w: integer, h: integer)
    blendFunc: function(source: integer | nil, dest: integer | nil)
    drawArrays: function(mode: integer, start: integer, count: integer)
    drawElements: function(mode: integer, start: integer, counter: integer, type_: integer)

    uniform1fv: function(location: integer, ...: number)
    uniform2fv: function(location: integer, ...: {number, number})
    uniform3fv: function(location: integer, ...: {number, number, number})
    uniform4fv: function(location: integer, ...: {number, number, number, number})
    uniformMatrix4fv: function(location: integer, count: integer, normalize: boolean, linmath.Mat4)

    -- Clear
    COLOR_BUFFER_BIT: integer
    DEPTH_BUFFER_BIT: integer
    STENCIL_BUFFER_BIT: integer
    -- Types
    BYTE: integer
    UNSIGNED_BYTE: integer
    SHORT: integer
    UNSIGNED_SHORT: integer
    INT: integer
    UNSIGNED_INT: integer
    FLOAT: integer

    DOUBLE: integer

    -- Enable
    DEPTH_TEST: integer
    STENCIL_TEST: integer
    SCISSOR_TEST: integer
    BLEND: integer
    CULL_FACE: integer
    -- Pixel

    RED: integer
    RG: integer
    BGR: integer
    BGRA: integer

    RGB: integer
    RGBA: integer
    DEPTH_COMPONENT: integer
    DEPTH_COMPONENT16: integer

    DEPTH_COMPONENT24: integer
    DEPTH_COMPONENT32: integer
    DEPTH_COMPONENT32F: integer
    DEPTH32F_STENCIL8: integer
    DEPTH24_STENCIL8: integer

    -- Funcs
    ZERO: integer
    ONE: integer
    SRC_COLOR: integer
    ONE_MINUS_SRC_COLOR: integer
    SRC_ALPHA: integer
    ONE_MINUS_SRC_ALPHA: integer
    DST_ALPHA: integer
    ONE_MINUS_DST_ALPHA: integer
    DST_COLOR: integer
    ONE_MINUS_DST_COLOR: integer
    SRC_ALPHA_SATURATE: integer
    CONSTANT_COLOR: integer
    ONE_MINUS_CONSTANT_COLOR: integer
    CONSTANT_ALPHA: integer
    ONE_MINUS_CONSTANT_ALPHA: integer
    -- Draw mode
    POINTS: integer
    LINES: integer
    TRIANGLES: integer
    -- Texture targers
    TEXTURE_2D: integer
    TEXTURE_CUBE_MAP: integer
    TEXTURE_CUBE_MAP_NEGATIVE_X: integer
    TEXTURE_CUBE_MAP_POSITIVE_X: integer
    TEXTURE_CUBE_MAP_NEGATIVE_Y: integer
    TEXTURE_CUBE_MAP_POSITIVE_Y: integer
    TEXTURE_CUBE_MAP_NEGATIVE_Z: integer
    TEXTURE_CUBE_MAP_POSITIVE_Z: integer
    -- Texture params
    TEXTURE_MIN_FILTER: integer
    TEXTURE_MAG_FILTER: integer
    TEXTURE_WRAP_S: integer
    TEXTURE_WRAP_T: integer
    NEAREST: integer
    LINEAR: integer
    REPEAT: integer
    CLAMP_TO_EDGE: integer

    CLAMP_TO_BORDER: integer

    -- Framebuffer targets
    FRAMEBUFFER: integer

    DRAW_FRAMEBUFFER: integer
    READ_FRAMEBUFFER: integer
    DEPTH_STENCIL_ATTACHMENT: integer

    COLOR_ATTACHMENT0: integer
    DEPTH_ATTACHMENT: integer
    STENCIL_ATTACHMENT: integer
    -- Buffer
    ARRAY_BUFFER: integer
    ELEMENT_ARRAY_BUFFER: integer

    UNIFORM_BUFFER: integer
    DYNAMIC_READ: integer
    DYNAMIC_COPY: integer
    STATIC_READ: integer
    STATIC_COPY: integer
    STREAM_READ: integer
    STREAM_COPY: integer

    DYNAMIC_DRAW: integer
    STATIC_DRAW: integer
    STREAM_DRAW: integer
    -- Shader
    FRAGMENT_SHADER: integer
    VERTEX_SHADER: integer

    GEOMETRY_SHADER: integer
end

local record image
    loadData: function(path: string, req_channels: integer) : Data, integer, integer, integer
end

local record linmath
    record Mat4
        userdata
        create: function() : Mat4

        identity: function(Mat4)
        translate: function(Mat4, x: number, y: number, z: number | nil)
        scale: function(Mat4, x: number, y: number, z: number | nil)
        rotateZ: function(Mat4, angle: number)

        ortho: function(Mat4, left: number, right: number, bottom: number, top: number, near: number, far: number)
        frustum: function(Mat4, left: number, right: number, bottom: number, top: number, near: number, far: number)
    end
end

local record sdl2
    record AudioDeviceID
        userdata
        open: function(dev_name: string, is_capture: boolean, table) : AudioDeviceID, table
        getCount: function(is_capture: boolean) : integer
        getName: function(dev_id: integer, is_capture: boolean) : string
        
        pause: function(AudioDeviceID, boolean)
        close: function(AudioDeviceID)
    end
    record AudioStream
        create: function(integer, integer, integer, integer, integer, integer) : AudioStream

        clear: function(AudioStream)
        free: function(AudioStream)
        flush: function(AudioStream)

        bind: function(AudioStream, device: AudioDeviceID)
        unbind: function(AudioStream, device: AudioDeviceID)
        put: function(AudioStream, data: string | Data, size: integer) : integer
        get: function(AudioStream, out: Data, size: integer | nil) : integer
        available: function(AudioStream) : integer
    end
    record Event
        userdata
        create: function() : Event
        
        getType: function(Event) : integer
        poll: function(Event) : boolean

        windowEvent: function(Event) : integer, integer, integer, integer
        keyboardEvent: function(Event) : string, boolean
        mouseMotionEvent: function(Event) : integer, integer, number, number, number, number
        mouseButtonEvent: function(Event) : integer, integer, number, number, integer
        audioDeviceEvent: function(Event) : integer
    end
    record Gamepad
        userdata
        open: function(index: integer) : Gamepad
        isValid: function(integer) : boolean

        getAxisFromString: function(string) : integer
        getButtonFromString: function(string) : integer

        getName: function(Gamepad) : string
        getVendor: function(Gamepad) : integer
        getProduct: function(Gamepad) : integer
        getProductVersion: function(Gamepad) : integer

        getAxis: function(Gamepad, axis: integer) : number
        getButton: function(Gamepad, button: integer) : boolean
        rumble: function(Gamepad, low: integer, high: integer, freq: integer | nil) : number
        currentPowerlevel: function(Gamepad)

        close: function(Gamepad)
    end
    record GLContext
        userdata
        create: function(Window) : GLContext

        destroy: function(GLContext)
    end
    record Joystick
        userdata
        open: function(index: integer) : Joystick
        count: function() : integer

        getName: function(Joystick) : string
        getVendor: function(Joystick) : integer
        getProduct: function(Joystick) : integer
        getProductVersion: function(Joystick) : integer

        getType: function(Joystick) : integer
        getTypeString: function(Joystick) : string

        numAxes: function(Joystick) : integer
        numBalls: function(Joystick) : integer
        numHats: function(Joystick) : integer
        numButtons: function(Joystick) : integer

        getAxis: function(Joystick, axis: integer) : integer
        getBall: function(Joystick, ball: integer) : integer, integer
        getHat: function(Joystick, hat: integer) : integer
        getButton: function(Joystick, button: integer) : boolean

        rumble: function(Joystick, low: integer, high: integer, freq: integer | nil) : boolean

        currentPowerLevel: function(Joystick) : integer

        close: function(Joystick)
    end
    record Window
        userdata
        create: function(title: string, x: integer, y: integer, width: integer, height: integer, flags: table) : Window

        destroy: function(Window)
        getSize: function(Window) : integer, integer
        getPosition: function(Window) : integer, integer
        setPosition: function(Window, integer, integer)

        swap: function(Window)

        setBordered: function(Window)

        maximize: function(Window)
        minimize: function(Window)
        restore: function(Window)

        showSimpleMessageBox: function()
    end

    init: function(...: integer) : boolean
    quit: function()

    -- OpenGL
    glMakeCurrent: function(Window, GLContext)
    glGetProcAddress: function() : pointer
    glSetAttribute: function(attr: integer, value: integer)
    glSetSwapInterval: function(boolean) : boolean
    getGetSwapInterval: function() : boolean

    -- Clipboard
    getClipboardText: function() : string
    hasClipboardText: function() : boolean
    setClipboardText: function(string)

    -- Filesystem
    getBasePath: function() : string
    getPrefPath: function(string, string) : string

    -- Shared object
    loadObject: function(string) : pointer
    unloadObject: function(pointer)
    loadFunction: function(pointer, string) : pointer

    -- Keyboard
    checkKeyState: function(integer) : boolean
    getKeyboardState: function() : pointer
    hasScreenKeyboardSupport: function() : boolean
    isScreenKeyboardShown: function(Window) : boolean
    getScancodeFromName: function(string) : integer
    getScancodeName: function(integer) : string

    -- Mouse
    getMousePosition: function() : number, number
    getRelativeMousePosition: function() : number, number
    isMouseDown: function(integer) : boolean

    -- Timer
    getTicks: function() : integer
    delay: function(number)
    getPerformanceCounter: function() : integer
    getPerformanceFrequency: function() : integer

    -- Error
    getError: function() : string
    setError: function(string)

    -- System
    getPlatform: function() : string
    getCPUCacheLineSize: function() : integer
    getCPUCount: function() : integer
    getSystemRAM: function() : integer

    -- Enums
    INIT_EVERYTHING: integer
    INIT_SENSOR: integer
    INIT_AUDIO: integer
    INIT_VIDEO: integer
    INIT_JOYSTICK: integer
    INIT_GAMECONTROLLER: integer
    INIT_TIMER: integer
    INIT_EVENTS: integer
    -- Audio
    AUDIO_S16: integer
    AUDIO_S16SYS: integer
    AUDIO_F32: integer
    -- Application events
    QUIT: integer
    APP_TERMINATING: integer
    APP_LOWMEMORY: integer
    APP_WILLENTERBACKGROUND: integer
    APP_DIDENTERBACKGROUND: integer
    APP_WILLENTERFOREGROUND: integer
    APP_DIDENTERFOREGROUND: integer
    LOCALECHANGED: integer

    -- Display events
    DISPLAYEVENT: integer

    -- Window events
    WINDOWEVENT: integer

    -- Keyboard events
    KEYDOWN: integer
    KEYUP: integer
    TEXTEDITING: integer
    TEXTINPUT: integer
    KEYMAPCHANGED: integer
    TEXTEDITING_EXT: integer

    -- Mouse events
    MOUSEMOTION: integer
    MOUSEBUTTONDOWN: integer
    MOUSEBUTTONUP: integer
    MOUSEWHEEL: integer

    -- Joystick events
    JOYAXISMOTION: integer
    JOYBALLMOTION: integer
    JOYHATMOTION: integer
    JOYBUTTONDOWN: integer
    JOYBUTTONUP: integer
    JOYDEVICEADDED: integer
    JOYDEVICEREMOVED: integer
    JOYBATTERYUPDATED: integer

    -- Game controller events
    CONTROLLERAXISMOTION: integer
    CONTROLLERBUTTONDOWN: integer
    CONTROLLERBUTTONUP: integer
    CONTROLLERDEVICEADDED: integer
    CONTROLLERDEVICEREMOVED: integer
    CONTROLLERDEVICEREMAPPED: integer
    CONTROLLERTOUCHPADDOWN: integer
    CONTROLLERTOUCHPADMOTION: integer
    CONTROLLERTOUCHPADUP: integer
    CONTROLLERSENSORUPDATE: integer

    -- Touch events
    FINGERDOWN: integer
    FINGERUP: integer
    FINGERMOTION: integer

    -- Gesture events
    DOLLARGESTURE: integer
    DOLLARRECORD: integer
    MULTIGESTURE: integer

    -- Clipboard events
    CLIPBOARDUPDATE: integer

    -- Drag and drop events
    DROPFILE: integer
    DROPTEXT: integer
    DROPBEGIN: integer
    DROPCOMPLETE: integer

    -- Audio hotplug events
    AUDIODEVICEADDED: integer
    AUDIODEVICEREMOVED: integer

    -- Sensor events
    SENSORUPDATE: integer

    -- Render events
    RENDER_TARGETS_RESET: integer
    RENDER_DEVICE_RESET: integer

    -- Window Events
    WINDOWEVENT_CLOSE: integer
    WINDOWEVENT_SIZE_CHANGED: integer
    WINDOWEVENT_RESIZED: integer
    WINDOWEVENT_MOVED: integer
    WINDOWEVENT_MINIMIZED: integer
    WINDOWEVENT_MAXIMIZED: integer
    WINDOWEVENT_RESTORED: integer
    WINDOWEVENT_SHOWN: integer
    WINDOWEVENT_HIDDEN: integer
    WINDOWEVENT_ENTER: integer
    WINDOWEVENT_LEAVE: integer
    WINDOWEVENT_FOCUS_GAINED: integer
    WINDOWEVENT_FOCUS_LOST: integer
    WINDOWEVENT_TAKE_FOCUS: integer
    WINDOWEVENT_HIT_TEST: integer

    -- Window Flags
    WINDOW_SHOWN: integer
    WINDOW_OPENGL: integer
    WINDOW_RESIZABLE: integer
    WINDOW_FULLSCREEN: integer
    WINDOW_FULLSCREEN_DESKTOP: integer
    WINDOW_BORDERLESS: integer
    WINDOW_ALWAYS_ON_TOP: integer

    WINDOWPOS_CENTERED: integer
    -- Message Box
    MESSAGEBOX_ERROR: integer
    MESSAGEBOX_WARNING: integer
    MESSAGEBOX_INFORMATION: integer
    -- GL
    GL_DOUBLEBUFFER: integer
    GL_DEPTH_SIZE: integer
    GL_STENCIL_SIZE: integer
    GL_CONTEXT_PROFILE_MASK: integer
    GL_CONTEXT_MAJOR_VERSION: integer
    GL_CONTEXT_MINOR_VERSION: integer
    GL_CONTEXT_PROFILE_CORE: integer
    GL_CONTEXT_PROFILE_COMPATIBILITY: integer
    GL_CONTEXT_PROFILE_ES: integer
end

local record system
    getOS: function() : string
    getArch: function() : string
end

global record selene
    Data: Data

    audio: audio
    font: font
    fs: fs
    gl: gl
    image: image
    linmath: linmath
    sdl2: sdl2
    system: system

    args: {string}

    getVersion: function() : string
    setRunning: function(boolean)
    isRunning: function() : boolean
    UTF8Codepoint: function(string, integer) : integer, integer
end