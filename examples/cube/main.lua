local conf = {
    window = {
        resizable = true
    }
}
selene('Cube Example', '1.0.0', 'org.selene.CubeExample', conf)

local renderer = selene.get_renderer()

-- Define vertices for a cube.
local cube_vertices = {
    -- front face (z = 0.5)
    {-0.5, -0.5,  0.5, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 },
    { 0.5, -0.5,  0.5, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 },
    { 0.5,  0.5,  0.5, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 },
    {-0.5,  0.5,  0.5, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 },
    -- back face (z = -0.5)
    {-0.5, -0.5, -0.5, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0 },
    {-0.5,  0.5, -0.5, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0 },
    { 0.5,  0.5,  -0.5, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0 },
    { 0.5,  -0.5, -0.5, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0 },
    -- left face (x = -0.5)
    {-0.5, -0.5, -0.5, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0 },
    {-0.5, -0.5, 0.5,  0.0, 0.0, 1.0, 1.0, 0.0, 0.0 },
    {-0.5, 0.5,  0.5,  0.0, 0.0, 1.0, 1.0, 0.0, 0.0 },
    {-0.5, 0.5,  -0.5, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0 },
    -- right face (x = 0.5)
    { 0.5,  -0.5, -0.5, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0 },
    { 0.5,  0.5,  -0.5, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0 },
    { 0.5,  0.5,  0.5,  1.0, 0.0, 1.0, 1.0, 0.0, 0.0 },
    { 0.5,  -0.5, 0.5,  1.0, 0.0, 1.0, 1.0, 0.0, 0.0 },
    -- top face (y = 0.5)
    { -0.5, 0.5,  -0.5, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0 },
    { -0.5, 0.5,  0.5,  1.0, 1.0, 0.0, 1.0, 0.0, 0.0 },
    { 0.5,  0.5,  0.5,  1.0, 1.0, 0.0, 1.0, 0.0, 0.0 },
    { 0.5,  0.5,  -0.5, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0 },
    -- bottom face (y = -0.5)
    { -0.5, -0.5, -0.5, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0 },
    { 0.5,  -0.5, -0.5, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0 },
    { 0.5,  -0.5, 0.5,  0.0, 1.0, 1.0, 1.0, 0.0, 0.0 },
    { -0.5, -0.5, 0.5,  0.0, 1.0, 1.0, 1.0, 0.0, 0.0 },
}

-- Define indices for the cube. Two triangles per face.
local cube_indices = {
    0, 1, 2,   0, 2, 3,      -- front face
    4, 5, 6,   4, 6, 7,      -- back face
    8, 9, 10,  8, 10, 11,    -- left face
    12, 13, 14, 12, 14, 15,   -- right face
    16, 17, 18, 16, 18, 19,   -- top face
    20, 21, 22, 20, 22, 23,   -- bottom face
}

print(renderer)
local x = 0
local y = 128
local z = -33

print(math.pi)
local last = sdl.get_ticks()
local time = 0

local dt = 0
local on_left = false
local on_right = false

local keys = {false, false, false, false}

selene.set_event(function(name, key, pressed)
    if name == 'quit' then selene.set_running(false)
    elseif name == 'window closed' then selene.set_running(false) end

    if name == 'key' then
        if key == sdl.get_scancode_from_name('left') then
            keys[1] = pressed
        elseif key == sdl.get_scancode_from_name('right') then
            keys[2] = pressed
        elseif key == sdl.get_scancode_from_name('up') then
            keys[3] = pressed
        elseif key == sdl.get_scancode_from_name('down') then
            keys[4] = pressed
        end
    end
end)

selene.set_step(function()
    renderer:clear(0.2, 0.3, 0.3, 1.0)
    --[[for i,val in ipairs(cube_indices) do
        local v = cube_vertices[val+1]
        renderer:push_vertex(v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9])
    end]]
    if keys[1] then
        x = x - (80*dt)
    elseif keys[2] then
        x = x + (80*dt)
    end

    if keys[3] then
        z = z - (80*dt)
    elseif keys[4] then
        z = z + (80*dt)
    end
    local current = sdl.get_ticks()
    local delta = (current - last)
    --print(delta, current, last)
    last = current
    dt = delta / 1000
    time = time + dt
    local ay = math.rad(45)*time
    --print(ay, time)
    renderer:push_cube({128, 64, -26}, {0, ay, math.rad(15)*time}, {32, 32, 32})

    renderer:push_cube({x, y, z}, {0, math.rad(60)*time, 0}, {16, 64, 32})
end)
