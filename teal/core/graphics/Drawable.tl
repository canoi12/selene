local Rect = require('core.Rect')
local record Drawable
    getTexture: function(Drawable) : selene.gl.Texture
    getUV: function(Drawable, Rect) : {number}
    getWidth: function(Drawable) : number
    getHeight: function(Drawable) : number
end
return Drawable