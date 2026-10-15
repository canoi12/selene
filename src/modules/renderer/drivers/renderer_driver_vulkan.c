#include "common.h"
#ifndef SELENE_NO_VULKAN
#include "modules/renderer.h"
#include "modules/window.h"

const int vk_pixel_formats_values[] = {
    VK_FORMAT_R8G8B8_UNORM,
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_B8G8R8A8_UNORM
};

const int vk_front_face_values[] = { VK_FRONT_FACE_CLOCKWISE, VK_FRONT_FACE_COUNTER_CLOCKWISE};
const int vk_cull_face_values[] = { VK_CULL_MODE_NONE, VK_CULL_MODE_FRONT_BIT, VK_CULL_MODE_BACK_BIT, VK_CULL_MODE_FRONT_AND_BACK };

const int vk_comparison_funcs[] = { VK_COMPARE_OP_NEVER, VK_COMPARE_OP_LESS, VK_COMPARE_OP_EQUAL, VK_COMPARE_OP_LESS_OR_EQUAL, VK_COMPARE_OP_GREATER, VK_COMPARE_OP_NOT_EQUAL, VK_COMPARE_OP_GREATER_OR_EQUAL, VK_COMPARE_OP_ALWAYS };

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

#define MAX_FRAMES_ 2

static void vk_destroy_framebuffers(selene_Renderer* self);

VkResult vk_create_swapchain(VkPhysicalDevice physical_device, VkDevice device,
                             VkSurfaceKHR surface,
                             struct QueueFamilyIndices indices, uint32_t width,
                             uint32_t height, struct VulkanSwapchain *outSwapchain);
void vk_destroy_swapchain(VkPhysicalDevice physical_device, VkDevice device,
                          struct VulkanSwapchain outSwapchain);
VkResult vk_create_texture(VkDevice device, VkPhysicalDevice physicalDevice,
                           uint32_t width, uint32_t height, VkFormat format,
                           VkImageTiling tiling, VkImageUsageFlags usage,
                           VkMemoryPropertyFlags properties, selene_Texture2D *outTexture);
