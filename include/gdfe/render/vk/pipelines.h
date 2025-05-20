#pragma once

#include "types.h"

typedef struct GDF_Shader_T* GDF_Shader;

/// Initializes a new GDF_VkPipelineBase object. Allocates memory and must be destroyed
/// with GDF_VkDestroyPipelineBase.
GDF_BOOL GDF_VkCreatePipelineBase(GDF_VkPipelineBase* out, GDF_VkPipelineCreateInfo create_info);

GDF_BOOL GDF_VkDestroyPipelineBase(GDF_VkPipelineBase* base);
