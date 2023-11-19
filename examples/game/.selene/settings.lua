return {
    name = "Game",
    version = "0.1.0",
    audio = {
        sampleRate = 44100,
        channels = 2,
        samples = 4096
    },
    window = {
        width = 640,
        height = 380,
        title = "${project.name}",
        resizable = false,
        borderless = false,
        fullscreen = false,
        alwaysOnTop = false
    },
}