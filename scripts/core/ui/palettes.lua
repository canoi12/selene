local Color = require('graphics.Color')
--- @alias ui.DefaultPalette
---| 'pastel-pop16'

--- @type {ui.DefaultPalette:ui.Palette}
local palettes = {}
palettes['pastel-pop16'] = {
    Color.hex("#000000"),
    Color.hex("#fffcff"),
    Color.hex("#e6e1f2"),
    Color.hex("#b2b7e1"),

    Color.hex("#595b7d"),
    Color.hex("#7b8ac6"),
    Color.hex("#ffd9f4"),
    Color.hex("#e1adc3"),

    Color.hex("#ad80a6"),
    Color.hex("#fba2d7"),
    Color.hex("#fae0c7"),
    Color.hex("#f0abab"),
    
    Color.hex("#97c4aa"),
    Color.hex("#bfedf5"),
    Color.hex("#73c9eb"),
    Color.hex("#caaff5"),
}
return palettes
