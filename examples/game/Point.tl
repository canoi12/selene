local record Point
      x: number
      y: number
end

Point.New = function(x, y) : Point
      local point = {}
      point.x = x or 0
      point.y = y or 0
      return point
end

return Point
