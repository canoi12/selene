--- @class Room
local Room = {}

local room_mt = {}
room_mt.__index = Room

function Room.create()
    local r = {}
    r.entities = {}

    return setmetatable(r, room_mt)
end

function Room:update(dt)
end

function Room:draw()
end

function Room:onEvent(ev)
end

return Room