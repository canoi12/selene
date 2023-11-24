--- @type Rect
local Rect = require('core.Rect')

--- @class Drawable
--- @field width integer
--- @field height integer
local Drawable = {}
--- @return selene.gl.Texture
function Drawable:getTexture() end
--- @param rect Rect
--- @return table
function Drawable:getUV(rect) end
--- @return integer
function Drawable:getWidth() return self.width end
--- @return integer
function Drawable:getHeight() return self.height end

return Drawable