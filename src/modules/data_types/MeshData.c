#include "selene.h"
#include "lua_helper.h"

META_INT_GET_FIELD(MeshData, num_vertices)
META_INT_GET_FIELD(MeshData, num_indices)
META_INT_GET_FIELD(MeshData, num_triangles)
META_INT_GET_FIELD(MeshData, data_size)
META_PTR_GET_FIELD(MeshData, void, vertices)
META_PTR_GET_FIELD(MeshData, void, normals)
META_PTR_GET_FIELD(MeshData, void, texcoords)
META_PTR_GET_FIELD(MeshData, void, indices)

MODULE_FUNCTION(MeshData, cubeData) {
    INIT_ARG();
    int size = 6 * 4 * 4; // vertices count (6 faces + 4 vertices each + 4 bytes (size of float))
    size *= 3 * 3 * 2; // 3 positions, 3 normals, 2 texcoords
    size *= 6 * 6 * 4; // indices count (6 faces + 6 indices each + 4 bytes (size of unsigned int))
    NEW_UDATA_ADD(MeshData, mesh, size);
    mesh->num_vertices = 24;
    mesh->num_triangles = 36;
    mesh->num_indices = 36;
    int vert_size = 4 * 3;
    char* vert_data = (char*)(mesh + 1);
    float vertices[] = {
        // Face 1 (frontal)
         0.5,  0.5,  0.5, // Vértice 1
        -0.5,  0.5,  0.5, // Vértice 2
        -0.5, -0.5,  0.5, // Vértice 3
         0.5, -0.5,  0.5, // Vértice 4

        // Face 2 (traseira)
         0.5,  0.5, -0.5, // Vértice 5
        -0.5,  0.5, -0.5, // Vértice 6
        -0.5, -0.5, -0.5, // Vértice 7
         0.5, -0.5, -0.5, // Vértice 8

        // Face 3 (esquerda)
        -0.5,  0.5,  0.5, // Vértice 9
        -0.5,  0.5, -0.5, // Vértice 10
        -0.5, -0.5, -0.5, // Vértice 11
        -0.5, -0.5,  0.5, // Vértice 12

        // Face 4 (direita)
         0.5,  0.5,  0.5, // Vértice 13
         0.5,  0.5, -0.5, // Vértice 14
         0.5, -0.5, -0.5, // Vértice 15
         0.5, -0.5,  0.5, // Vértice 16

        // Face 5 (superior)
         0.5,  0.5, -0.5, // Vértice 17
        -0.5,  0.5, -0.5, // Vértice 18
        -0.5,  0.5,  0.5, // Vértice 19
         0.5,  0.5,  0.5, // Vértice 20

        // Face 6 (inferior)
         0.5, -0.5, -0.5, // Vértice 21
        -0.5, -0.5, -0.5, // Vértice 22
        -0.5, -0.5,  0.5, // Vértice 23
         0.5, -0.5,  0.5  // Vértice 24
    };
    mesh->vertices = (float*)vert_data;
    memcpy(mesh->vertices, vertices, sizeof(vertices));

    // Normais de cada face
    float normals[] = {
        // Face 1 (frontal)
         1.0,  0.0,  1.0, // Normal
         1.0,  0.0,  1.0,
         1.0,  0.0,  1.0,
         1.0,  0.0,  1.0,

        // Face 2 (traseira)
         0.0,  1.0,  1.0, // Normal
         0.0,  1.0,  1.0,
         0.0,  1.0,  1.0,
         0.0,  1.0,  1.0,

        // Face 3 (esquerda)
         1.0,  1.0,  0.0, // Normal
         1.0,  1.0,  0.0,
         1.0,  1.0,  0.0,
         1.0,  1.0,  0.0,

        // Face 4 (direita)
         1.0,  0.0,  0.0, // Normal
         1.0,  0.0,  0.0,
         1.0,  0.0,  0.0,
         1.0,  0.0,  0.0,

        // Face 5 (superior)
         0.0,  1.0,  0.0, // Normal
         0.0,  1.0,  0.0,
         0.0,  1.0,  0.0,
         0.0,  1.0,  0.0,

        // Face 6 (inferior)
         0.0,  0.0,  1.0, // Normal
         0.0,  0.0,  1.0,
         0.0,  0.0,  1.0,
         0.0,  0.0,  1.0
    };

    char* norm_data = vert_data + (3 * 4 * 4 * 6);
    mesh->normals = (float*)norm_data;
    memcpy(mesh->normals, normals, sizeof(normals));

    // Coordenadas de textura (0.0 a 1.0)
    float texcoords[] = {
        // Face 1 (frontal)
         1.0,  1.0, // Vértice 1
         0.0,  1.0, // Vértice 2
         0.0,  0.0, // Vértice 3
         1.0,  0.0, // Vértice 4

        // Face 2 (traseira)
         1.0,  1.0, // Vértice 5
         0.0,  1.0, // Vértice 6
         0.0,  0.0, // Vértice 7
         1.0,  0.0, // Vértice 8

        // Face 3 (esquerda)
         1.0,  1.0, // Vértice 9
         0.0,  1.0, // Vértice 10
         0.0,  0.0, // Vértice 11
         1.0,  0.0, // Vértice 12

        // Face 4 (direita)
         1.0,  1.0, // Vértice 13
         0.0,  1.0, // Vértice 14
         0.0,  0.0, // Vértice 15
         1.0,  0.0, // Vértice 16

        // Face 5 (superior)
         1.0,  1.0, // Vértice 17
         0.0,  1.0, // Vértice 18
         0.0,  0.0, // Vértice 19
         1.0,  0.0, // Vértice 20

        // Face 6 (inferior)
         1.0,  1.0, // Vértice 21
         0.0,  1.0, // Vértice 22
         0.0,  0.0, // Vértice 23
         1.0,  0.0  // Vértice 24
    };
    char* tex_data = norm_data + (3 * 4 * 4 * 6);
    mesh->texcoords = (float*)tex_data;
    memcpy(mesh->texcoords, texcoords, sizeof(texcoords));

    char* index_data = tex_data + (2 * 4 * 4 * 6);
    mesh->indices = (unsigned int*)index_data;
    unsigned int indices[] = {
        // Frente
        0, 1, 2,   0, 2, 3,
        // Trás
        4, 5, 6,   4, 6, 7,
        // Esquerda
        8, 9, 10,  8, 10, 11,
        // Direita
        12, 13, 14, 12, 14, 15,
        // Topo
        16, 17, 18, 16, 18, 19,
        // Fundo
        20, 21, 22, 20, 22, 23
    };
    memcpy(mesh->indices, indices, sizeof(indices));

    mesh->data_size = sizeof(vertices) + sizeof(normals) + sizeof(texcoords) + sizeof(indices);

    return 1;
}

