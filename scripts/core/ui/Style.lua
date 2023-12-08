--- @alias ui.Palette Color[]

--- @class ui.FrameStyle
--- @field bg Color
--- @field borders Color[]
local FrameStyle = {}

--- @class ui.WindowStyle : ui.FrameStyle
--- @field titleBar ui.FrameStyle
local WindowStyle = {}

--- @alias ui.ButtonStyle {normal:ui.FrameStyle, hover:ui.FrameStyle, press: ui.FrameStyle, release: ui.FrameStyle}

--- @alias ui.TextStyle {fg:Color,shadow:Color}

--- @class ui.Style
--- @field text ui.TextStyle
--- @field window ui.WindowStyle
--- @field button ui.ButtonStyle
local Style = {}

--- @param palette ui.Palette
function Style.create(palette)
    --- @type ui.Style
    local style = {
        text = {
            fg = palette[2],
            shadow = palette[1]
        },
        window = {
            bg = palette[4],
            borders = {palette[2], palette[6]},
            titleBar = {
                bg = palette[16],
                borders = {palette[2], palette[9]},
            }
        },
        button = {
            normal = {
                bg = palette[4],
                borders = {palette[2], palette[5]}
            },
            hover = {
                bg = palette[3],
                borders = {palette[2], palette[5]}
            },
            press = {
                bg = palette[6],
                borders = {palette[5], palette[2]}
            }
        }
    }
    return style
end

return Style