VkResult vk_create_buffer(selene_Renderer *self, int size, int usage, int flags, VkBuffer *out_buf, VkDeviceMemory *out_mem);
int transition_image_layout(selene_Renderer* self, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
void copy_buffer_to_image(selene_Renderer* self, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

VkImageView vk_create_image_view(selene_Renderer* self, VkImage image, VkFormat format);
VkResult vk_create_sampler(selene_Renderer* self, VkFilter filter, VkSampler *out);

static struct QueueFamilyIndices find_queue_families(VkPhysicalDevice dev, VkSurfaceKHR surface) {
    struct QueueFamilyIndices indices = {-1, -1};

    uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, NULL);

    VkQueueFamilyProperties *families = malloc(sizeof(VkQueueFamilyProperties) * count);
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
    DEBUG_ERROR("No suitable queue family found\n");

RETURN:
    free(families);
    return indices;
}

const int vk_buffer_target_types_values[] = {
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT};

static int find_memory_type(VkPhysicalDevice phys, int type_filter, VkMemoryPropertyFlags properties) {
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
    DEBUG_LOG("Destroying Vulkan renderer\n");
    if (self->vk.device) {
        vkDeviceWaitIdle(self->vk.device);
    }
    if (self->vk.descriptor_pool) vkDestroyDescriptorPool(self->vk.device, self->vk.descriptor_pool, NULL);
    self->vk.descriptor_pool = VK_NULL_HANDLE;

    for (int f = 0; f < SELENE_VK_MAX_FRAMES; f++) {
        if (self->vk.frames[f].upload_ptr)
            vkUnmapMemory(self->vk.device, self->vk.frames[f].upload_mem);
        if (self->vk.frames[f].upload)
            vkDestroyBuffer(self->vk.device, self->vk.frames[f].upload, NULL);
        if (self->vk.frames[f].upload_mem)
            vkFreeMemory(self->vk.device, self->vk.frames[f].upload_mem, NULL);
        if (self->vk.frames[f].fence)
            vkDestroyFence(self->vk.device, self->vk.frames[f].fence, NULL);
        if (self->vk.frames[f].image_acquired)
            vkDestroySemaphore(self->vk.device, self->vk.frames[f].image_acquired, NULL);
        if (self->vk.frames[f].present_sem)
            vkDestroySemaphore(self->vk.device, self->vk.frames[f].present_sem, NULL);
        for (int s = 0; s < SELENE_VK_MAX_SUBMITS; s++) {
            if (self->vk.frames[f].submit_sem[s])
                vkDestroySemaphore(self->vk.device, self->vk.frames[f].submit_sem[s], NULL);
        }
        self->vk.frames[f].upload_ptr = NULL;
        self->vk.frames[f].upload = VK_NULL_HANDLE;
        self->vk.frames[f].fence = VK_NULL_HANDLE;
    }

    vk_destroy_framebuffers(self);
    
    vk_destroy_swapchain(self->vk.phys_device, self->vk.device,
                        self->vk.swapchain);
    
    for (int f = 0; f < SELENE_VK_MAX_FRAMES; f++) {
        int n = 0;
        VkCommandBuffer cbs[SELENE_VK_MAX_SUBMITS];
        for (int s = 0; s < SELENE_VK_MAX_SUBMITS; s++) {
            if (self->vk.frames[f].cmd[s])
                cbs[n++] = self->vk.frames[f].cmd[s];
        }
        if (n)
            vkFreeCommandBuffers(self->vk.device, self->vk.command_pool, (uint32_t)n, cbs);
    }
    if (self->vk.command_pool)
        vkDestroyCommandPool(self->vk.device, self->vk.command_pool, NULL);
    if (self->vk.target_render_pass)
        vkDestroyRenderPass(self->vk.device, self->vk.target_render_pass, NULL);
    if (self->vk.render_pass)
        vkDestroyRenderPass(self->vk.device, self->vk.render_pass, NULL);
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
    self->vk.target_render_pass = VK_NULL_HANDLE;

    self->command_pool = NULL;
    return 0;
}

static void vk_blend_from_mode(int mode, VkBlendFactor* src, VkBlendFactor* dst, VkBlendOp* op) {
    *op = VK_BLEND_OP_ADD;
    switch (mode) {
        case SELENE_BLEND_ADDITIVE:
            *src = VK_BLEND_FACTOR_SRC_ALPHA;
            *dst = VK_BLEND_FACTOR_ONE;
            break;
        case SELENE_BLEND_SUBTRACT:
            *src = VK_BLEND_FACTOR_SRC_ALPHA;
            *dst = VK_BLEND_FACTOR_ONE;
            *op = VK_BLEND_OP_SUBTRACT;
            break;
        case SELENE_BLEND_MULTIPLY:
            *src = VK_BLEND_FACTOR_DST_COLOR;
            *dst = VK_BLEND_FACTOR_ZERO;
            break;
        default:
            *src = VK_BLEND_FACTOR_SRC_ALPHA;
            *dst = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            break;
    }
}

static VkShaderStageFlags vk_stage_flags(int flags) {
    VkShaderStageFlags out = 0;
    if (flags & SELENE_STAGE_VERTEX) out |= VK_SHADER_STAGE_VERTEX_BIT;
    if (flags & SELENE_STAGE_PIXEL) out |= VK_SHADER_STAGE_FRAGMENT_BIT;
    if (flags & SELENE_STAGE_COMPUTE) out |= VK_SHADER_STAGE_COMPUTE_BIT;
    return out ? out : VK_SHADER_STAGE_ALL_GRAPHICS;
}

static void vk_apply_viewport(VkViewport* view, int x, int y, int w, int h) {
    /* Selene: top-left Y-down. Negative height matches GL/D3D NDC. */
    view->x = (float)x;
    view->y = (float)(y + h);
    view->width = (float)w;
    view->height = -(float)h;
    view->minDepth = 0.f;
    view->maxDepth = 1.f;
}

int l_VK_Renderer__create_pipeline(lua_State *L) {
    CHECK_META(selene_Renderer);
    struct selene_PipelineDesc desc;
    renderer_parse_pipeline_desc(L, arg, &desc);
    VkResult result = -1;

    VkPipelineShaderStageCreateInfo shader_stages[2] = {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = desc.vs->vk.handle,
            .pName = "main"
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = desc.ps->vk.handle,
            .pName = "main"
        }
    };

    int len = desc.layout.count;
    VkVertexInputBindingDescription binding_desc = {
        .binding = 0,
        .stride = (uint32_t)desc.layout.vertex_stride,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription attrs_descs[8] = {0};
    for (int i = 0; i < len; i++) {
        VkVertexInputAttributeDescription *attr = attrs_descs + i;
        int size = desc.layout.attributes[i].size;
        int type = desc.layout.attributes[i].type;
        switch (type) {
            case SELENE_FLOAT: {
                if (size == 1) attr->format = VK_FORMAT_R32_SFLOAT;
                else if (size == 2) attr->format = VK_FORMAT_R32G32_SFLOAT;
                else if (size == 3) attr->format = VK_FORMAT_R32G32B32_SFLOAT;
                else attr->format = VK_FORMAT_R32G32B32A32_SFLOAT;
            } break;
            default:
                attr->format = VK_FORMAT_R32_SFLOAT;
                break;
        }
        attr->binding = 0;
        attr->location = (uint32_t)i;
        attr->offset = (uint32_t)desc.layout.attributes[i].offset;
        DEBUG_LOG("name: %s, offset: %d, size: %d, type: %d\n",
                  desc.layout.attributes[i].name,
                  desc.layout.attributes[i].offset, size, type);
    }

    VkPipelineVertexInputStateCreateInfo vert_input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding_desc,
        .vertexAttributeDescriptionCount = (uint32_t)len,
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

    VkPrimitiveTopology vk_topo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    if (desc.topology == SELENE_TOPOLOGY_POINTS) vk_topo = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    else if (desc.topology == SELENE_TOPOLOGY_LINES) vk_topo = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    VkPipelineInputAssemblyStateCreateInfo input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = vk_topo,
        .primitiveRestartEnable = VK_FALSE
    };

    VkPipelineRasterizationStateCreateInfo raster_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .depthBiasEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = (desc.raster.fill_mode == SELENE_POLYGON_LINE)
            ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasConstantFactor = VK_FALSE,
        .lineWidth = 1.f
    };
    if (desc.raster.cull_enabled) {
        raster_info.cullMode = vk_cull_face_values[desc.raster.cull_face];
        raster_info.frontFace = vk_front_face_values[desc.raster.front_face];
    }

    VkPipelineDepthStencilStateCreateInfo depth_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = desc.depth.depth_enabled ? VK_TRUE : VK_FALSE,
        .depthWriteEnable = desc.depth.depth_write_mask ? VK_TRUE : VK_FALSE,
        .depthCompareOp = vk_comparison_funcs[desc.depth.depth_func],
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
    };

    VkPipelineMultisampleStateCreateInfo multisampling_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

    VkBlendFactor src_f = VK_BLEND_FACTOR_SRC_ALPHA;
    VkBlendFactor dst_f = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    VkBlendOp blend_op = VK_BLEND_OP_ADD;
    vk_blend_from_mode(desc.blend.mode, &src_f, &dst_f, &blend_op);
    VkPipelineColorBlendAttachmentState color_blend_attach = {
        .blendEnable = desc.blend.enabled ? VK_TRUE : VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .srcColorBlendFactor = src_f,
        .dstColorBlendFactor = dst_f,
        .srcAlphaBlendFactor = src_f,
        .dstAlphaBlendFactor = dst_f,
        .colorBlendOp = blend_op,
        .alphaBlendOp = blend_op
    };

    VkPipelineColorBlendStateCreateInfo color_blend_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attach,
        .blendConstants = {0.f, 0.f, 0.f, 0.f},
    };

    VkDescriptorSetLayoutBinding descriptor_bindings[SELENE_PIPELINE_MAX_BINDINGS] = {0};
    uint32_t descriptor_count = (uint32_t)desc.binding_count;
    for (uint32_t i = 0; i < descriptor_count; i++) {
        VkDescriptorSetLayoutBinding* binding = descriptor_bindings + i;
        binding->binding = i;
        binding->descriptorCount = desc.bindings[i].count > 0 ? (uint32_t)desc.bindings[i].count : 1;
        binding->stageFlags = vk_stage_flags(desc.bindings[i].stage_flags);
        switch (desc.bindings[i].type) {
            case SELENE_BINDING_UNIFORM:
                /* Dynamic offset selects the ring slice recorded in the command buffer. */
                binding->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                break;
            case SELENE_BINDING_STORAGE:
                binding->descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                break;
            default:
                binding->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                break;
        }
    }

    VkDescriptorSetLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = descriptor_count,
        .pBindings = descriptor_bindings
    };

    VkDescriptorSetLayout descriptor_layout = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkDescriptorSet sets[SELENE_VK_MAX_FRAMES];
    memset(sets, 0, sizeof(sets));
    if (descriptor_count > 0) {
        result = vkCreateDescriptorSetLayout(self->vk.device, &layout_info, NULL, &descriptor_layout);
        if (result != VK_SUCCESS)
            return luaL_error(L, "failed to create descriptor set layout, error: 0x%8x", result);

        VkDescriptorSetLayout layouts[SELENE_VK_MAX_FRAMES];
        for (int fi = 0; fi < SELENE_VK_MAX_FRAMES; fi++)
            layouts[fi] = descriptor_layout;
        VkDescriptorSetAllocateInfo allocInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = self->vk.descriptor_pool,
            .descriptorSetCount = SELENE_VK_MAX_FRAMES,
            .pSetLayouts = layouts
        };
        result = vkAllocateDescriptorSets(self->vk.device, &allocInfo, sets);
        if (result != VK_SUCCESS || sets[0] == VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(self->vk.device, descriptor_layout, NULL);
            return luaL_error(L, "failed to alloc descriptor set, error: 0x%x", result);
        }
        descriptorSet = sets[0];
        for (int fi = 0; fi < SELENE_VK_MAX_FRAMES; fi++) {
            for (uint32_t bi = 0; bi < descriptor_count; bi++) {
                if (desc.bindings[bi].type != SELENE_BINDING_UNIFORM) continue;
                if (!self->vk.frames[fi].upload) continue;
                VkDescriptorBufferInfo binfo = {
                    .buffer = self->vk.frames[fi].upload,
                    .offset = 0,
                    .range = self->vk.ubo_stride ? self->vk.ubo_stride : SELENE_VK_UBO_RANGE,
                };
                VkWriteDescriptorSet write = {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = sets[fi],
                    .dstBinding = bi,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                    .descriptorCount = 1,
                    .pBufferInfo = &binfo,
                };
                vkUpdateDescriptorSets(self->vk.device, 1, &write, 0, NULL);
            }
        }
    } else {
        descriptorSet = VK_NULL_HANDLE;
    }

    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pushConstantRangeCount = 0,
        .setLayoutCount = descriptor_count > 0 ? 1 : 0,
        .pSetLayouts = descriptor_count > 0 ? &descriptor_layout : NULL
    };

    VkPipelineLayout pipeline_layout;
    if (vkCreatePipelineLayout(self->vk.device, &pipeline_layout_info, NULL, &pipeline_layout) != VK_SUCCESS)
        return luaL_error(L, "failed to create Vulkan pipeline layout");

    VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shader_stages,
        .pVertexInputState = &vert_input_info,
        .pInputAssemblyState = &input_info,
        .pRasterizationState = &raster_info,
        .pViewportState = &viewport_info,
        .pMultisampleState = &multisampling_info,
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
    result = vkCreateGraphicsPipelines(self->vk.device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &handle);
    if (result != VK_SUCCESS || handle == VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(self->vk.device, pipeline_layout, NULL);
        if (descriptor_layout) vkDestroyDescriptorSetLayout(self->vk.device, descriptor_layout, NULL);
        return luaL_error(L, "failed to create Vulkan pipeline, error: 0x%8x", result);
    }

    NEW_UDATA(selene_RenderPipeline, pipeline);
    memset(pipeline, 0, sizeof(*pipeline));
    renderer_apply_pipeline_desc(pipeline, &desc);
    pipeline->id = next_pipeline_id++;
    pipeline->vk.handle = handle;
    pipeline->vk.layout = pipeline_layout;
    pipeline->vk.descriptor_layout = descriptor_layout;
    memcpy(pipeline->vk.descriptor_sets, sets, sizeof(sets));
    pipeline->vk.descriptor_set = descriptorSet;
    fprintf(stderr, "pipeline %p, handle: %p\n", (void*)pipeline, (void*)handle);
    return 1;
}

static int l_VK_Renderer__destroy_pipeline(lua_State *L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_RenderPipeline, pipeline);
    vkDeviceWaitIdle(self->vk.device); /* the last submit may still use it */
    if (pipeline->vk.descriptor_layout)
        vkDestroyDescriptorSetLayout(self->vk.device, pipeline->vk.descriptor_layout, NULL);
    if (pipeline->vk.layout)
        vkDestroyPipelineLayout(self->vk.device, pipeline->vk.layout, NULL);
    if (pipeline->vk.handle)
        vkDestroyPipeline(self->vk.device, pipeline->vk.handle, NULL);
    pipeline->vk.descriptor_layout = VK_NULL_HANDLE;
    pipeline->vk.layout = VK_NULL_HANDLE;
    pipeline->vk.handle = VK_NULL_HANDLE;
    return 0;
}

/**
 * Buffers
 */

int l_VK_Renderer__create_buffer(lua_State *L) {
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
        .memoryTypeIndex = find_memory_type(self->vk.phys_device, mem_reqs.memoryTypeBits,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    };

    if (vkAllocateMemory(self->vk.device, &alloc_info, NULL, &buff_memory) !=
        VK_SUCCESS) {
        vkDestroyBuffer(self->vk.device, handle, NULL);
        return luaL_error(L, "failed to alloc memory for Vulkan buffer");
    }
    vkBindBufferMemory(self->vk.device, handle, buff_memory, 0);

    NEW_UDATA(selene_GpuBuffer, buffer);
    buffer->type = opt;
    buffer->size = size;
    buffer->index_type = (opt == SELENE_BUFFER_INDEX) ? SELENE_INDEX_UINT32 : 0;
    buffer->vk.handle = handle;
    buffer->vk.mem = buff_memory;
    return 1;
}

