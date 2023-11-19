local Rect = {}












local rect_mt = {}
rect_mt.__index = Rect

function Rect.create(x, y, w, h)
   local r = {}

   r.x = x
   r.y = y
   r.w = w
   r.h = h

   return setmetatable(r, rect_mt)
end

function Rect:getValues()
   return self.x, self.y, self.w, self.h
end

return Rect
