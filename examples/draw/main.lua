local App = require('App')
local app = App.defaultEngine()
local Image = require('graphics.Image')
local img = Image.load(app.projectFs:resolve('sprites.png'))

--- @param r Renderer
function app:onRender(r)
  r:clearColor({ 0, 0, 0, 255 })
  r:clear()
  r:copy(img)
end

return app
