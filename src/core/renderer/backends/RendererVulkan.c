#ifndef SELENE_NO_VULKAN
#include "selene_renderer.h"

const int vk_pixel_formats_values[] = {VK_FORMAT_R8G8B8_UNORM,
                                       VK_FORMAT_R8G8B8A8_UNORM};

struct QueueFamilyIndices {
    int graphics_family;
    int present_family;
};

#if 0
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
#endif

void vk_create_swapchain(VkPhysicalDevice physical_device, VkDevice device,
                         VkSurfaceKHR surface,
                         struct QueueFamilyIndices indices, uint32_t width,
                         uint32_t height, struct VulkanSwapchain *outSwapchain);
void vk_destroy_swapchain(VkPhysicalDevice physical_device, VkDevice device,
                          struct VulkanSwapchain outSwapchain);
VkResult createTexture(VkDevice device, VkPhysicalDevice physicalDevice,
                       uint32_t width, uint32_t height, VkFormat format,
                       VkImageTiling tiling, VkImageUsageFlags usage,
                       VkMemoryPropertyFlags properties, Texture2D *outTexture);
int vk_create_buffer(selene_Renderer *self, int size, int usage, int flags, VkBuffer *out_buf, VkDeviceMemory *out_mem);
int transition_image_layout(selene_Renderer* self, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
void copy_buffer_to_image(selene_Renderer* self, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

static struct QueueFamilyIndices find_queue_families(VkPhysicalDevice dev,
                                                     VkSurfaceKHR surface) {
    struct QueueFamilyIndices indices = {-1, -1};

    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, NULL);

    VkQueueFamilyProperties *families =
        malloc(sizeof(VkQueueFamilyProperties) * count);
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, families);
    for (int i = 0; i < count; i++) {
      if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indices.graphics_family = i;
      }
      VkBool32 present_support = VK_FALSE;
      vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, surface, &present_support);
      if (present_support)
        indices.present_family = i;

      if (indices.present_family != -1 && indices.graphics_family != -1)
        goto RETURN;
    }
    fprintf(stderr, "No suitable queue family found\n");

RETURN:
    return indices;
}

const int vk_buffer_target_types_values[] = {
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT};

static int find_memory_type(VkPhysicalDevice phys, int type_filter,
                            VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties mem_props;
  vkGetPhysicalDeviceMemoryProperties(phys, &mem_props);
  for (int i = 0; i < mem_props.memoryTypeCount; i++) {
    if ((type_filter & (1 << i)) &&
        (mem_props.memoryTypes[i].propertyFlags & properties) == properties)
      return i;
  }
  return -1;
}

int l_VK_Renderer__destroy(lua_State *L) {
    CHECK_META(selene_Renderer);
    fprintf(stdout, "Destroying Vulkan renderer\n");
    VkDevice dev = self->vk.device;
    vkDeviceWaitIdle(dev);
    if (self->vk.image_semaphore) vkDestroySemaphore(dev, self->vk.image_semaphore, NULL);
    if (self->vk.render_semaphore) vkDestroySemaphore(dev, self->vk.render_semaphore, NULL);
    if (self->vk.fence) vkDestroyFence(dev, self->vk.fence, NULL);
    
    self->vk.image_semaphore = NULL;
    self->vk.render_semaphore = NULL;
    self->vk.fence = NULL;

    vk_destroy_swapchain(self->vk.phys_device, self->vk.device,
                        self->vk.swapchain);
    for (int i = 0; i < self->vk.swapchain.image_count; i++) {
        vkDestroyFramebuffer(self->vk.device, self->vk.framebuffers[i], NULL);
    }
    free(self->vk.framebuffers);
    if (self->vk.command_buffer)
        vkFreeCommandBuffers(dev, self->vk.command_pool, 1,
                            &(self->vk.command_buffer));
    if (self->vk.command_pool)
        vkDestroyCommandPool(dev, self->vk.command_pool, NULL);
    if (self->vk.render_pass)
        vkDestroyRenderPass(dev, self->vk.render_pass, NULL);
    if (self->vk.device)
        vkDestroyDevice(self->vk.device, NULL);
    if (self->vk.surface)
        vkDestroySurfaceKHR(self->vk.instance, self->vk.surface, NULL);
    if (self->vk.instance)
        vkDestroyInstance(self->vk.instance, NULL);
    if (self->command_pool)
        free(self->command_pool);

    self->vk.surface = VK_NULL_HANDLE;
    self->vk.instance = VK_NULL_HANDLE;
    self->vk.device = VK_NULL_HANDLE;
    self->vk.command_buffer = VK_NULL_HANDLE;
    self->vk.command_pool = VK_NULL_HANDLE;
    self->vk.render_pass = VK_NULL_HANDLE;

    self->command_pool = NULL;
    return 0;
}

