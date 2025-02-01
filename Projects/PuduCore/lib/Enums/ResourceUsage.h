//
// Created by Hojaverde on 2/1/2025.
//

#pragma once
class ResourceUsage
{
    public:
    enum Enum
    {
        UNDEFINED = 0,
        VERTEX_AND_CONSTANT_BUFFER = 0x1,
        INDEX_BUFFER = 0x2,
        RENDER_TARGET = 0x4,
        UNORDERED_ACCESS = 0x8,
        DEPTH_WRITE = 0x10,
        DEPTH_READ = 0x20,
        NON_PIXEL_SHADER_RESOURCE = 0x40,
        PIXEL_SHADER_RESOURCE = 0x80,
        SHADER_RESOURCE = 0x40 | 0x80,
        STREAM_OUT = 0x100,
        INDIRECT_ARGUMENT = 0x200,
        COPY_DEST = 0x400,
        COPY_SOURCE = 0x800,
        GENERIC_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
        PRESENT = 0x1000,
        COMMON = 0x2000,
        RAYTRACING_ACCELERATION_STRUCTURE = 0x4000,
        SHADING_RATE_SOURCE = 0x8000,
    };
};