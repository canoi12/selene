local core = require('core').init()
local Color = require('core.graphics.Color')
local Image = require('core.graphics.Image')

local img = Image.load(core.projectFs:resolve('sprites.png'))

--- @param r core.Renderer
function core.onRender(r)
  r:setClearColor(Color.black)
  r:clear()
  r:blit(img)
end

return core.default()