int l_VK_Renderer__destroy_buffer(lua_State *L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_GpuBuffer, buffer);
    vkDeviceWaitIdle(self->vk.device); /* the last submit may still use it */
    if (buffer->vk.handle)
        vkDestroyBuffer(self->vk.device, buffer->vk.handle, NULL);
    if (buffer->vk.mem)
        vkFreeMemory(self->vk.device, buffer->vk.mem, NULL);
    buffer->vk.handle = VK_NULL_HANDLE;
    buffer->vk.mem = VK_NULL_HANDLE;
    return 0;
}

int l_VK_Renderer__send_buffer_data(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_GpuBuffer, buffer);
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
    if (lua_type(L, arg) == LUA_TSTRING)
        data = (void *)lua_tostring(L, arg++);
    else if (lua_isuserdata(L, arg))
        data = lua_touserdata(L, arg++);
    int target = 0;
    if (lua_isboolean(L, arg))
        target = lua_toboolean(L, arg++);
    VkFormat pixel_format = vk_pixel_formats_values[opt]; // Map to VkFormat
    // Render targets are drawn with the swapchain render pass, so the
    // attachment format must match it (VUID-VkFramebufferCreateInfo-pAttachments-00880).
    if (target)
        pixel_format = self->vk.swapchain.format;

    VkImageCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent = {.width = width, .height = height, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = pixel_format,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
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

    VkImageView image_view;
    image_view = vk_create_image_view(self, handle, pixel_format);
    if (image_view == NULL) return luaL_error(L, SDL_GetError());

    VkSampler sampler;
    VkResult result = vk_create_sampler(self, VK_FILTER_NEAREST, &sampler);
    if (result != VK_SUCCESS) {
        return luaL_error(L, SDL_GetError());
    }

    NEW_UDATA(selene_Texture2D, texture);
    texture->width = width;
    texture->height = height;
    texture->vk.handle = handle;
    texture->vk.mem = memory;
    texture->vk.view = image_view;
    texture->vk.sampler = sampler;
#if _DEBUG
    DEBUG_LOG("Created Vulkan image: %p %d %d %p\n", handle, width, height, image_view);
#endif
    return 1;
}

int l_VK_Renderer__destroy_texture(lua_State *L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_Texture2D, tex);
    if (tex->vk.sampler)
        vkDestroySampler(self->vk.device, tex->vk.sampler, NULL);
    if (tex->vk.view)
        vkDestroyImageView(self->vk.device, tex->vk.view, NULL);
    if (tex->vk.handle)
        vkDestroyImage(self->vk.device, tex->vk.handle, NULL);
    if (tex->vk.mem)
        vkFreeMemory(self->vk.device, tex->vk.mem, NULL);

    tex->vk.view = VK_NULL_HANDLE;
    tex->vk.handle = VK_NULL_HANDLE;
    tex->vk.mem = VK_NULL_HANDLE;
    tex->vk.sampler = VK_NULL_HANDLE;

    return 0;
}

/**
 * Render Target
 */

int l_VK_Renderer__create_render_target(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_Texture2D, color);
    TEST_UDATA(selene_Texture2D, depth);

    VkFramebuffer handle = VK_NULL_HANDLE;
    VkRenderPass rp = self->vk.target_render_pass ? self->vk.target_render_pass : self->vk.render_pass;
    VkFramebufferCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = rp,
        .attachmentCount = 1,
        .pAttachments = &color->vk.view,
        .width = color->width,
        .height = color->height,
        .layers = 1
    };
    if (vkCreateFramebuffer(self->vk.device, &info, NULL, &handle) != VK_SUCCESS) {
        return luaL_error(L, "failed to create framebuffer");
    }

    NEW_UDATA(selene_RenderTarget, target);
    memset(target, 0, sizeof(*target));
    target->color = color;
    target->depth = depth;
    target->width = color->width;
    target->height = color->height;
    target->vk.handle = handle;

    return 1;
}

int l_VK_Renderer__destroy_render_target(lua_State* L) {
    CHECK_META(selene_Renderer);
    CHECK_UDATA(selene_RenderTarget, target);
    if (target->vk.handle) {
        vkDestroyFramebuffer(self->vk.device, target->vk.handle, NULL);
        target->vk.handle = VK_NULL_HANDLE;
    }
    return 0;
}

/**
 * Shader
 */

int l_VK_Renderer__create_shader(lua_State *L) {
    CHECK_META(selene_Renderer);
    int opt = luaL_checkoption(L, arg++, "vertex", shader_type_options);
    size_t size = 0;
    void* data = NULL;
    if (lua_isinteger(L, arg)) {
        size = luaL_checkinteger(L, arg++);
        data = lua_touserdata(L, arg++);
    } else if(lua_isstring(L, arg)) {
        data = (void*)luaL_checklstring(L, arg++, &size);
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
    shader->vk.handle = VK_NULL_HANDLE;
    return 0;
}

/**
 * Draw and Flush
 */

static void vk_write_texture_descriptor(selene_Renderer* self, selene_RenderPipeline* pipe, int slot, selene_Texture2D* tex) {
    if (!pipe || !tex) return;
    // Additional check for valid device and descriptor set
    if (!self || !self->vk.device) return;
    VkDescriptorSet dset = pipe->vk.descriptor_sets[self->vk.frame_index];
    if (!dset) dset = pipe->vk.descriptor_set;
    if (!dset) return;
    
    // Don't try to update descriptors if there are no bindings
    if (pipe->binding_count <= 0) return;
    
    int binding = renderer_pipeline_binding_index(pipe, SELENE_BINDING_TEXTURE, slot);
    // The pipeline layout may not declare a texture binding (e.g. primitive2d):
    // never write a combined image sampler over another descriptor type.
    if (binding < 0 || binding >= pipe->binding_count ||
        pipe->bindings[binding].type != SELENE_BINDING_TEXTURE) return;
    
    // Validate texture components before updating descriptors
    if (tex->vk.view == VK_NULL_HANDLE || tex->vk.sampler == VK_NULL_HANDLE) {
        return;
    }
    
    VkDescriptorImageInfo image_info = {
        .imageView = tex->vk.view,
        .sampler = tex->vk.sampler,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    };
    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = dset,
        .dstBinding = (uint32_t)binding,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &image_info,
        .descriptorCount = 1,
        .dstArrayElement = 0,
    };
    vkUpdateDescriptorSets(self->vk.device, 1, &write, 0, NULL);
}

static void vk_write_uniform_descriptor(selene_Renderer* self, selene_RenderPipeline* pipe, int slot, selene_GpuBuffer* buf) {
    /* Uniforms are DYNAMIC descriptors aimed at the frame upload ring.
     * The set is written once at pipeline creation; the command buffer carries the offset. */
    (void)self; (void)pipe; (void)slot; (void)buf;
}

/* Descriptor sets are host-written before recording. The plugin may bind
 * textures/uniforms before set_pipeline, so remember the last resource per
 * slot and re-apply it whenever the pipeline changes. */
static void vk_update_descriptors_from_commands(selene_Renderer* self) {
    selene_RenderPipeline* pipe = NULL;
    selene_Texture2D* textures[SELENE_PIPELINE_MAX_BINDINGS] = {0};
    selene_GpuBuffer* uniforms[SELENE_PIPELINE_MAX_BINDINGS] = {0};
    for (int i = self->commands_submitted; i < self->command_offset; i++) {
        struct RenderCommand* rc = self->command_pool + i;
        if (rc->type == RENDER_COMMAND_SET_PIPELINE) {
            pipe = rc->pipeline;
            if (!pipe) continue;
            for (int b = 0; b < SELENE_PIPELINE_MAX_BINDINGS; b++) {
                if (textures[b]) vk_write_texture_descriptor(self, pipe, b, textures[b]);
                if (uniforms[b]) vk_write_uniform_descriptor(self, pipe, b, uniforms[b]);
            }
        } else if (rc->type == RENDER_COMMAND_SET_TEXTURE) {
            int slot = rc->texture.slot;
            if (slot < 0 || slot >= SELENE_PIPELINE_MAX_BINDINGS) slot = 0;
            textures[slot] = (selene_Texture2D*)rc->texture.ptr;
            vk_write_texture_descriptor(self, pipe, slot, textures[slot]);
        } else if (rc->type == RENDER_COMMAND_SET_UNIFORM_BUFFER) {
            int slot = rc->buffer.slot;
            if (slot < 0 || slot >= SELENE_PIPELINE_MAX_BINDINGS) slot = 0;
            uniforms[slot] = rc->buffer.ptr;
            vk_write_uniform_descriptor(self, pipe, slot, uniforms[slot]);
        }
    }
}

int l_VK_Renderer__begin_frame(lua_State *L);
int l_VK_Renderer__end_frame(lua_State *L);

/* Copy host-visible buffer bytes into the current frame's upload ring.
 * The command buffer binds the copy, so the plugin can overwrite the original
 * before this submit finishes. */
