local lines = {}
for line in io.lines(selene.__dir .. "/CatModel/12221_Cat_v1_l3.obj") do
    table.insert(lines, line)
end

local vertices = {}
local indices = {}
local vert_pos = 1
local coord_pos = 1
for i,line in ipairs(lines) do
    local terms = {}
    for term in line:gmatch('[^ ]+') do
        table.insert(terms, term)
    end
    if terms[1] == "v" then
        if not vertices[vert_pos] then
            local v = {0, 0, 0, 1, 1, 1, 0, 0}
            vertices[vert_pos] = v
        end
        local t = vertices[vert_pos]
        -- print(i, terms[2], terms[3], terms[4])
        t[1] = tonumber(terms[2])
        t[2] = tonumber(terms[3])
        t[3] = tonumber(terms[4])
        vert_pos = vert_pos + 1
    end
    if terms[1] == "vt" then
        if not vertices[coord_pos] then
            local v = {0, 0, 0, 1, 1, 1, 0, 0}
            vertices[coord_pos] = v
        end
        -- print(i, terms[2], terms[3])
        local t = vertices[coord_pos]
        t[7] = tonumber(terms[2])
        t[8] = tonumber(terms[3])
        coord_pos = coord_pos + 1
    end
    if terms[1] == "f" then
        -- print(i, , terms[3], terms[4])
        for j=2,#terms do
            local s = string.find(terms[j], '/')
            
            if s then
            table.insert(indices, tonumber(string.sub(terms[j], 1, s-1)))
            end
        end
        -- table.insert(indices, tonumber(terms[2]))
        -- table.insert(indices, tonumber(terms[5]))
        -- table.insert(indices, tonumber(terms[8]))
        -- table.insert(indices, tonumber(terms[11]))
    end
end

return { vertices = vertices, indices = indices }