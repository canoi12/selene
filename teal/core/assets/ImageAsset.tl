local sdl = selene.sdl2
local gl = selene.gl
local Image = require('graphics.Image')
local record ImageAsset
    name: string
    path: string

    filters: {integer}
    wraps: {string}

    getName: function(ImageAsset) : string
    getPath: function(ImageAsset) : string
    load: function(ImageAsset) : Image
end

return ImageAsset