static int vk_snapshot(selene_Renderer* self, selene_GpuBuffer* buf,
                       VkDeviceSize bytes, VkDeviceSize align,
                       VkBuffer* out_buf, VkDeviceSize* out_off) {
    if (!buf || !buf->vk.mem || buf->size <= 0) return -1;
    struct VulkanFrame* fr = &self->vk.frames[self->vk.frame_index];
    if (!fr->upload_ptr) return -1;
    if (align < 1) align = 1;
    VkDeviceSize off = (fr->upload_used + align - 1) & ~(align - 1);
    if (bytes > SELENE_VK_UPLOAD_CAPACITY || off + bytes > SELENE_VK_UPLOAD_CAPACITY)
        return -1;
    void* src = NULL;
    if (vkMapMemory(self->vk.device, buf->vk.mem, 0, (VkDeviceSize)buf->size, 0, &src) != VK_SUCCESS)
        return -1;
    memset((char*)fr->upload_ptr + off, 0, (size_t)bytes);
    VkDeviceSize n = bytes < (VkDeviceSize)buf->size ? bytes : (VkDeviceSize)buf->size;
    memcpy((char*)fr->upload_ptr + off, src, (size_t)n);
    vkUnmapMemory(self->vk.device, buf->vk.mem);
    fr->upload_used = off + bytes;
    *out_buf = fr->upload;
    *out_off = off;
    return 0;
}

static void vk_bind_pipe_sets(selene_Renderer* self, selene_RenderPipeline* pipe, const uint32_t* ubo_dyn) {
    if (!pipe) return;
    VkDescriptorSet set = pipe->vk.descriptor_sets[self->vk.frame_index];
    if (!set) set = pipe->vk.descriptor_set;
    if (!set) return;
    uint32_t dyn[SELENE_PIPELINE_MAX_BINDINGS];
    uint32_t n = 0;
    for (int i = 0; i < pipe->binding_count; i++) {
        if (pipe->bindings[i].type != SELENE_BINDING_UNIFORM) continue;
        int slot = pipe->bindings[i].slot;
        if (slot < 0 || slot >= SELENE_PIPELINE_MAX_BINDINGS) slot = 0;
        dyn[n++] = ubo_dyn ? ubo_dyn[slot] : 0;
    }
    vkCmdBindDescriptorSets(self->vk.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipe->vk.layout, 0, 1, &set, n, n ? dyn : NULL);
}

/* Set while recording a flush: this command buffer renders to the swapchain. */
static int s_recording_swapchain;

static VkExtent2D vk_pass_extent(selene_Renderer* self, selene_RenderTarget* target) {
    if (target && target->width > 0 && target->height > 0)
        return (VkExtent2D){ (uint32_t)target->width, (uint32_t)target->height };
    return self->vk.swapchain.extent;
}

/* Offscreen images are sampled in a later pass. The swapchain pass ends in
 * PRESENT, which cannot be sampled, so targets use their own render pass. */
static void vk_end_draw_pass(selene_Renderer* self, int* open, selene_RenderTarget* target) {
    if (!*open) return;
    vkCmdEndRenderPass(self->vk.command_buffer);
    *open = 0;
    /* finalLayout is already shader-read; this barrier publishes the color
     * writes to the fragment shader that samples the canvas next. */
    if (!target || !target->color || !target->color->vk.handle) return;
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = target->color->vk.handle,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
    };
    vkCmdPipelineBarrier(self->vk.command_buffer,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, NULL, 0, NULL, 1, &barrier);
}

static void vk_open_draw_pass(selene_Renderer* self, selene_RenderTarget* target,
                              int* open, selene_RenderTarget** current, const float* clear_color) {
    if (*open && *current == target) return;
    vk_end_draw_pass(self, open, *current);

    int offscreen = target && target->vk.handle;
    VkRenderPass rp = offscreen && self->vk.target_render_pass
        ? self->vk.target_render_pass : self->vk.render_pass;
    VkFramebuffer fb = offscreen
        ? target->vk.handle
        : self->vk.framebuffers[self->vk.current_framebuffer];
    VkExtent2D extent = vk_pass_extent(self, offscreen ? target : NULL);
    float rgba[4] = {0.f, 0.f, 0.f, 1.f};
    if (clear_color) {
        rgba[0] = clear_color[0];
        rgba[1] = clear_color[1];
        rgba[2] = clear_color[2];
        rgba[3] = clear_color[3];
    }
    VkClearValue clear_value = {.color = {{rgba[0], rgba[1], rgba[2], rgba[3]}}};
    VkRenderPassBeginInfo info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = rp,
        .framebuffer = fb,
        .renderArea = {.offset = {0, 0}, .extent = extent},
        .clearValueCount = 1,
        .pClearValues = &clear_value,
    };
    vkCmdBeginRenderPass(self->vk.command_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    if (!offscreen) s_recording_swapchain = 1;
    VkViewport view;
    vk_apply_viewport(&view, 0, 0, (int)extent.width, (int)extent.height);
    VkRect2D scissor = {.offset = {0, 0}, .extent = extent};
    vkCmdSetViewport(self->vk.command_buffer, 0, 1, &view);
    vkCmdSetScissor(self->vk.command_buffer, 0, 1, &scissor);
    *open = 1;
    *current = target;
}