int l_VK_Renderer__create_pipeline(lua_State *L) {
    CHECK_META(selene_Renderer);
    if (!lua_istable(L, arg))
        return luaL_argerror(L, arg, "must be a table");
    lua_getfield(L, arg, "vs");
    selene_Shader *vertex = (selene_Shader *)luaL_checkudata(L, -1, "selene_Shader");
    lua_getfield(L, arg, "ps");
    selene_Shader *pixel = (selene_Shader *)luaL_checkudata(L, -1, "selene_Shader");
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

    if (lua_getfield(L, arg, "layout") != LUA_TTABLE)
        return luaL_error(L, "invalid layout field");
    NEW_UDATA(selene_RenderPipeline, pipe);
    if (lua_getfield(L, -2, "stride") == LUA_TNUMBER)
        pipe->layout.vertex_stride = lua_tointeger(L, -1);
    lua_pop(L, 1);
    int len = lua_rawlen(L, -2);
    pipe->layout.count = len;
    for (int i = 0; i < len; i++) {
        lua_rawgeti(L, -2, i + 1);
        lua_getfield(L, -1, "name");
        const char *name = luaL_checkstring(L, -1);
        // pipe->layout.attributes[i].binding = glGetAttribLocation(program, name);
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
        fprintf(stdout, "name: %s, offset: %d, size: %d, type: %d\n", name,
                pipe->layout.attributes[i].offset, size,
                pipe->layout.attributes[i].type);
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
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription attrs_descs[8] = {0};
    int offset_sum = 0;
    for (int i = 0; i < len; i++) {
        VkVertexInputAttributeDescription *attr = attrs_descs + i;
        int binding = pipe->layout.attributes[i].binding;
        int offset = pipe->layout.attributes[i].offset;
        int size = pipe->layout.attributes[i].size;
        int type = pipe->layout.attributes[i].type;
        int stride = pipe->layout.vertex_stride;
        switch (type) {
            case SELENE_FLOAT: {
                if (size == 1)
                  attr->format = VK_FORMAT_R32_SFLOAT;
                else if (size == 2)
                  attr->format = VK_FORMAT_R32G32_SFLOAT;
                else if (size == 3)
                  attr->format = VK_FORMAT_R32G32B32_SFLOAT;
                else if (size == 4)
                  attr->format = VK_FORMAT_R32G32B32A32_SFLOAT;
            } break;
        }
        attr->binding = binding;
        attr->location = i;
        attr->offset = offset_sum;
        offset_sum += pipe->layout.attributes[i].stride;
    }

    VkPipelineVertexInputStateCreateInfo vert_input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding_desc,
        .vertexAttributeDescriptionCount = len,
        .pVertexAttributeDescriptions = attrs_descs};

    VkPipelineViewportStateCreateInfo viewport_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1
    };

    VkDynamicState dyn_states[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

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
        .depthBiasEnable = VK_FALSE,
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
        .stencilTestEnable = VK_FALSE,
    };

    VkPipelineMultisampleStateCreateInfo multisampling_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

    VkPipelineColorBlendAttachmentState color_blend_attach = {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo color_blend_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attach,
        .blendConstants[0] = 0.f,
        .blendConstants[1] = 0.f,
        .blendConstants[2] = 0.f,
        .blendConstants[3] = 0.f,
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0
    };

    VkPipelineLayout pipeline_layout;
    if (vkCreatePipelineLayout(self->vk.device, &pipeline_layout_info, NULL, &pipeline_layout) != VK_FALSE) {
        return luaL_error(L, "failed to create Vulkan pipeline layout");
    }

    VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shader_stages,
        .pVertexInputState = &vert_input_info,
        .pInputAssemblyState = &input_info,
        .pRasterizationState = &raster_info,
        .pViewportState = &viewport_info,
        .pMultisampleState = &multisampling_info,
        // .pDepthStencilState = &depth_info,
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
        vkDestroyPipelineLayout(self->vk.device, pipeline_layout, NULL);
        return luaL_error(L, "failed to create Vulkan pipeline");
    }
    if (handle == VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(self->vk.device, pipeline_layout, NULL);
        return luaL_error(L, "failed to create Vulkan pipeline, NULL handle");
    }

    NEW_UDATA(selene_RenderPipeline, pipeline);
    pipeline->vk.handle = handle;
    pipeline->vk.layout = pipeline_layout;
    fprintf(stderr, "pipeline %p, handle: %p\n", pipeline, handle);
    return 1;
}

static int l_VK_Renderer__destroy_pipeline(lua_State *L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_RenderPipeline, pipeline);
    vkDeviceWaitIdle(self->vk.device);
    if (pipeline->vk.layout)
        vkDestroyPipelineLayout(self->vk.device, pipeline->vk.layout, NULL);
    if (pipeline->vk.handle)
        vkDestroyPipeline(self->vk.device, pipeline->vk.handle, NULL);
    pipeline->vk.layout = NULL;
    pipeline->vk.handle = NULL;
    return 0;
}

