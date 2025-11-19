@type compute
#version 450 core

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

// AutoGL: SSBO 자동 생성
layout(std430, binding = 0) buffer Data {
    uint values[];
};

// AutoGL built-in uniforms
uniform float iTime;
uniform vec2  iResolution;
uniform vec4  iMouse;

void main() {
    uint idx = gl_GlobalInvocationID.x + gl_GlobalInvocationID.y * 8;
    values[idx] = idx * 2 + uint(iTime);  // 시간에 따라 값이 약간 변함
}