int l_VK_Renderer__flush(lua_State *L) {
    CHECK_META(selene_Renderer);
    // DEBUG_LOG("Render flush\n");
    struct {
        selene_RenderPipeline *pipe;
        selene_GpuBuffer *vertex;
        selene_GpuBuffer *index;
        selene_GpuBuffer *uniform;
        selene_Texture2D *texture;
        selene_RenderTarget *target;
        int stride;
    } state;
    memset(&state, 0, sizeof(state));
    state.stride = sizeof(Vertex2D);
    state.target = &self->default_target;
    uint32_t ubo_dyn[SELENE_PIPELINE_MAX_BINDINGS];
    memset(ubo_dyn, 0, sizeof(ubo_dyn));

    struct VulkanFrame* fr = &self->vk.frames[self->vk.frame_index];
    if (!fr->cmd[0]) {
        return luaL_error(L, "Vulkan command buffer not initialized");
    }

    /* Each flush submits only the commands added since the previous one.
     * The plugin overwrites the vertex buffer every pass, so re-recording the
     * whole frame at present time would replay earlier draws with the wrong
     * vertices. */
    if (!self->in_frame) {
        int rec = l_VK_Renderer__begin_frame(L);
        if (rec != 0) return rec;
    }
    if (self->commands_submitted >= self->command_offset)
        return 0;

    if (fr->submit_count >= SELENE_VK_MAX_SUBMITS)
        return luaL_error(L, "too many Vulkan submits in one frame");
    self->vk.command_buffer = fr->cmd[fr->submit_count];

    vk_update_descriptors_from_commands(self);
    s_recording_swapchain = 0;

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pNext = NULL
    };
    if (vkBeginCommandBuffer(self->vk.command_buffer, &begin_info) != VK_SUCCESS) {
        return luaL_error(L, "failed to begin the Vulkan command buffer");
    }

    if (self->vk.current_framebuffer >= self->vk.swapchain.image_count ||
        !self->vk.framebuffers || !self->vk.framebuffers[self->vk.current_framebuffer]) {
        vkEndCommandBuffer(self->vk.command_buffer);
        return luaL_error(L, "invalid framebuffer");
    }

    /* Passes may target the swapchain or an offscreen framebuffer. The pass
     * opens on the first clear/draw and closes on end_pass or a target change
     * so the canvas is in shader-read before a later pass samples it. */
    int pass_open = 0;
    selene_RenderTarget* draw_target = NULL;
    for (int i = self->commands_submitted; i < self->command_offset; i++) {
        struct RenderCommand *rc = self->command_pool + i;
    switch (rc->type) {
        case RENDER_COMMAND_BEGIN_PASS: {
            if (rc->pass.target != draw_target) {
                vk_end_draw_pass(self, &pass_open, draw_target);
                draw_target = rc->pass.target;
            }
            vk_open_draw_pass(self, draw_target, &pass_open, &draw_target,
                              rc->pass.clear_color ? rc->pass.color : NULL);
            if (rc->pass.has_viewport) {
                VkViewport pass_view;
                vk_apply_viewport(&pass_view,
                                  rc->pass.viewport.x, rc->pass.viewport.y,
                                  rc->pass.viewport.width, rc->pass.viewport.height);
                vkCmdSetViewport(self->vk.command_buffer, 0, 1, &pass_view);
            }
            if (rc->pass.has_scissor) {
                VkRect2D pass_scissor;
                if (!rc->pass.scissor.enabled) {
                    pass_scissor.offset = (VkOffset2D){0, 0};
                    pass_scissor.extent = vk_pass_extent(self, draw_target);
                } else {
                    pass_scissor.offset.x = rc->pass.scissor.x;
                    pass_scissor.offset.y = rc->pass.scissor.y;
                    pass_scissor.extent.width = (uint32_t)rc->pass.scissor.width;
                    pass_scissor.extent.height = (uint32_t)rc->pass.scissor.height;
                }
                vkCmdSetScissor(self->vk.command_buffer, 0, 1, &pass_scissor);
            }
            if (rc->pass.clear_color) {
                float *c = rc->pass.color;
                VkClearAttachment clear_attachment = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .colorAttachment = 0,
                    .clearValue = {.color = {{c[0], c[1], c[2], c[3]}}},
                };
                VkClearRect clear_rect = {
                    .rect = {.offset = {0, 0}, .extent = vk_pass_extent(self, draw_target)},
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                };
                vkCmdClearAttachments(self->vk.command_buffer, 1, &clear_attachment, 1, &clear_rect);
            }
        } break;
        case RENDER_COMMAND_END_PASS:
            vk_end_draw_pass(self, &pass_open, draw_target);
            break;
        case RENDER_COMMAND_CLEAR_COLOR: {
            vk_open_draw_pass(self, draw_target, &pass_open, &draw_target, rc->clear.color);
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
                        .extent = vk_pass_extent(self, draw_target),
                    },
                .baseArrayLayer = 0,
                .layerCount = 1,
            };

            vkCmdClearAttachments(self->vk.command_buffer, 1, &clear_attachment, 1, &clear_rect);
        } break;
        case RENDER_COMMAND_SET_PIPELINE: {
            if (!rc->pipeline) break;
            if (rc->pipeline->vk.handle) {
                vkCmdBindPipeline(self->vk.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, rc->pipeline->vk.handle);
                vk_bind_pipe_sets(self, rc->pipeline, ubo_dyn);
            }
            state.pipe = rc->pipeline;
        } break; 
        case RENDER_COMMAND_SET_VERTEX_BUFFER: {
            selene_GpuBuffer* buf = rc->buffer.ptr;
            if (buf && buf->vk.handle) {
                VkBuffer copy;
                VkDeviceSize copy_off = 0;
                if (vk_snapshot(self, buf, (VkDeviceSize)buf->size, 16, &copy, &copy_off) != 0)
                    return luaL_error(L, "Vulkan upload ring is full");
                VkBuffer buffers[] = {copy};
                VkDeviceSize offsets[1] = {copy_off + (VkDeviceSize)rc->buffer.offset};
                vkCmdBindVertexBuffers(self->vk.command_buffer, (uint32_t)rc->buffer.slot, 1, buffers, offsets);
            }
        } break;
        case RENDER_COMMAND_SET_INDEX_BUFFER: {
            selene_GpuBuffer* buf = rc->buffer.ptr;
            if (buf && buf->vk.handle) {
                VkBuffer copy;
                VkDeviceSize copy_off = 0;
                if (vk_snapshot(self, buf, (VkDeviceSize)buf->size, 16, &copy, &copy_off) != 0)
                    return luaL_error(L, "Vulkan upload ring is full");
                VkIndexType itype = (buf->index_type == SELENE_INDEX_UINT16)
                    ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
                vkCmdBindIndexBuffer(self->vk.command_buffer, copy,
                                     copy_off + (VkDeviceSize)rc->buffer.offset, itype);
            }
        } break;
        case RENDER_COMMAND_DRAW: {
            vk_open_draw_pass(self, draw_target, &pass_open, &draw_target, NULL);
            uint32_t instances = rc->draw.instance_count < 1 ? 1u : (uint32_t)rc->draw.instance_count;
            vkCmdDraw(self->vk.command_buffer, (uint32_t)rc->draw.count, instances, (uint32_t)rc->draw.start, 0);
        } break;
        case RENDER_COMMAND_DRAW_INDEXED: {
            vk_open_draw_pass(self, draw_target, &pass_open, &draw_target, NULL);
            uint32_t instances = rc->draw.instance_count < 1 ? 1u : (uint32_t)rc->draw.instance_count;
            vkCmdDrawIndexed(self->vk.command_buffer, (uint32_t)rc->draw.count, instances,
                             (uint32_t)rc->draw.start, rc->draw.base_vertex, 0);
        } break;
        case RENDER_COMMAND_SET_TEXTURE: {
            state.texture = (selene_Texture2D*)rc->texture.ptr;
            if (state.pipe)
                vk_bind_pipe_sets(self, state.pipe, ubo_dyn);
        } break;
        case RENDER_COMMAND_UPDATE_UNIFORM_BUFFER: {
            selene_GpuBuffer* b = rc->uniform.buffer;
            if (b && b->vk.mem && rc->uniform.size > 0 && rc->uniform.offset >= 0 &&
                rc->uniform.offset < b->size) {
                int size = rc->uniform.size;
                if (rc->uniform.offset + size > b->size)
                    size = b->size - rc->uniform.offset;
                void* out = NULL;
                if (vkMapMemory(self->vk.device, b->vk.mem, (VkDeviceSize)rc->uniform.offset,
                                (VkDeviceSize)size, 0, &out) == VK_SUCCESS) {
                    memcpy(out, &rc->uniform.frame, (size_t)size);
                    vkUnmapMemory(self->vk.device, b->vk.mem);
                }
            }
        } break;
        case RENDER_COMMAND_SET_UNIFORM_BUFFER: {
            selene_GpuBuffer* ubuf = rc->buffer.ptr;
            state.uniform = ubuf;
            int slot = rc->buffer.slot;
            if (slot < 0 || slot >= SELENE_PIPELINE_MAX_BINDINGS) slot = 0;
            if (ubuf && ubuf->vk.mem) {
                VkBuffer copy;
                VkDeviceSize copy_off = 0;
                VkDeviceSize stride = self->vk.ubo_stride ? self->vk.ubo_stride : SELENE_VK_UBO_RANGE;
                if (vk_snapshot(self, ubuf, stride, self->vk.uniform_align ? self->vk.uniform_align : 256,
                                &copy, &copy_off) != 0)
                    return luaL_error(L, "Vulkan upload ring is full");
                ubo_dyn[slot] = (uint32_t)copy_off;
            }
            if (state.pipe)
                vk_bind_pipe_sets(self, state.pipe, ubo_dyn);
        } break;
        case RENDER_COMMAND_SET_RENDER_TARGET: {
            if (rc->target.ptr != draw_target) {
                vk_end_draw_pass(self, &pass_open, draw_target);
                draw_target = rc->target.ptr;
            }
        } break;
        case RENDER_COMMAND_SET_VIEWPORT: {
            if (!pass_open) break;
            VkViewport view;
            vk_apply_viewport(&view, rc->viewport.x, rc->viewport.y,
                              rc->viewport.width, rc->viewport.height);
            vkCmdSetViewport(self->vk.command_buffer, 0, 1, &view);
        } break;
        case RENDER_COMMAND_SET_SCISSOR: {
            if (!pass_open) break;
            VkRect2D scissor_rect;
            if (!rc->scissor.enabled) {
                scissor_rect.extent = vk_pass_extent(self, draw_target);
                scissor_rect.offset = (VkOffset2D){ 0, 0 };
            }
            else {
                scissor_rect.extent.width = (uint32_t)rc->scissor.width;
                scissor_rect.extent.height = (uint32_t)rc->scissor.height;
                scissor_rect.offset.x = rc->scissor.x;
                scissor_rect.offset.y = rc->scissor.y;
            }
            vkCmdSetScissor(self->vk.command_buffer, 0, 1, &scissor_rect);
        } break;
        default:
            break;
        }
    }
    vk_end_draw_pass(self, &pass_open, draw_target);

    if (vkEndCommandBuffer(self->vk.command_buffer) != VK_SUCCESS) {
        return luaL_error(L, "failed to record command buffer");
    }

    VkSemaphore waits[2];
    VkPipelineStageFlags stages[2];
    uint32_t nwait = 0;
    if (s_recording_swapchain && fr->image_sem_pending) {
        waits[nwait] = fr->image_acquired;
        stages[nwait] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        nwait++;
        fr->image_sem_pending = 0;
    }
    if (fr->submit_count > 0) {
        waits[nwait] = fr->submit_sem[fr->submit_count - 1];
        stages[nwait] = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
                      | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        nwait++;
    }
    VkSemaphore signal = fr->submit_sem[fr->submit_count];
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = nwait,
        .pWaitSemaphores = nwait ? waits : NULL,
        .pWaitDstStageMask = nwait ? stages : NULL,
        .commandBufferCount = 1,
        .pCommandBuffers = &self->vk.command_buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &signal,
    };
    /* Fence stays NULL: several passes submit before the frame ends, and a
     * fence can only be attached to one of them. end_frame signals it. */
    VkResult res = vkQueueSubmit(self->vk.graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    if (res != VK_SUCCESS)
        return luaL_error(L, "failed to submit Vulkan queue: 0x%x", res);
    fr->submit_count++;
    self->commands_submitted = self->command_offset;
    return 0;
}

static void vk_destroy_framebuffers(selene_Renderer* self) {
    if (self->vk.framebuffers) {
        for (uint32_t i = 0; i < self->vk.swapchain.image_count; i++) {
            if (self->vk.framebuffers[i])
                vkDestroyFramebuffer(self->vk.device, self->vk.framebuffers[i], NULL);
        }
        free(self->vk.framebuffers);
        self->vk.framebuffers = NULL;
    }
    free(self->vk.images_in_flight);
    self->vk.images_in_flight = NULL;
}

static int vk_make_framebuffers(selene_Renderer* self) {
    uint32_t n = self->vk.swapchain.image_count;
    VkFramebuffer* fbs = calloc(n, sizeof(VkFramebuffer));
    VkFence* inflight = calloc(n, sizeof(VkFence));
    if (!fbs || !inflight) {
        free(fbs);
        free(inflight);
        return -1;
    }
    for (uint32_t i = 0; i < n; i++) {
        VkFramebufferCreateInfo fb_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = self->vk.render_pass,
            .attachmentCount = 1,
            .pAttachments = &self->vk.swapchain.views[i],
            .width = self->vk.swapchain.extent.width,
            .height = self->vk.swapchain.extent.height,
            .layers = 1,
        };
        if (vkCreateFramebuffer(self->vk.device, &fb_info, NULL, &fbs[i]) != VK_SUCCESS) {
            for (uint32_t j = 0; j < i; j++)
                vkDestroyFramebuffer(self->vk.device, fbs[j], NULL);
            free(fbs);
            free(inflight);
            return -1;
        }
    }
    self->vk.framebuffers = fbs;
    self->vk.images_in_flight = inflight;
    return 0;
}

