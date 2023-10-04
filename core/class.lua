local class = {}
class.__class = "Class"

function class:extend(name)
  local o = setmetatable({}, { __index = self })
  o.__class = name
  return o
end

function class:new(...)
  local o = setmetatable({}, { __index = self })
  o:constructor(...)
  return o
end

function class:constructor(...)
end

return class
