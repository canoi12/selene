--- @meta
--- @class font
font = {}

--- @class font.Glyph : userdata
local FontGlyph = {}

--- @alias font.Data {data:selene.Data,width:integer,height:integer,channels:integer,format:integer,glyphs:font.Glyph}

--- @return font.Data
function font.create8x8() end

--- @param filename string
--- @return font.Data
function font.from_ttf(filename) end