static int vk_init_frames(selene_Renderer* self) {
    VkSemaphoreCreateInfo sem_info = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };
    for (int f = 0; f < SELENE_VK_MAX_FRAMES; f++) {
        struct VulkanFrame* fr = &self->vk.frames[f];
        VkCommandBufferAllocateInfo alloc_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = self->vk.command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = SELENE_VK_MAX_SUBMITS,
        };
        if (vkAllocateCommandBuffers(self->vk.device, &alloc_info, fr->cmd) != VK_SUCCESS)
            return -1;
        if (vkCreateFence(self->vk.device, &fence_info, NULL, &fr->fence) != VK_SUCCESS)
            return -1;
        if (vkCreateSemaphore(self->vk.device, &sem_info, NULL, &fr->image_acquired) != VK_SUCCESS)
            return -1;
        if (vkCreateSemaphore(self->vk.device, &sem_info, NULL, &fr->present_sem) != VK_SUCCESS)
            return -1;
        for (int s = 0; s < SELENE_VK_MAX_SUBMITS; s++) {
            if (vkCreateSemaphore(self->vk.device, &sem_info, NULL, &fr->submit_sem[s]) != VK_SUCCESS)
                return -1;
        }

        VkBufferCreateInfo buf_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = SELENE_VK_UPLOAD_CAPACITY,
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        if (vkCreateBuffer(self->vk.device, &buf_info, NULL, &fr->upload) != VK_SUCCESS)
            return -1;
        VkMemoryRequirements reqs;
        vkGetBufferMemoryRequirements(self->vk.device, fr->upload, &reqs);
        VkMemoryAllocateInfo alloc = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = reqs.size,
            .memoryTypeIndex = (uint32_t)find_memory_type(self->vk.phys_device, (int)reqs.memoryTypeBits,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
        };
        if (vkAllocateMemory(self->vk.device, &alloc, NULL, &fr->upload_mem) != VK_SUCCESS)
            return -1;
        vkBindBufferMemory(self->vk.device, fr->upload, fr->upload_mem, 0);
        if (vkMapMemory(self->vk.device, fr->upload_mem, 0, SELENE_VK_UPLOAD_CAPACITY, 0, &fr->upload_ptr) != VK_SUCCESS)
            return -1;
    }
    return 0;
}

static int vk_recreate_swapchain(selene_Renderer* self) {
    vkDeviceWaitIdle(self->vk.device);
    VkFormat old_format = self->vk.swapchain.format;
    vk_destroy_framebuffers(self);
    vk_destroy_swapchain(self->vk.phys_device, self->vk.device, self->vk.swapchain);
    memset(&self->vk.swapchain, 0, sizeof(self->vk.swapchain));

    int width = self->width, height = self->height;
    if (self->window_ptr && self->window_ptr->handle)
        SDL_GetWindowSize(self->window_ptr->handle, &width, &height);
    struct QueueFamilyIndices indices = {
        .graphics_family = self->vk.graphics_family,
        .present_family = self->vk.present_family,
    };
    if (vk_create_swapchain(self->vk.phys_device, self->vk.device, self->vk.surface,
                            indices, (uint32_t)width, (uint32_t)height, &self->vk.swapchain) < 0)
        return -1;
    if (old_format != VK_FORMAT_UNDEFINED && self->vk.swapchain.format != old_format)
        return -2;
    if (vk_make_framebuffers(self) != 0)
        return -1;
    self->width = width;
    self->height = height;
    self->vk.swapchain_dirty = 0;
    return 0;
}

static int vk_close_frame(selene_Renderer* self) {
    struct VulkanFrame* fr = &self->vk.frames[self->vk.frame_index];
    if (fr->closed) return 0;

    VkSemaphore waits[1];
    VkPipelineStageFlags stages[1];
    uint32_t nwait = 0;
    if (fr->submit_count > 0) {
        waits[0] = fr->submit_sem[fr->submit_count - 1];
        stages[0] = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        nwait = 1;
    } else if (fr->image_sem_pending) {
        waits[0] = fr->image_acquired;
        stages[0] = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
        nwait = 1;
        fr->image_sem_pending = 0;
    }
    VkSemaphore signal = fr->present_sem;
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = nwait,
        .pWaitSemaphores = nwait ? waits : NULL,
        .pWaitDstStageMask = nwait ? stages : NULL,
        .commandBufferCount = 0,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &signal,
    };
    VkResult res = vkQueueSubmit(self->vk.graphics_queue, 1, &submit_info, fr->fence);
    if (res != VK_SUCCESS) return -1;
    fr->closed = 1;
    return 0;
}

int l_VK_Renderer__begin_frame(lua_State *L) {
    CHECK_META(selene_Renderer);
    if (self->in_frame) return 0;

    if (self->vk.swapchain_dirty) {
        int rc = vk_recreate_swapchain(self);
        if (rc == -2)
            return luaL_error(L, "Vulkan swapchain format changed");
        if (rc != 0)
            return luaL_error(L, "failed to recreate Vulkan swapchain");
    }

    struct VulkanFrame* fr = &self->vk.frames[self->vk.frame_index];
    if (fr->fence) {
        vkWaitForFences(self->vk.device, 1, &fr->fence, VK_TRUE, UINT64_MAX);
        vkResetFences(self->vk.device, 1, &fr->fence);
    }
    fr->upload_used = 0;
    fr->submit_count = 0;
    fr->closed = 0;
    fr->image_sem_pending = 0;
    for (int i = 0; i < SELENE_VK_MAX_SUBMITS; i++) {
        if (fr->cmd[i])
            vkResetCommandBuffer(fr->cmd[i], 0);
    }

    uint32_t image_index = 0;
    VkResult res = vkAcquireNextImageKHR(self->vk.device, self->vk.swapchain.handle, UINT64_MAX,
                                         fr->image_acquired, VK_NULL_HANDLE, &image_index);
    if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        self->vk.swapchain_dirty = 1;
        int rc = vk_recreate_swapchain(self);
        if (rc == -2)
            return luaL_error(L, "Vulkan swapchain format changed");
        if (rc != 0)
            return luaL_error(L, "failed to recreate Vulkan swapchain");
        res = vkAcquireNextImageKHR(self->vk.device, self->vk.swapchain.handle, UINT64_MAX,
                                    fr->image_acquired, VK_NULL_HANDLE, &image_index);
    }
    if (res == VK_SUBOPTIMAL_KHR)
        self->vk.swapchain_dirty = 1;
    if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
        return luaL_error(L, "failed to acquire Vulkan swapchain image: 0x%x", res);

    if (self->vk.images_in_flight && self->vk.images_in_flight[image_index]
        && self->vk.images_in_flight[image_index] != fr->fence) {
        vkWaitForFences(self->vk.device, 1, &self->vk.images_in_flight[image_index], VK_TRUE, UINT64_MAX);
    }
    if (self->vk.images_in_flight)
        self->vk.images_in_flight[image_index] = fr->fence;

    self->vk.current_framebuffer = (int)image_index;
    fr->image_sem_pending = 1;
    self->in_frame = 1;
    self->commands_submitted = 0;
    return 0;
}

int l_VK_Renderer__end_frame(lua_State *L) {
    CHECK_META(selene_Renderer);
    if (!self->in_frame) return 0;

    int rec = l_VK_Renderer__flush(L);
    if (rec != 0) return rec;
    if (vk_close_frame(self) != 0)
        return luaL_error(L, "failed to close Vulkan frame");

    self->in_frame = 0;
    self->in_pass = 0;
    self->command_offset = 0;
    self->commands_submitted = 0;
    return 0;
}

int l_VK_Renderer__present(lua_State *L) {
    CHECK_META(selene_Renderer);
    if (self->in_frame || self->command_offset > 0) {
        if (!self->in_frame) {
            int rec = l_VK_Renderer__begin_frame(L);
            if (rec != 0) return rec;
        }
        int rec = l_VK_Renderer__end_frame(L);
        if (rec != 0) return rec;
    }

    struct VulkanFrame* fr = &self->vk.frames[self->vk.frame_index];
    if (self->vk.present_queue && fr->present_sem) {
        uint32_t image_index = (uint32_t)self->vk.current_framebuffer;
        VkSemaphore signal_sem[] = {fr->present_sem};
        VkPresentInfoKHR present_info = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .swapchainCount = 1,
            .pSwapchains = &self->vk.swapchain.handle,
            .pImageIndices = &image_index,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signal_sem,
        };
        VkResult res = vkQueuePresentKHR(self->vk.present_queue, &present_info);
        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
            self->vk.swapchain_dirty = 1;
        else if (res != VK_SUCCESS)
            return luaL_error(L, "failed to present Vulkan swapchain: 0x%x", res);
    }
    self->vk.frame_index = (self->vk.frame_index + 1) % SELENE_VK_MAX_FRAMES;
    return 0;
}

