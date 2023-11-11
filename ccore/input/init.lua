local Input = {}

local input_mt = {}
input_mt.RegisterAction = function(self, name)
  if not self.actions[name] then
    self.actions[name] = {}
  end
end

input_mt.AddAction = function(self, name, device, bind)
  assert(self.actions[name], "Action not registered")
  table.insert(self.actions[name], { device = device, bind = bind })
end

input_mt.IsActionPressed = function(self, name)
  for i,a in ipairs(self.actions) do
  end
end

function Input.New()
  return setmetatable({
      actions = {},
      axis = {}
  }, input_mt)
end

return Input
