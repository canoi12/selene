local record Class
    constructor: function()
end

function Class:Extend(name: string)
    local mt : metatable<Class>
    mt = {
        __index = self,
        __name = name,
        __call = self.new
    }
    for k,v in pairs(self) do
        if k:find("__") then
            mt[k] = v
        end
    end
    local o = setmetatable({}, mt)
    o.__class = name
    return o
end

function Class:New(...)
    local mt : metatable<Class>
    mt = {
        __index = self,
        __name = self.__class
    }
    for k,v in pairs(self) do
        if k:find("__") then
            mt[k] = v
        end
    end
    local o = setmetatable({}, mt)
    o:constructor(...)
    return o
end

return Class