int l_VK_Renderer_create(lua_State *L) {
    INIT_ARG();
    selene_Window* win = (selene_Window*)luaL_checkudata(L, arg++, selene_Window_METANAME);
    int width, height;
    SDL_GetWindowSize(win->handle, &width, &height);

    VkInstance instance;
    VkApplicationInfo app_info = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                    .pApplicationName = "Vulkan App",
                                    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                                    .pEngineName = "selene",
                                    .engineVersion = VK_MAKE_VERSION(0, 5, 0),
                                    .apiVersion = VK_API_VERSION_1_3
                                };

    unsigned int ext_count;
    if (SDL_Vulkan_GetInstanceExtensions(win->handle, &ext_count, NULL) != VK_TRUE) {
        return luaL_error(L, "failed to get Vulkan extensions count: %s", SDL_GetError());
    }

    const char **exts = malloc(sizeof(const char *) * ext_count);
    SDL_Vulkan_GetInstanceExtensions(win->handle, &ext_count, exts);
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
    DEBUG_LOG("created Vulkan instance\n");
    VkSurfaceKHR surface;
    if (SDL_Vulkan_CreateSurface(win->handle, instance, &surface) != SDL_TRUE) {
        vkDestroyInstance(instance, NULL);
        return luaL_error(L, "failed to create Vulkan surface: %s", SDL_GetError());
    }
    DEBUG_LOG("created Vulkan surface\n");
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
    fprintf(stdout, "Vulkan device: %s\n", pdev_props.deviceName);

    struct QueueFamilyIndices indices = find_queue_families(phys, surface);
    if (indices.graphics_family < 0 || indices.present_family < 0) {
        vkDestroySurfaceKHR(instance, surface, NULL);
        vkDestroyInstance(instance, NULL);
        return luaL_error(L, "no suitable Vulkan queue families");
    }

    float queue_priority = 1.f;
    VkDeviceQueueCreateInfo queue_infos[2];
    uint32_t queue_info_count = 1;
    queue_infos[0] = (VkDeviceQueueCreateInfo){
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = (uint32_t)indices.graphics_family,
        .queueCount = 1,
        .pQueuePriorities = &queue_priority
    };
    if (indices.graphics_family != indices.present_family) {
        queue_infos[1] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = (uint32_t)indices.present_family,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority
        };
        queue_info_count = 2;
    }

    const char *device_extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_MAINTENANCE1_EXTENSION_NAME
    };
    VkPhysicalDeviceFeatures available_features;
    vkGetPhysicalDeviceFeatures(phys, &available_features);
    VkPhysicalDeviceFeatures enabled_features;
    memset(&enabled_features, 0, sizeof(enabled_features));
    enabled_features.samplerAnisotropy = available_features.samplerAnisotropy;

    VkDeviceCreateInfo dev_crt_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queue_infos,
        .queueCreateInfoCount = queue_info_count,
        .pEnabledFeatures = &enabled_features,
        .enabledExtensionCount = 2,
        .ppEnabledExtensionNames = device_extensions,
        .enabledLayerCount = 0
    };

    VkDevice device;
    if (vkCreateDevice(phys, &dev_crt_info, NULL, &device) != VK_SUCCESS) {
        vkDestroySurfaceKHR(instance, surface, NULL);
        vkDestroyInstance(instance, NULL);
        return luaL_error(L, "failed to create Vulkan device");
    }
    DEBUG_LOG("created Vulkan device\n");

    VkQueue gfx_queue, present_queue;
    vkGetDeviceQueue(device, indices.graphics_family, 0, &gfx_queue);
    vkGetDeviceQueue(device, indices.present_family, 0, &present_queue);

    struct VulkanSwapchain swapchain;
    if (vk_create_swapchain(phys, device, surface, indices, width, height, &swapchain) < 0) {
        return luaL_error(L, SDL_GetError());
    }

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

    VkResult result = vkCreateRenderPass(device, &render_pass_info, NULL, &render_pass);
    if (result != VK_SUCCESS) {
        return luaL_error(L, "failed to create Vulkan render pass");
    }

    /* Same subpass and dependency as the swapchain pass so pipelines stay
     * compatible. Only the final layout differs: shader-read, not present. */
    VkAttachmentDescription target_attachment = color_attachment;
    target_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    target_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkRenderPassCreateInfo target_pass_info = render_pass_info;
    target_pass_info.pAttachments = &target_attachment;
    VkRenderPass target_render_pass = VK_NULL_HANDLE;
    result = vkCreateRenderPass(device, &target_pass_info, NULL, &target_render_pass);
    if (result != VK_SUCCESS) {
        return luaL_error(L, "failed to create Vulkan target render pass");
    }

    VkCommandPool command_pool;

    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = indices.graphics_family,
    };

    if (vkCreateCommandPool(device, &pool_info, NULL, &command_pool) != VK_SUCCESS) {
        return luaL_error(L, "failed to create Vulkan command pool");
    }

    VkDescriptorPool descriptor_pool;
    VkDescriptorPoolSize descriptor_pool_size[] = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 64 },
    };

    const int max_sets_count = 10;
    VkDescriptorPoolCreateInfo descriptor_pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 100,
        .poolSizeCount = 3,
        .pPoolSizes = descriptor_pool_size
    };
    result = vkCreateDescriptorPool(device, &descriptor_pool_info, NULL, &descriptor_pool);
    if (result != VK_SUCCESS) {
        return luaL_error(L, "failed to create descriptor pool, 0x%8x", result);
    }

    NEW_UDATA(selene_Renderer, ren);
    memset(ren, 0, sizeof(*ren));
    ren->vk.device = device;
    ren->vk.instance = instance;
    ren->vk.surface = surface;
    ren->vk.phys_device = phys;
    ren->vk.render_pass = render_pass;
    ren->vk.target_render_pass = target_render_pass;
    ren->vk.command_pool = command_pool;
    ren->vk.swapchain = swapchain;
    ren->vk.graphics_queue = gfx_queue;
    ren->vk.present_queue = present_queue;
    ren->vk.graphics_family = indices.graphics_family;
    ren->vk.present_family = indices.present_family;
    ren->vk.descriptor_pool = descriptor_pool;

    VkPhysicalDeviceProperties limits_props;
    vkGetPhysicalDeviceProperties(phys, &limits_props);
    ren->vk.uniform_align = limits_props.limits.minUniformBufferOffsetAlignment;
    if (ren->vk.uniform_align < 1) ren->vk.uniform_align = 1;
    ren->vk.ubo_stride = (SELENE_VK_UBO_RANGE + ren->vk.uniform_align - 1) & ~(ren->vk.uniform_align - 1);

    if (vk_make_framebuffers(ren) != 0)
        return luaL_error(L, "failed to create Vulkan framebuffers");
    if (vk_init_frames(ren) != 0)
        return luaL_error(L, "failed to create Vulkan frame sync");

    ren->window_ptr = win;
    lua_pushvalue(L, 1);
    ren->l_window_ref = luaL_ref(L, LUA_REGISTRYINDEX);

    ren->width = width;
    ren->height = height;
    ren->in_frame = 0;
    ren->in_pass = 0;
    if (!renderer_init_command_pool(ren, 512))
        return luaL_error(L, "failed to alloc renderer command pool");

    ren->backend = SELENE_RENDERER_VULKAN;

    ren->destroy = l_VK_Renderer__destroy;
    ren->begin_frame = l_VK_Renderer__begin_frame;
    ren->end_frame = l_VK_Renderer__end_frame;
    ren->begin_pass = NULL;
    ren->end_pass = NULL;
    ren->flush = l_VK_Renderer__flush;
    ren->present = l_VK_Renderer__present;

    ren->create_pipeline = l_VK_Renderer__create_pipeline;
    ren->destroy_pipeline = l_VK_Renderer__destroy_pipeline;

    ren->create_texture2d = l_VK_Renderer__create_texture2d;
    ren->destroy_texture = l_VK_Renderer__destroy_texture;

    ren->create_render_target = l_VK_Renderer__create_render_target;
    ren->destroy_render_target = l_VK_Renderer__destroy_render_target;

    ren->create_shader = l_VK_Renderer__create_shader;
    ren->destroy_shader = l_VK_Renderer__destroy_shader;

    ren->create_buffer = l_VK_Renderer__create_buffer;
    ren->destroy_buffer = l_VK_Renderer__destroy_buffer;
    ren->send_buffer_data = l_VK_Renderer__send_buffer_data;

    DEBUG_LOG("Created the Vulkan renderer: instance(%p) device(%p)\n", instance, device);

    return 1;
}

