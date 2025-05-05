#include "selene_renderer.h"
#ifndef SELENE_NO_VULKAN
struct QueueFamilyIndices {
    int graphics_family;
    int present_family;
};

static struct QueueFamilyIndices find_queue_families(VkPhysicalDevice dev, VkSurfaceKHR surface) {
    struct QueueFamilyIndices indices = {-1, -1};

    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, NULL);


    VkQueueFamilyProperties* families = malloc(sizeof(VkQueueFamilyProperties) * count);
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, families);
    for (int i = 0; i < count; i++) {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = i;
        }
        VkBool32 present_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surface, &present_support);
        if (present_support) indices.present_family = i;

        if (indices.present_family != -1 && indices.graphics_family != -1)
            goto RETURN;
    }
    fprintf(stderr, "No suitable queue family found\n");

RETURN:
    return indices;
}

const int vk_buffer_target_types_values[] = {VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT};

static int find_memory_type(VkPhysicalDevice phys, int type_filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(phys, &mem_props);
    for (int i = 0; i < mem_props.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }
    return -1;
}

int l_VK_Renderer__destroy(lua_State* L) {
    CHECK_META(selene_Renderer);
    if (self->vk.surface) vkDestroySurfaceKHR(self->vk.instance, self->vk.surface, NULL);
    if (self->vk.instance) vkDestroyInstance(self->vk.instance, NULL);
    if (self->vk.device) vkDestroyDevice(self->vk.device, NULL);
    free(self->command_pool);
    return 0;
}

int l_VK_Renderer__create_pipeline(lua_State* L) {
    CHECK_META(selene_Renderer);
    if (!lua_istable(L, arg)) return luaL_argerror(L, arg, "must be a table");
    lua_getfield(L, arg, "vs");
    selene_Shader* vertex = (selene_Shader*)luaL_checkudata(L, -1, "SeleneShader");
    lua_getfield(L, arg, "ps");
    selene_Shader* pixel = (selene_Shader*)luaL_checkudata(L, -1, "SeleneShader");
    lua_pop(L, 2);

    VkPipelineShaderStageCreateInfo shader_stages[2] = {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertex->vk.handle,
            .pName = "main"
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = pixel->vk.handle,
            .pName = "main"
        }
    };

    if (lua_getfield(L, arg, "layout") != LUA_TTABLE) return luaL_error(L, "invalid layout field");
    NEW_UDATA(selene_RenderPipeline, pipe);
    if (lua_getfield(L, -2, "stride") == LUA_TNUMBER) pipe->layout.vertex_stride = lua_tointeger(L, -1);
    lua_pop(L, 1);
    int len = lua_rawlen(L, -2);
    pipe->layout.count = len;
    for (int i = 0; i < len; i++) {
        lua_rawgeti(L, -2, i+1);
        lua_getfield(L, -1, "name");
        const char* name = luaL_checkstring(L, -1);
        //pipe->layout.attributes[i].binding = glGetAttribLocation(program, name);
        pipe->layout.attributes[i].binding = 0;

        lua_pop(L, 1);
        lua_getfield(L, -1, "offset");
        pipe->layout.attributes[i].offset = lua_tointeger(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, -1, "size");
        const int size = lua_tointeger(L, -1);
        pipe->layout.attributes[i].size = size;
        lua_pop(L, 1);

        lua_getfield(L, -1, "type");
        int opt = luaL_checkoption(L, -1, NULL, type_name_options);
        pipe->layout.attributes[i].type = opt;
        lua_pop(L, 1);
        fprintf(stdout, "name: %s, offset: %d, size: %d, type: %d\n", name, pipe->layout.attributes[i].offset, size, pipe->layout.attributes[i].type);
        switch (opt) {
            case 0:
            case 1:
                pipe->layout.attributes[i].stride = size;
                break;
            case 2:
            case 3:
                pipe->layout.attributes[i].stride = size * 2;
                break;
            default:
                pipe->layout.attributes[i].stride = size * 4;
        }
        lua_pop(L, 1);
    }

    VkVertexInputBindingDescription binding_desc = {
        .binding = 0,
        .stride = pipe->layout.vertex_stride,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkVertexInputAttributeDescription attrs_descs[8] = {};
    int offset_sum = 0;
    for (int i = 0; i < len; i++) {
        VkVertexInputAttributeDescription* attr = attrs_descs + i;
        int binding = pipe->layout.attributes[i].binding;
        int offset = pipe->layout.attributes[i].offset;
        int size = pipe->layout.attributes[i].size;
        int type = pipe->layout.attributes[i].type;
        int stride = pipe->layout.vertex_stride;
        switch (type) {
            case SELENE_FLOAT: {
                if (size == 1) attr->format = VK_FORMAT_R32_SFLOAT;
                else if (size == 2) attr->format = VK_FORMAT_R32G32_SFLOAT;
                else if (size == 3) attr->format = VK_FORMAT_R32G32B32_SFLOAT;
                else if (size == 4) attr->format = VK_FORMAT_R32G32B32A32_SFLOAT;
            }
            break;
        }
        attr->binding = binding;
        attr->location = offset;
        attr->offset = offset_sum;
        offset_sum += stride;
    }

    VkPipelineVertexInputStateCreateInfo vert_input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = len,
        .pVertexBindingDescriptions = &binding_desc,
        .vertexAttributeDescriptionCount = len,
        .pVertexAttributeDescriptions = attrs_descs
    };

    VkDynamicState dyn_states[2] = {
        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dyn_state_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 2,
        .pDynamicStates = dyn_states
    };

    VkPipelineInputAssemblyStateCreateInfo input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    VkPipelineRasterizationStateCreateInfo raster_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasConstantFactor = VK_FALSE,
        .lineWidth = 1.f
    };

    VkPipelineDepthStencilStateCreateInfo depth_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_FALSE,
        .depthWriteEnable = VK_FALSE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE
    };

    VkPipelineColorBlendAttachmentState color_blend_attach = {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo color_blend_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attach
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
    };

    VkPipelineLayout pipeline_layout;
    vkCreatePipelineLayout(self->vk.device, &pipeline_layout_info, NULL, &pipeline_layout);

    VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shader_stages,
        .pVertexInputState = &vert_input_info,
        .pInputAssemblyState = &input_info,
        .pRasterizationState = &raster_info,
        .pDepthStencilState = &depth_info,
        .pColorBlendState = &color_blend_info,
        .pDynamicState = &dyn_state_info,
        .layout = pipeline_layout,
        .renderPass = self->vk.render_pass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    VkPipeline handle;
    if (vkCreateGraphicsPipelines(self->vk.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &handle) != VK_SUCCESS) {
        return luaL_error(L, "failed to create Vulkan pipeline");
    }

    NEW_UDATA(selene_RenderPipeline, pipeline);
    pipeline->vk.handle = handle;
    pipeline->vk.layout = pipeline_layout;

    return 1;
}