/**
 * Buffers
 */

int l_VK_Renderer__create_buffer(lua_State *L) {
    CHECK_META(selene_Renderer);
    int opt = luaL_checkoption(L, arg++, "vertex", buffer_target_options);
    int size = (int)luaL_checkinteger(L, arg++);
    VkBufferCreateInfo buf_info = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                    .size = size,
                                    .usage = vk_buffer_target_types_values[opt],
                                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
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
        .memoryTypeIndex =
            find_memory_type(self->vk.phys_device, mem_reqs.memoryTypeBits,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)};

    if (vkAllocateMemory(self->vk.device, &alloc_info, NULL, &buff_memory) !=
        VK_SUCCESS) {
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

int l_VK_Renderer__destroy_buffer(lua_State *L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(GpuBuffer, buffer);
    vkDeviceWaitIdle(self->vk.device);
    if (buffer->vk.handle)
        vkDestroyBuffer(self->vk.device, buffer->vk.handle, NULL);
    if (buffer->vk.mem)
        vkFreeMemory(self->vk.device, buffer->vk.mem, NULL);
    buffer->vk.handle = NULL;
    buffer->vk.mem = NULL;
    return 0;
}

int l_VK_Renderer__send_buffer_data(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(GpuBuffer, buffer);
    CHECK_INTEGER(size);
    if (!lua_isuserdata(L, arg)) return luaL_argerror(L, arg, "userdata or lightuserdata expected");
    void* data = lua_touserdata(L, arg++);
    void* out;
    vkMapMemory(self->vk.device, buffer->vk.mem, 0, size, 0, &out);
    memcpy(out, data, size);
    vkUnmapMemory(self->vk.device, buffer->vk.mem);
    return 0;
}

/**
 * Texture
 */

int l_VK_Renderer__create_texture2d(lua_State *L) {
    CHECK_META(selene_Renderer);
    CHECK_INTEGER(width);
    CHECK_INTEGER(height);
    int opt = luaL_checkoption(L, arg++, "rgba", pixel_formats);
    void *data = NULL;
    if (lua_isuserdata(L, arg))
        data = lua_touserdata(L, arg++);
    int target = 0;
    if (lua_isboolean(L, arg))
        target = lua_toboolean(L, arg++);
    VkFormat pixel_format = vk_pixel_formats_values[opt]; // Map to VkFormat

    VkImageCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent = {.width = width, .height = height, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = pixel_format,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .flags = 0};
    if (target)
        create_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Render target

    VkImage handle;
    if (vkCreateImage(self->vk.device, &create_info, NULL, &handle) != VK_SUCCESS) {
        return luaL_error(L, "Failed to create Vulkan image");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(self->vk.device, handle, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(
        self->vk.phys_device,
        memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    VkDeviceMemory memory;
    if (vkAllocateMemory(self->vk.device, &allocInfo, NULL, &memory) != VK_SUCCESS) {
        vkDestroyImage(self->vk.device, handle, NULL);
        return luaL_error(L, "Failed to allocate Vulkan image memory");
    }
    vkBindImageMemory(self->vk.device, handle, memory, 0);

    if (data) {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        int res = vk_create_buffer(
            self,
            width * height * 4, // Assuming RGBA8 (4 bytes per pixel)
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &stagingBuffer,
            &stagingBufferMemory
        );
        if (res < 0) return luaL_error(L, "failed to create the transfer buffer");

        void* mappedData;
        vkMapMemory(self->vk.device, stagingBufferMemory, 0, width * height * 4, 0, &mappedData);
        memcpy(mappedData, data, width * height * 4);
        vkUnmapMemory(self->vk.device, stagingBufferMemory);

        transition_image_layout(self, handle, pixel_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copy_buffer_to_image(self, stagingBuffer, handle, width, height);
        transition_image_layout(self, handle, pixel_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(self->vk.device, stagingBuffer, NULL);
        vkFreeMemory(self->vk.device, stagingBufferMemory, NULL);
    } else {
        transition_image_layout(self, handle, pixel_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    NEW_UDATA(Texture2D, texture);
    texture->width = width;
    texture->height = height;
    texture->vk.handle = handle;
    texture->vk.mem = memory;
    return 1;
}

int l_VK_Renderer__destroy_texture(lua_State *L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(Texture2D, tex);
    if (tex->vk.view)
        vkDestroyImageView(self->vk.device, tex->vk.view, NULL);
    if (tex->vk.handle)
        vkDestroyImage(self->vk.device, tex->vk.handle, NULL);
    if (tex->vk.mem)
        vkFreeMemory(self->vk.device, tex->vk.mem, NULL);

    tex->vk.view = VK_NULL_HANDLE;
    tex->vk.handle = VK_NULL_HANDLE;
    tex->vk.mem = VK_NULL_HANDLE;

    return 0;
}

/**
 * Shader
 */

int l_VK_Renderer__create_shader(lua_State *L) {
    CHECK_META(selene_Renderer);
    int opt = luaL_checkoption(L, arg++, "vertex", shader_type_options);
    int size = 0;
    void* data = NULL;
    if (lua_isinteger(L, arg)) {
        size = luaL_checkinteger(L, arg++);
        data = lua_touserdata(L, arg++);
    } else if(lua_isstring(L, arg)) {
        data = (void*)luaL_checkstring(L, arg++);
        size = strlen((const char*)data);
    }
    VkShaderModule handle = NULL;
    VkShaderModuleCreateInfo shader_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = data
    };
  if (vkCreateShaderModule(self->vk.device, &shader_info, NULL, &handle) != VK_SUCCESS) {
    return luaL_error(L, "failed to create Vulkan shader");
  }
  if (handle == NULL) {
        return luaL_error(L, "failed to create Vulkan shader, handle NULL");
  }
  NEW_UDATA(selene_Shader, shader);
  shader->type = opt;
  shader->vk.handle = handle;
  return 1;
}

int l_VK_Renderer__destroy_shader(lua_State *L) {
  CHECK_META(selene_Renderer);
  CHECK_UDATA(selene_Shader, shader);
  if (shader->vk.handle)
    vkDestroyShaderModule(self->vk.device, shader->vk.handle, NULL);
  shader->vk.handle = NULL;
  return 0;
}

/**
 * Draw and Flush
 */

int l_VK_Renderer__flush(lua_State *L) {
    CHECK_META(selene_Renderer);
    struct {
        selene_RenderPipeline *pipe;
        GpuBuffer *vertex;
        GpuBuffer *index;
        GpuBuffer *uniform;
        Texture2D *texture;
        selene_RenderTarget *target;
        int stride;
    } state;
    memset(&state, 0, sizeof(state));
    state.stride = sizeof(Vertex2D);
    state.target = &self->default_target;
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        .pNext = NULL
    };
    if (vkBeginCommandBuffer(self->vk.command_buffer, &begin_info) != VK_SUCCESS) {
        return luaL_error(L, "failed to begin the Vulkan command buffer");
    }
    VkRenderPassBeginInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = self->vk.render_pass,
        .framebuffer = self->vk.framebuffers[self->vk.current_framebuffer],
        .renderArea =
            {
                .offset = {0, 0},
                .extent = self->vk.swapchain.extent,
            },
        .clearValueCount = 1,
        .pClearValues = &(VkClearValue){.color = {0.5, 0.5, 0.5, 1}}};
    vkCmdBeginRenderPass(self->vk.command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    VkViewport view;
    view.x = 0;
    view.y = 0;
    view.width = self->vk.swapchain.extent.width;
    view.height = self->vk.swapchain.extent.height;
    view.minDepth = 0.f;
    view.maxDepth = 1.f;
    VkRect2D scissor;
    scissor.extent = self->vk.swapchain.extent;
    scissor.offset = (VkOffset2D){0, 0};
    vkCmdSetViewport(self->vk.command_buffer, 0, 1, &view);
    vkCmdSetScissor(self->vk.command_buffer, 0, 1, &scissor);
    for (int i = 0; i < self->command_offset; i++) {
        struct RenderCommand *rc = self->command_pool + i;
    switch (rc->type) {
        case RENDER_COMMAND_CLEAR_COLOR: {
            float *c = rc->clear.color;
            // fprintf(stdout, "Clear Color command\n");
            VkClearAttachment clear_attachment = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, // Clear color
                .colorAttachment = 0, // Index of the color attachment to clear
                .clearValue = {.color = {c[0], c[1], c[2], c[3]}},
            };

            VkClearRect clear_rect = {
                .rect =
                    {
                        .offset = {0, 0},
                        .extent = self->vk.swapchain.extent,
                    },
                .baseArrayLayer = 0,
                .layerCount = 1,
            };

            vkCmdClearAttachments(self->vk.command_buffer, 1, &clear_attachment, 1, &clear_rect);
        } break;
        case RENDER_COMMAND_SET_PIPELINE: {
                vkCmdBindPipeline(self->vk.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, rc->pipeline->vk.handle);
        } break; 
        case RENDER_COMMAND_SET_VERTEX_BUFFER: {
            GpuBuffer* buf = rc->buffer.ptr;
            VkBuffer buffers[] = {buf->vk.handle};
            VkDeviceSize offsets[1] = {0};
            vkCmdBindVertexBuffers(self->vk.command_buffer, 0, 1, buffers, offsets);
        } break;
        case RENDER_COMMAND_SET_INDEX_BUFFER: {
            GpuBuffer* buf = rc->buffer.ptr;
            vkCmdBindIndexBuffer(self->vk.command_buffer, buf->vk.handle, 0, VK_INDEX_TYPE_UINT32);
        } break;
        case RENDER_COMMAND_DRAW_VERTEX: {
            vkCmdDraw(self->vk.command_buffer, rc->draw.count, 1, rc->draw.start, 0);
        } break;
        case RENDER_COMMAND_SET_TEXTURE: {
        } break;
        case RENDER_COMMAND_SET_VIEWPORT: {
            VkViewport view;
            view.x = rc->viewport.x;
            view.y = rc->viewport.y;
            view.width = rc->viewport.width;
            view.height = rc->viewport.height;
            vkCmdSetViewport(self->vk.command_buffer, 0, 1, &view);
        } break;
        case RENDER_COMMAND_SET_SCISSOR: {
            VkRect2D scissor;
            if (!rc->scissor.enabled) {
                scissor.extent = self->vk.swapchain.extent;
                scissor.offset = (VkOffset2D){ 0, 0 };
            }
            else {
                scissor.extent.width = rc->scissor.width;
                scissor.extent.height = rc->scissor.height;
                scissor.offset.x = rc->scissor.x;
                scissor.offset.y = rc->scissor.y;
            }
            vkCmdSetScissor(self->vk.command_buffer, 0, 1, &scissor);
        } break;
        default:
            break;
        }
    }
    self->command_offset = 0;
    vkCmdEndRenderPass(self->vk.command_buffer);

    if (vkEndCommandBuffer(self->vk.command_buffer) != VK_SUCCESS) {
        return luaL_error(L, "failed to record command buffer");
    }
    return 0;
}

int l_VK_Renderer__present(lua_State *L) {
    CHECK_META(selene_Renderer);
    vkWaitForFences(self->vk.device, 1, &(self->vk.fence), VK_TRUE, UINT64_MAX);
    vkResetFences(self->vk.device, 1, &(self->vk.fence));

    uint32_t image_index;
    vkAcquireNextImageKHR(self->vk.device, self->vk.swapchain.handle, UINT64_MAX,
                            self->vk.image_semaphore,
                            NULL, &image_index);
    self->vk.current_framebuffer = image_index;
    vkResetCommandBuffer(self->vk.command_buffer, 0);
    l_VK_Renderer__flush(L);
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &self->vk.command_buffer,
        .pNext = NULL
    };

    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.pWaitDstStageMask = wait_stages;
    
    VkSemaphore wait_sem[] = {self->vk.image_semaphore};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_sem;

    VkSemaphore signal_sem[] = {self->vk.render_semaphore};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_sem;

    vkQueueSubmit(self->vk.graphics_queue, 1, &submit_info, self->vk.fence);
    // vkQueueWaitIdle(self->vk.graphics_queue); // Wait for GPU to finish (simplified)

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .swapchainCount = 1,
        .pSwapchains = &self->vk.swapchain.handle,
        .pImageIndices = &self->vk.current_framebuffer,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signal_sem,
        .pNext = NULL
    };
    vkQueuePresentKHR(self->vk.present_queue, &present_info);

  return 0;
}

int l_VK_Renderer_create(lua_State *L) {
    INIT_ARG();
    SDL_Window **win = (SDL_Window **)luaL_checkudata(L, arg++, "sdlWindow");
    int width, height;
    SDL_GetWindowSize(*win, &width, &height);

    VkInstance instance;
    VkApplicationInfo app_info = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                    .pApplicationName = "Vulkan App",
                                    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                                    .pEngineName = "selene",
                                    .engineVersion = VK_MAKE_VERSION(0, 5, 0),
                                    .apiVersion = VK_API_VERSION_1_3
                                };

    unsigned int ext_count;
    if (SDL_Vulkan_GetInstanceExtensions(*win, &ext_count, NULL) != VK_TRUE) {
        return luaL_error(L, "failed to get Vulkan extensions count: %s", SDL_GetError());
    }

    const char **exts = malloc(sizeof(const char *) * ext_count);
    SDL_Vulkan_GetInstanceExtensions(*win, &ext_count, exts);
    const char *validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
    VkInstanceCreateInfo create_info =
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = ext_count,
        .ppEnabledExtensionNames = exts,
        .enabledLayerCount = 1,
        .ppEnabledLayerNames = validation_layers
    };

    if (vkCreateInstance(&create_info, NULL, &instance) != VK_SUCCESS) {
        free(exts);
        return luaL_error(L, "failed to create Vulkan instance");
    }
    free(exts);
    fprintf(stdout, "created Vulkan instance\n");
    VkSurfaceKHR surface;
    if (SDL_Vulkan_CreateSurface(*win, instance, &surface) != SDL_TRUE) {
        vkDestroyInstance(instance, NULL);
        return luaL_error(L, "failed to create Vulkan surface: %s", SDL_GetError());
    }
    fprintf(stdout, "created Vulkan surface\n");
#if 1
    uint32_t dev_count = 0;
    vkEnumeratePhysicalDevices(instance, &dev_count, NULL);
    if (dev_count == 0) {
        vkDestroySurfaceKHR(instance, surface, NULL);
        vkDestroyInstance(instance, NULL);
        return luaL_error(L, "no Vulkan compatible devices found");
    }
    VkPhysicalDevice *devices = malloc(dev_count * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &dev_count, devices);
    VkPhysicalDevice phys = devices[0];
    free(devices);
#endif

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

    const char *device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    
    VkDeviceCreateInfo dev_crt_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = &queue_crt_info,
        .queueCreateInfoCount = 1,
        .pEnabledFeatures = &pdev_features,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = device_extensions,
        .enabledLayerCount = 0
    };

    VkDevice device;
    if (vkCreateDevice(phys, &dev_crt_info, NULL, &device) != VK_SUCCESS) {
        vkDestroySurfaceKHR(instance, surface, NULL);
        vkDestroyInstance(instance, NULL);
        return luaL_error(L, "failed to create Vulkan device");
    }
    fprintf(stdout, "created Vulkan device\n");

    VkQueue gfx_queue, present_queue;
    vkGetDeviceQueue(device, indices.graphics_family, 0, &gfx_queue);
    vkGetDeviceQueue(device, indices.present_family, 0, &present_queue);

    struct VulkanSwapchain swapchain;
    vk_create_swapchain(phys, device, surface, indices, width, height,
                        &swapchain);

    VkRenderPass render_pass;

    VkAttachmentDescription color_attachment = {
        .format = swapchain.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };

    VkAttachmentReference color_attachment_ref = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref,
    };

    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };

    VkRenderPassCreateInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency
    };

    if (vkCreateRenderPass(device, &render_pass_info, NULL, &render_pass) !=
        VK_SUCCESS) {
        return luaL_error(L, "failed to create Vulkan render pass");
    }

    VkCommandPool command_pool;

    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = indices.graphics_family,
    };

    if (vkCreateCommandPool(device, &pool_info, NULL, &command_pool) !=
        VK_SUCCESS) {
        return luaL_error(L, "failed to create Vulkan command pool");
    }
    VkCommandBuffer command_buffer;

    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    if (vkAllocateCommandBuffers(device, &alloc_info, &command_buffer) != VK_SUCCESS) {
        return luaL_error(L, "failed to allocate command buffer");
    }

    VkFramebuffer *swapchain_framebuffers = malloc(swapchain.image_count * sizeof(VkFramebuffer));

    for (uint32_t i = 0; i < swapchain.image_count; i++) {
        VkFramebufferCreateInfo fb_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = render_pass, // Your render pass (must be compatible)
            .attachmentCount = 1,
            .pAttachments = &swapchain.views[i], // Bind the image view
            .width = swapchain.extent.width,
            .height = swapchain.extent.height,
            .layers = 1,
        };

        if (vkCreateFramebuffer(device, &fb_info, NULL, &swapchain_framebuffers[i]) != VK_SUCCESS) {
            return luaL_error(L, "failed to create framebuffer %d", i);
        }
    }


    VkSemaphore image_sem;
    VkSemaphore render_sem;
    VkFence fence;
    VkSemaphoreCreateInfo sem_info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    if (
        vkCreateSemaphore(device, &sem_info, NULL, &image_sem) != VK_SUCCESS ||
        vkCreateSemaphore(device, &sem_info, NULL, &render_sem) != VK_SUCCESS ||
        vkCreateFence(device, &fence_info, NULL, &fence)
    ) {
        return luaL_error(L, "failed to create Vulkan semaphores");
    }

    fprintf(stderr, "sem(%p %p) fence(%p)\n", image_sem, render_sem, fence);

    NEW_UDATA(selene_Renderer, ren);
    memset(ren, 0, sizeof(*ren));
    ren->vk.device = device;
    ren->vk.instance = instance;
    ren->vk.surface = surface;
    ren->vk.phys_device = phys;
    ren->vk.render_pass = render_pass;
    ren->vk.command_pool = command_pool;
    ren->vk.command_buffer = command_buffer;
    ren->vk.framebuffers = swapchain_framebuffers;
    // ren->vk.swap_chain = swap_chain;
    ren->vk.swapchain = swapchain;
    ren->vk.graphics_queue = gfx_queue;
    ren->vk.present_queue = present_queue;

    ren->vk.image_semaphore = image_sem;
    ren->vk.render_semaphore = render_sem;
    ren->vk.fence = fence;

    ren->window_ptr = *win;
    lua_pushvalue(L, 1);
    ren->l_window_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    ren->command_offset = 0;
    ren->command_count = 512;
    ren->command_pool = (struct RenderCommand *)malloc(sizeof(struct RenderCommand) * ren->command_count);

    ren->backend = SELENE_RENDERER_VULKAN;

    ren->destroy = l_VK_Renderer__destroy;
    ren->flush = l_VK_Renderer__flush;
    ren->present = l_VK_Renderer__present;

    ren->create_pipeline = l_VK_Renderer__create_pipeline;
    ren->destroy_pipeline = l_VK_Renderer__destroy_pipeline;

    ren->create_texture2d = l_VK_Renderer__create_texture2d;
    ren->destroy_texture = l_VK_Renderer__destroy_texture;

    ren->create_shader = l_VK_Renderer__create_shader;
    ren->destroy_shader = l_VK_Renderer__destroy_shader;

    ren->create_buffer = l_VK_Renderer__create_buffer;
    ren->destroy_buffer = l_VK_Renderer__destroy_buffer;
    ren->send_buffer_data = l_VK_Renderer__send_buffer_data;

    fprintf(stdout, "Created the Vulkan renderer: instance(%p) device(%p)\n",
            instance, device);

    return 1;
}

