#pragma once
#include <cstdint>


static const uint8_t K_MAX_IMAGE_OUTPUTS = 8;
// Maximum number of images/render_targets/fbo attachments usable. +1 if we count depth/stencil
static const uint8_t K_MAX_DESCRIPTOR_SET_LAYOUTS = 8; // Maximum number of layouts in the pipeline.
static const uint8_t K_MAX_SHADER_STAGES = 5;
// Maximum simultaneous shader stages. Applicable to all different type of pipelines.
static const uint8_t K_MAX_DESCRIPTORS_PER_SET = 16;
// Maximum list elements for both descriptor set layout and descriptor sets.
static const uint8_t K_MAX_VERTEX_STREAMS = 16;
static const uint8_t K_MAX_VERTEX_ATTRIBUTES = 16;

static const uint32_t K_SUBMIT_HEADER_SENTINEL = 0xfefeb7ba;
static const uint32_t K_MAX_RESOURCE_DELETIONS = 64;

static const uint32_t k_INVALID_HANDLE = 0xffffffff;