VkResult vk_create_swapchain(VkPhysicalDevice physical_device, VkDevice device,
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
        if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = formats[i];
            DEBUG_LOG("found surface format in %d\n", i);
            break;
        }
    }
    free(formats);
    DEBUG_LOG("Vulkan surface format: %d %d\n", surfaceFormat.format, surfaceFormat.colorSpace);
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
        DEBUG_LOG("different indices for queue family: %d %d\n", indices.graphics_family, indices.present_family);
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        DEBUG_LOG("same indices for queue family: %d %d\n", indices.graphics_family, indices.present_family);
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkSwapchainKHR swapchain;
    VkResult res = vkCreateSwapchainKHR(device, &create_info, NULL, &swapchain);
    if (res != VK_SUCCESS) {
        SDL_SetError("failed to create Vulkan toolchain, error: 0x%8x", res);
        return -1;
    }
    DEBUG_LOG("created Vulkan swapchain\n");

    vkGetSwapchainImagesKHR(device, swapchain, &image_count, NULL);
    outSwapchain->images = malloc(image_count * sizeof(VkImage));
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, outSwapchain->images);

    outSwapchain->handle = swapchain;
    outSwapchain->views = malloc(image_count * sizeof(VkImageView));
    if (!outSwapchain->views) {
        SDL_SetError("failed to alloc memory for Vulkan image views");
        return -1;
    }
    DEBUG_LOG("memory allocated for the Vulkan Swapchain image views: %p\n", outSwapchain->views);
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
            DEBUG_LOG("failed to create Vulkan ImageView, error: 0x%8x\n", result);
            for (uint32_t j = 0; j < i; j++) {
                vkDestroyImageView(device, outSwapchain->views[j], NULL);
            }
            free(outSwapchain->views);
            free(outSwapchain->images);
            return result;
        }
    }

    outSwapchain->format = surfaceFormat.format;
    outSwapchain->extent = extent;
    outSwapchain->image_count = image_count;
    return 0;
}

void vk_destroy_swapchain(VkPhysicalDevice physical_device, VkDevice device, struct VulkanSwapchain swap) {
    for (int i = 0; i < swap.image_count; i++) {
        vkDestroyImageView(device, swap.views[i], NULL);
    }
    vkDestroySwapchainKHR(device, swap.handle, NULL);
    free(swap.views);
    free(swap.images);
}

VkResult vk_create_texture(VkDevice device, VkPhysicalDevice physicalDevice,
                       uint32_t width, uint32_t height, VkFormat format,
                       VkImageTiling tiling, VkImageUsageFlags usage,
                       VkMemoryPropertyFlags properties,
                       selene_Texture2D *outTexture) {
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

    VkResult result = vkCreateImage(device, &imageInfo, NULL, &outTexture->vk.handle);
    if (result != VK_SUCCESS) {
        SDL_SetError("failed to create Vulkan image, error: 0x%8x", result);
        return result;
    }

    // Allocate memory
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, outTexture->vk.handle, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    allocInfo.allocationSize = memRequirements.size;

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    uint32_t memoryTypeIndex;
    for (memoryTypeIndex = 0; memoryTypeIndex < memProperties.memoryTypeCount; memoryTypeIndex++) {
        if ((memRequirements.memoryTypeBits & (1 << memoryTypeIndex)) &&
            (memProperties.memoryTypes[memoryTypeIndex].propertyFlags &
                properties) == properties) {
            break;
        }
    }

    allocInfo.memoryTypeIndex = memoryTypeIndex;
    result = vkAllocateMemory(device, &allocInfo, NULL, &outTexture->vk.mem);
    if (result != VK_SUCCESS) {
        SDL_SetError("failed to allocate memory for Vulkan texture: 0x%8x", result);
        return result;
    }

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
    if (result != VK_SUCCESS) {
        SDL_SetError("failed to create Vulkan image view, error: 0x%8x", result);
        return result;
    }

    outTexture->width = width;
    outTexture->height = height;
    outTexture->format = format;

    return result;
}

VkResult vk_create_sampler(selene_Renderer* self, VkFilter filter, VkSampler *out) {
    VkPhysicalDeviceProperties properties = {0};
    vkGetPhysicalDeviceProperties(self->vk.phys_device, &properties);
    VkPhysicalDeviceFeatures features = {0};
    vkGetPhysicalDeviceFeatures(self->vk.phys_device, &features);

    VkSamplerCreateInfo samplerInfo = {0};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = filter;
    samplerInfo.minFilter = filter;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = features.samplerAnisotropy;
    samplerInfo.maxAnisotropy = features.samplerAnisotropy ? properties.limits.maxSamplerAnisotropy : 1.f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VkSampler handle;
    VkResult result = vkCreateSampler(self->vk.device, &samplerInfo, NULL, &handle);
    if (result != VK_SUCCESS) {
        SDL_SetError("failed to create texture sampler, error: 0x%8x", result);
        return -1;
    }
    *out = handle;
    return VK_SUCCESS;
}

VkImageView vk_create_image_view(selene_Renderer* self, VkImage image, VkFormat format) {
    VkImageViewCreateInfo viewInfo = {0};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    VkResult result = vkCreateImageView(self->vk.device, &viewInfo, NULL, &imageView);
    if (result != VK_SUCCESS) {
        SDL_SetError("failed to create texture image view, error: 0x%8x", result);
        return NULL;
    }
    return imageView;
}

int vk_create_descriptor_pool(selene_Renderer* self) {
    if (!self->vk.device) {
        return -1;
    }
    
    VkDescriptorPoolSize poolSize = {0};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = MAX_FRAMES_;

    VkDescriptorPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = MAX_FRAMES_;
    
    if (!self->vk.device) {
        SDL_SetError("Vulkan device not initialized");
        return -1;
    }
    
    VkResult result = vkCreateDescriptorPool(self->vk.device, &poolInfo, NULL, &(self->vk.descriptor_pool));
    if (result != VK_SUCCESS) {
        SDL_SetError("failed to create descriptor pool, error: 0x%8x", result);
        self->vk.descriptor_pool = VK_NULL_HANDLE;
        return -1;
    }
    return 0;
}

int vk_create_descriptor_sets(selene_Renderer* self, VkBuffer* uniforms, int uniform_size) {
    if (!self->vk.descriptor_pool || !self->vk.framebuffer_count) {
        return -1;
    }
    
    VkDescriptorSetLayout* layouts = malloc(self->vk.framebuffer_count * sizeof(VkDescriptorSetLayout));
    if (!layouts) {
        return -1;
    }
    
    VkDescriptorSetAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = self->vk.descriptor_pool;
    allocInfo.descriptorSetCount = self->vk.framebuffer_count;
    allocInfo.pSetLayouts = layouts;

    self->vk.descriptor_sets = malloc(sizeof(self->vk.framebuffer_count));
    if (!self->vk.descriptor_sets) {
        free(layouts);
        return -1;
    }
    
    VkResult result = vkAllocateDescriptorSets(self->vk.device, &allocInfo, self->vk.descriptor_sets);
    if (result != VK_SUCCESS) {
        SDL_SetError("failed to allocate descriptor sets, error: 0x%8x", result);
        free(layouts);
        free(self->vk.descriptor_sets);
        self->vk.descriptor_sets = NULL;
        return -1;
    }

    for (size_t i = 0; i < self->vk.framebuffer_count; i++) {
        VkDescriptorBufferInfo bufferInfo = {0};
        bufferInfo.buffer = uniforms[i];
        bufferInfo.offset = 0;
        bufferInfo.range = uniform_size;

        VkWriteDescriptorSet descriptorWrite = {0};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = self->vk.descriptor_sets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(self->vk.device, 1, &descriptorWrite, 0, NULL);
    }
    
    free(layouts);
    return 0;
}


VkResult vk_create_buffer(selene_Renderer *self, int size, int usage, int flags, VkBuffer *out_buf, VkDeviceMemory *out_mem) {
    VkBufferCreateInfo buf_info =
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    VkBuffer handle;

    VkResult result = vkCreateBuffer(self->vk.device, &buf_info, NULL, &handle);
    if (result != VK_SUCCESS) {
        SDL_SetError("failed to create Vulkan buffer, error: 0x%8x", result);
        return -1;
    }
    VkDeviceMemory buff_memory;
    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(self->vk.device, handle, &mem_reqs);
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(self->vk.phys_device, &mem_properties);

    VkMemoryAllocateInfo alloc_info =
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_reqs.size,
        .memoryTypeIndex = find_memory_type(self->vk.phys_device, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    };

    result = vkAllocateMemory(self->vk.device, &alloc_info, NULL, &buff_memory);
    if (result != VK_SUCCESS) {
        vkDestroyBuffer(self->vk.device, handle, NULL);
        SDL_SetError("failed to alloc memory for Vulkan buffer, error: 0x%8x", result);
        return -1;
    }
    vkBindBufferMemory(self->vk.device, handle, buff_memory, 0);
    if (out_buf) *out_buf = handle;
    if (out_mem) *out_mem = buff_memory;
    return 0;
}

VkCommandBuffer begin_single_time_command(selene_Renderer* self) {
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = self->vk.command_pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(self->vk.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void end_single_time_command(selene_Renderer* self, VkCommandBuffer cb) {
    vkEndCommandBuffer(cb);

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cb;

    vkQueueSubmit(self->vk.graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(self->vk.graphics_queue);

    vkFreeCommandBuffers(self->vk.device, self->vk.command_pool, 1, &cb);
}

int transition_image_layout(selene_Renderer* self, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer buf = begin_single_time_command(self);
    VkImageMemoryBarrier barrier = {0};
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
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        /* empty textures / render targets: make them sampleable from the start */
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        SDL_SetError("unsupported layout transition!");
        return -1;
    }

    vkCmdPipelineBarrier(
        buf,
        sourceStage, destinationStage,
        0,
        0, NULL,
        0, NULL,
        1, &barrier
    );

    end_single_time_command(self, buf);
    return 0;
}

void copy_buffer_to_image(selene_Renderer* self, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = begin_single_time_command(self);

    VkBufferImageCopy region = {0};
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
