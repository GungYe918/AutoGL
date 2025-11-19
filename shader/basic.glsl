@type vertex
#version 450 core

layout(location = 0) in vec2 aPos;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
}

@type fragment
#version 450 core

out vec4 FragColor;
in vec2 FragCoord;

void main() {
    float y = gl_FragCoord.y / 600.0; // 화면 높이 기준
    FragColor = mix(vec4(0.2, 0.1, 0.6, 1.0),
                    vec4(0.2, 0.6, 1.0, 1.0),
                    y);
}