static int l_VK_Renderer__destroy_pipeline(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_RenderPipeline, pipeline);
    if (pipeline->vk.layout) vkDestroyPipelineLayout(self->vk.device, pipeline->vk.layout, NULL);
    if (pipeline->vk.handle) vkDestroyPipeline(self->vk.device, pipeline->vk.handle, NULL);
    pipeline->vk.layout = NULL;
    pipeline->vk.handle = NULL;
    return 0;
}

/**
 * Buffers
 */

int l_VK_Renderer__create_buffer(lua_State* L) {
    CHECK_META(selene_Renderer);
    int opt = luaL_checkoption(L, arg++, "vertex", buffer_target_options);
    int size = (int)luaL_checkinteger(L, arg++);
    VkBufferCreateInfo buf_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = vk_buffer_target_types_values[opt],
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    VkBuffer handle;
    if (vkCreateBuffer(self->vk.device, &buf_info, NULL, &handle) != VK_SUCCESS) {
        return luaL_error(L, "failed to create Vulkan buffer");
    }
    VkDeviceMemory buff_memory;
    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(self->vk.device, handle, &mem_reqs);
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(self->vk.phys_device, &mem_properties);

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_reqs.size,
        .memoryTypeIndex = find_memory_type(self->vk.phys_device, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    };

    if (vkAllocateMemory(self->vk.device, &alloc_info, NULL, &buff_memory) != VK_SUCCESS) {
        vkDestroyBuffer(self->vk.device, handle, NULL);
        return luaL_error(L, "failed to alloc memory for Vulkan buffer");
    }
    vkBindBufferMemory(self->vk.device, handle, buff_memory, 0);

    NEW_UDATA(GpuBuffer, buffer);
    buffer->type = opt;
    buffer->size = size;
    buffer->vk.handle = handle;
    buffer->vk.mem = buff_memory;
    return 1;
}

int l_VK_Renderer__destroy_buffer(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(GpuBuffer, buffer);
    if (buffer->vk.handle) vkDestroyBuffer(self->vk.device, buffer->vk.handle, NULL);
    if (buffer->vk.mem) vkFreeMemory(self->vk.device, buffer->vk.mem, NULL);
    buffer->vk.handle = NULL;
    buffer->vk.mem = NULL;
    return 0;
}

/**
 * Texture
 */

int l_VK_Renderer__create_texture2d(lua_State* L) {
    return 1;
}

/**
 * Shader
 */

