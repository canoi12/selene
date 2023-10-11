local class = {}
class.__class = "Class"

function class:extend(name)
  local mt = { __index = self, __call = self.new }
  for k,v in pairs(self) do
    if k:find('__') then
      mt[k] = v
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
      mt[k] = v
    end
  end
  local o = setmetatable({}, mt)
  o:constructor(...)
  return o
end

function class:is(name)
  return self.__class == name
end

function class:constructor(...)
end

return class
