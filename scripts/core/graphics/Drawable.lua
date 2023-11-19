--- @type Rect
local Rect = require('core.Rect')

--- @class Drawable
local Drawable = {}
function Drawable:getTexture() end
function Drawable:getUV(rect) end
function Drawable:getWidth() end
function Drawable:getHeight(Drawable) end

return Drawable