#type vertex
#version 450 core

layout(location = 0) in vec2 aPos;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
}

#type fragment
#version 450 core

out vec4 FragColor;

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(800.0, 600.0); // 화면 비율 보정

    float wave = sin(uv.x * 20.0) * cos(uv.y * 20.0);
    float color = (wave + 1.0) * 0.5; // 0~1 정규화

    FragColor = vec4(color * 0.2, color * 0.7, color * 1.0, 1.0);
}