void vk_create_swapchain(VkPhysicalDevice physical_device, VkDevice device,
                         VkSurfaceKHR surface,
                         struct QueueFamilyIndices indices, uint32_t width,
                         uint32_t height,
                         struct VulkanSwapchain *outSwapchain) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, NULL);
    VkSurfaceFormatKHR *formats = malloc(format_count * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats);

    VkSurfaceFormatKHR surfaceFormat = formats[0];
    for (uint32_t i = 0; i < format_count; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM &&
            formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        surfaceFormat = formats[i];
        fprintf(stdout, "found surface format in %d\n", i);
        break;
        }
    }
    free(formats);
    fprintf(stdout, "Vulkan surface format: %d %d\n", surfaceFormat.format, surfaceFormat.colorSpace);
#if 0
    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, NULL);
    VkPresentModeKHR* present_modes = malloc(present_mode_count * sizeof(VkPresentModeKHR));
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes);
#endif
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR; // Guaranteed to be supported
#if 0
    for (uint32_t i = 0; i < present_mode_count; i++) {
        if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) { // Triple-buffering
            present_mode = present_modes[i];
            break;
        }
    }
    free(present_modes);
#endif
    VkExtent2D extent;
    if (capabilities.currentExtent.width != UINT32_MAX) {
        extent = capabilities.currentExtent;
    } else {
        extent = (VkExtent2D){.width = (uint32_t)width, .height = (uint32_t)height};
        // Clamp to min/max supported
        extent = (VkExtent2D){
            .width = MAX(capabilities.minImageExtent.width, MIN(capabilities.maxImageExtent.width, extent.width)),
            .height = MAX(capabilities.minImageExtent.height, MIN(capabilities.maxImageExtent.height, extent.height))};
    }

    uint32_t image_count = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount) {
        image_count = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = image_count,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1, // 2+ for stereoscopic 3D
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = capabilities.currentTransform, // No rotation
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_TRUE,            // Ignore obscured pixels
        .oldSwapchain = VK_NULL_HANDLE // Needed for resize
    };

    uint32_t queue_family_indices[] = {indices.graphics_family, indices.present_family};
    if (indices.graphics_family != indices.present_family) {
        fprintf(stdout, "different indices for queue family: %d %d\n", indices.graphics_family, indices.present_family);
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        fprintf(stdout, "same indices for queue family: %d %d\n", indices.graphics_family, indices.present_family);
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkSwapchainKHR swapchain;
    if (vkCreateSwapchainKHR(device, &create_info, NULL, &swapchain) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create swapchain!\n");
        return;
    }
    fprintf(stdout, "created Vulkan swapchain\n");

    // uint32_t image_count;
    // vkGetSwapchainImagesKHR(device, swapchain, &image_count, NULL);
    outSwapchain->images = malloc(image_count * sizeof(VkImage));
    vkGetSwapchainImagesKHR(device, swapchain, &image_count,
                            outSwapchain->images);

    outSwapchain->handle = swapchain;
    outSwapchain->views = malloc(image_count * sizeof(VkImageView));
    if (!outSwapchain->views) {
        fprintf(stderr, "failed to alloc memory for Vulkan image views\n");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "memory allocated for the Vulkan Swapchain image views: %p\n", outSwapchain->views);
    for (uint32_t i = 0; i < image_count; i++) {
        VkImageViewCreateInfo viewInfo = {0};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = outSwapchain->images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = surfaceFormat.format;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkResult result = vkCreateImageView(device, &viewInfo, NULL, &outSwapchain->views[i]);
        if (result != VK_SUCCESS) {
            // Cleanup on failure
            fprintf(stderr, "failed to create Vulkan ImageView\n");
            for (uint32_t j = 0; j < i; j++) {
                vkDestroyImageView(device, outSwapchain->views[j], NULL);
            }
            free(outSwapchain->views);
            free(outSwapchain->images);
            return;
        }
    }

    outSwapchain->format = surfaceFormat.format;
    outSwapchain->extent = extent;
    outSwapchain->image_count = image_count;
}

