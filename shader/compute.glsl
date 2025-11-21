@type vertex
#version 450 core

layout(location = 0) in vec2 aPos;
out vec2 vUV;

void main() {
    vUV = (aPos + 1.0) * 0.5;
    gl_Position = vec4(aPos, 0.0, 1.0);
}

@type fragment
#version 450 core

layout(std430, binding = 0) buffer Data {
    float values[];
};

in vec2 vUV;
out vec4 FragColor;

void main() {
    int idx = int(vUV.x * 255.0);
    float v = values[idx] / 255.0;
    FragColor = vec4(v, v, v, 1.0);
}
