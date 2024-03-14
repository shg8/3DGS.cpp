#version 450
#extension GL_GOOGLE_include_directive : enable
#include "./common.glsl"

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : enable

layout (std430, set = 0, binding = 0) readonly buffer SortedList {
    uint64_t keys[];
};

layout (std430, set = 0, binding = 1) writeonly buffer Out {
    uint boundaries[];
};

layout( push_constant ) uniform Constants
{
    uint numInstances;
};

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

void main() {
    uint index = gl_GlobalInvocationID.x;
    if (index >= numInstances) {
        return;
    }

    uint key = uint(keys[index] >> 32);
    if (index == 0) {
        boundaries[key * 2] = index;
    } else {
        uint prevKey = uint(keys[index - 1] >> 32);
        if (prevKey > key) {
//            debugPrintfEXT("prevKey > key: %d > %d\n", prevKey, key);
        }
        if (key != prevKey) {
            boundaries[key * 2] = index;
            boundaries[prevKey * 2 + 1] = index;
//            if (prevKey == 61) {
//                debugPrintfEXT("end | key: %d, index: %d\n", key, index);
//            }
//            if (key == 61) {
//                debugPrintfEXT("start | key: %d, index: %d\n", key, index);
//            }
        }
    }
    if (index == numInstances - 1) {
        boundaries[key * 2 + 1] = numInstances;
    }
}