void vk_destroy_swapchain(VkPhysicalDevice physical_device, VkDevice device,
                          struct VulkanSwapchain swap) {
  for (int i = 0; i < swap.image_count; i++) {
    vkDestroyImageView(device, swap.views[i], NULL);
  }
  vkDestroySwapchainKHR(device, swap.handle, NULL);
  free(swap.views);
  free(swap.images);
}

VkResult createTexture(VkDevice device, VkPhysicalDevice physicalDevice,
                       uint32_t width, uint32_t height, VkFormat format,
                       VkImageTiling tiling, VkImageUsageFlags usage,
                       VkMemoryPropertyFlags properties,
                       Texture2D *outTexture) {
  // Create image
  VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

  VkResult result =
      vkCreateImage(device, &imageInfo, NULL, &outTexture->vk.handle);
  if (result != VK_SUCCESS)
    return result;

  // Allocate memory
  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(device, outTexture->vk.handle, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  allocInfo.allocationSize = memRequirements.size;

  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  uint32_t memoryTypeIndex;
  for (memoryTypeIndex = 0; memoryTypeIndex < memProperties.memoryTypeCount;
       memoryTypeIndex++) {
    if ((memRequirements.memoryTypeBits & (1 << memoryTypeIndex)) &&
        (memProperties.memoryTypes[memoryTypeIndex].propertyFlags &
         properties) == properties) {
      break;
    }
  }

  allocInfo.memoryTypeIndex = memoryTypeIndex;
  result = vkAllocateMemory(device, &allocInfo, NULL, &outTexture->vk.mem);
  if (result != VK_SUCCESS)
    return result;

  vkBindImageMemory(device, outTexture->vk.handle, outTexture->vk.mem, 0);

  // Create image view
  VkImageViewCreateInfo viewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  viewInfo.image = outTexture->vk.handle;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  result = vkCreateImageView(device, &viewInfo, NULL, &outTexture->vk.view);

  outTexture->width = width;
  outTexture->height = height;
  outTexture->format = format;

  return result;
}

int vk_create_buffer(selene_Renderer *self, int size, int usage, int flags,
                     VkBuffer *out_buf, VkDeviceMemory *out_mem) {
  VkBufferCreateInfo buf_info = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                 .size = size,
                                 .usage = usage,
                                 .sharingMode = VK_SHARING_MODE_EXCLUSIVE};
  VkBuffer handle;
  if (vkCreateBuffer(self->vk.device, &buf_info, NULL, &handle) != VK_SUCCESS) {
    fprintf(stderr, "failed to create Vulkan buffer\n");
    return -1;
  }
  VkDeviceMemory buff_memory;
  VkMemoryRequirements mem_reqs;
  vkGetBufferMemoryRequirements(self->vk.device, handle, &mem_reqs);
  VkPhysicalDeviceMemoryProperties mem_properties;
  vkGetPhysicalDeviceMemoryProperties(self->vk.phys_device, &mem_properties);

  VkMemoryAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = mem_reqs.size,
      .memoryTypeIndex =
          find_memory_type(self->vk.phys_device, mem_reqs.memoryTypeBits,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)};

  if (vkAllocateMemory(self->vk.device, &alloc_info, NULL, &buff_memory) !=
      VK_SUCCESS) {
    vkDestroyBuffer(self->vk.device, handle, NULL);
    fprintf(stderr, "failed to alloc memory for Vulkan buffer\n");
    return -1;
  }
  vkBindBufferMemory(self->vk.device, handle, buff_memory, 0);
    if (out_buf) *out_buf = handle;
    if (out_mem) *out_mem = buff_memory;
  return 0;
}

VkCommandBuffer begin_single_time_command(selene_Renderer* self) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = self->vk.command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(self->vk.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void end_single_time_command(selene_Renderer* self, VkCommandBuffer cb) {
    vkEndCommandBuffer(cb);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cb;

    vkQueueSubmit(self->vk.graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(self->vk.graphics_queue);

    vkFreeCommandBuffers(self->vk.device, self->vk.command_pool, 1, &cb);
}

int transition_image_layout(selene_Renderer* self, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer buf = begin_single_time_command(self);
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        fprintf(stderr, "unsupported layout transition!");
        return -1;
    }

    vkCmdPipelineBarrier(
        self->vk.command_buffer,
        sourceStage, destinationStage,
        0,
        0, NULL,
        0, NULL,
        1, &barrier
    );

    end_single_time_command(self, buf);
}

void copy_buffer_to_image(selene_Renderer* self, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = begin_single_time_command(self);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = (VkOffset3D){0, 0, 0};
    region.imageExtent = (VkExtent3D){
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    end_single_time_command(self, commandBuffer);
}
#endif
