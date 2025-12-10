local shaders = {}

function shaders.compile_all(renderer)
    local list = {
        ['SPRITE2D'] = {
            vertex = renderer:load_shader('vertex', 'root://spv/spr2d_vs.spv'),
            pixel = renderer:load_shader('pixel', 'root://spv/spr2d_fs.spv')
        },
        ['PRIMITIVE2D'] = {
            vertex = renderer:create_shader('vertex', 'root://spv/priv2d_vs.spv'),
            pixel = renderer:create_shader('pixel', 'root://spv/priv2d_fs.spv')
        }
    }

    return list
end

return shaders
