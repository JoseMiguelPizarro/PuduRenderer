//
// Created by Administrator on 6/23/2025.
//
#pragma once

#include "PuduCore.h"

ENUM_STRING(
    GPUResourceType,
    UNDEFINED,
    Texture,
    Texture2D,
    TextureCube,
    Texture2DArray,
    Sampler,
    RenderPass,
    Framebuffer,
    Pipeline,
    ShaderState,
    DescriptorSetLayout,
    Shader,
    Mesh,
    Material,
    ComputeShader,
    Buffer,
    Semaphore,
    GPUCommands,
    RenderTexture,
    CommandPool,
    DescriptorPool);