local palette = require('editor.palette')
return {
    text = {
        fg = palette[2],
        shadow = palette[1]
    },
    window = {
        bg = palette[4],
        border = {
            light = palette[2],
            dark = palette[6]
        },
        titleBar = {
            bg = palette[16],
            border = {
                light = palette[2],
                dark = palette[9]
            }
        }
    },
    button = {
        normal = {
            bg = palette[4],
            border = {
                light = palette[2],
                dark = palette[5]
            }
        },
        hover = {
            bg = palette[3],
            border = {
                light = palette[2],
                dark = palette[5]
            }
        },
        press = {
            bg = palette[6],
            border = {
                light = palette[5],
                dark = palette[2]
            }
        },
        release = {

        }
    }
}