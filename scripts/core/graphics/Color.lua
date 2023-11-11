local _tl_compat; if (tonumber((_VERSION or ''):match('[%d.]*$')) or 0) < 5.3 then local p, m = pcall(require, 'compat53.module'); if p then _tl_compat = m end end; local ipairs = _tl_compat and _tl_compat.ipairs or ipairs; local table = _tl_compat and _tl_compat.table or table; local _tl_table_unpack = unpack or table.unpack; local Color = {}













function Color.rgb(r, g, b)
   local c = {}
   c[1] = r
   c[2] = g
   c[3] = b
   c[4] = 255
   return setmetatable(c, { __index = Color })
end

function Color.rgba(r, g, b, a)
   local c = {}
   c[1] = r
   c[2] = g
   c[3] = b
   c[4] = a
   return setmetatable(c, { __index = Color })
end

function Color:toFloat()
   local cf = {}
   for i, cc in ipairs(self) do
      cf[i] = cc / 255
   end
   return _tl_table_unpack(cf)
end

Color.red = Color.rgb(255, 0, 0)
Color.black = Color.rgb(0, 0, 0)
Color.white = Color.rgb(255, 255, 255)

return Color
