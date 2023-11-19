--- @meta

--- @class selene.font.Glyph
--- @field ax number
--- @field ay number
--- @field bl number
--- @field bt number
--- @field bw number
--- @field bh number
--- @field tx number
local Glyph = {}

--- @class selene.font
local font = {}

--- Get default bitmap font
--- @return selene.Data, integer, integer, selene.font.Glyph
function font.getDefault() end

--- Load TTF from file
--- @return selene.Data, integer, integer, selene.font.Glyph
function font.loadTTF(path, size) end