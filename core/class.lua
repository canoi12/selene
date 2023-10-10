local class = {}
class.__class = "Class"

function class:extend(name)
  local mt = { __index = self }
  for k,v in pairs(self) do
    if k:find('__') then
      mt[v] = k
    end
  end
  local o = setmetatable({}, mt)
  o.__class = name
  return o
end

function class:new(...)
  local mt = { __index = self }
  for k,v in pairs(self) do
    if k:find('__') then
      print(k)
      mt[k] = v
    end
  end
  local o = setmetatable({}, mt)
  o:constructor(...)
  return o
end

function class:constructor(...)
end

return class
