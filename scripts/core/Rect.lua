local Rect = {}












function Rect.create(x, y, w, h)
   local r = {}

   r.x = x
   r.y = y
   r.w = w
   r.h = h

   return setmetatable(r, { __index = Rect })
end

function Rect:getValues()
   return self.x, self.y, self.w, self.h
end

return Rect