int l_VK_Renderer__create_shader(lua_State* L) {
    CHECK_META(selene_Renderer);
    int opt = luaL_checkoption(L, arg++, "vertex", shader_type_options);
    const char* source = luaL_checkstring(L, arg++);
    VkShaderModule handle = NULL;
    VkShaderModuleCreateInfo shader_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = strlen(source),
        .pCode = (void*)source
    };
    if (vkCreateShaderModule(self->vk.device, &shader_info, NULL, &handle) != VK_SUCCESS) {
        return luaL_error(L, "failed to create Vulkan shader");
    }
    NEW_UDATA(selene_Shader, shader);
    shader->type = opt;
    shader->vk.handle = handle;
    return 1;
}

int l_VK_Renderer__destroy_shader(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_Shader, shader);
    if (shader->vk.handle) vkDestroyShaderModule(self->vk.device, shader->vk.handle, NULL);
    shader->vk.handle = NULL;
    return 0;
}

/**
 * Draw and Flush
 */

int l_VK_Renderer__draw(lua_State* L) {
    return 0;
}

int l_VK_Renderer_create(lua_State* L) {
    INIT_ARG();
    SDL_Window** win = (SDL_Window**)luaL_checkudata(L, arg++, "sdlWindow");
    int width, height;
    SDL_GetWindowSize(*win, &width, &height);

    VkInstance instance;
    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan App",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "selene",
        .engineVersion = VK_MAKE_VERSION(0, 5, 0),
        .apiVersion = VK_API_VERSION_1_0
    };

    unsigned int ext_count;
    if (SDL_Vulkan_GetInstanceExtensions(*win, &ext_count, NULL) != VK_TRUE) {
        return luaL_error(L, "failed to get Vulkan extensions count: %s", SDL_GetError());
    }

    const char** exts = malloc(sizeof(const char*) * ext_count);
    SDL_Vulkan_GetInstanceExtensions(*win, &ext_count, exts);

    VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = ext_count,
        .ppEnabledExtensionNames = exts,
        .enabledLayerCount = 0
    };

    if (vkCreateInstance(&create_info, NULL, &instance) != VK_SUCCESS) {
        free(exts);
        return luaL_error(L, "failed to create Vulkan instance");
    }
    free(exts);
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(*win, instance, &surface)) {
        vkDestroyInstance(instance, NULL);
        return luaL_error(L, "failed to create Vulkan surface: %s", SDL_GetError());
    }
    uint32_t dev_count = 0;
    vkEnumeratePhysicalDevices(instance, &dev_count, NULL);
    if (dev_count == 0) {
        vkDestroySurfaceKHR(instance, surface, NULL);
        vkDestroyInstance(instance, NULL);
        return luaL_error(L, "no Vulkan compatible devices found");
    }
    VkPhysicalDevice* devices = malloc(dev_count * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &dev_count, devices);

    VkPhysicalDevice phys = devices[0];
    free(devices);

    VkPhysicalDeviceProperties pdev_props;
    vkGetPhysicalDeviceProperties(phys, &pdev_props);

    VkPhysicalDeviceFeatures pdev_features;
    vkGetPhysicalDeviceFeatures(phys, &pdev_features);

    struct QueueFamilyIndices indices = find_queue_families(phys, surface);

    float queue_priority = 1.f;
    VkDeviceQueueCreateInfo queue_crt_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = indices.graphics_family,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority
    };

    VkBool32 present_support = VK_FALSE;
    // vkGetPhysicalDeviceSurfaceSupportKHR()
    
    VkDeviceCreateInfo dev_crt_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = &queue_crt_info,
        .queueCreateInfoCount = 1,
        .pEnabledFeatures = &pdev_features,
        .enabledExtensionCount = 0,
        .enabledLayerCount = 0
    };

    VkDevice device;
    if (vkCreateDevice(phys, &dev_crt_info, NULL, &device) != VK_SUCCESS) {
        vkDestroySurfaceKHR(instance, surface, NULL);
        vkDestroyInstance(instance, NULL);
        return luaL_error(L, "failed to create Vulkan device");
    }

    VkRenderPassCreateInfo rpass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
    };
    VkRenderPass render_pass;

    VkQueue gfx_queue;
    vkGetDeviceQueue(device, indices.graphics_family, 0, &gfx_queue);

    NEW_UDATA(selene_Renderer, ren);
    ren->vk.device = device;
    ren->vk.instance = instance;
    ren->vk.surface = surface;
    ren->vk.phys_device = phys;
    ren->window_ptr = *win;
    lua_pushvalue(L, 1);
    ren->l_window_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    ren->command_offset = 0;
    ren->command_count = 512;
    ren->command_pool = (struct RenderCommand*)malloc(sizeof(struct RenderCommand)*ren->command_count);

    ren->backend = SELENE_RENDERER_VULKAN;

    ren->destroy = l_VK_Renderer__destroy;

    ren->create_pipeline = l_VK_Renderer__create_pipeline;
    ren->destroy_pipeline = l_VK_Renderer__destroy_pipeline;

    return 1;
}
#endif