META_FUNCTION(MeshData, index);
META_FUNCTION(MeshData, index) {
    CHECK_META(MeshData);
    CHECK_STRING(key);

    lua_rawgetp(L, LUA_REGISTRYINDEX, l_MeshData__index);
    lua_getfield(L, -1, key);
    lua_remove(L, -2);
    if (lua_type(L, -1) != LUA_TNIL) {
        lua_pushvalue(L, 1);
        if (lua_pcall(L, 1, 1, 0) != LUA_OK)
            return luaL_error(L, "[selene] failed to get MeshData field %s", key);
        return 1;
    }
    lua_pop(L, 1);

    luaL_getmetatable(L, "MeshData");
    lua_getfield(L, -1, key);
    lua_remove(L, -2);
    
    return 1;
}

static BEGIN_META_REG(field_regs)
    META_REG_GET_FIELD(MeshData, num_vertices),
    META_REG_GET_FIELD(MeshData, num_indices),
    META_REG_GET_FIELD(MeshData, num_triangles),
    META_REG_GET_FIELD(MeshData, data_size),
    META_REG_GET_FIELD(MeshData, vertices),
    META_REG_GET_FIELD(MeshData, normals),
    META_REG_GET_FIELD(MeshData, texcoords),
    META_REG_GET_FIELD(MeshData, indices),
END_META_REG()

int luaopen_MeshData(lua_State* L) {
    luaL_newmetatable(L, "MeshData");
    lua_pushcfunction(L, l_MeshData__index);
    lua_setfield(L, -2, "__index");
    LOAD_GET_META_FIELDS(MeshData, field_regs);
    BEGIN_REG(reg)
        REG_FIELD(MeshData, cubeData),
    END_REG()
    luaL_setfuncs(L, reg, 0);
    return 1;
}