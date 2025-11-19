@type vertex
#version 450 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;

out vec2 vUV;

void main() {
    vUV = aUV;
    gl_Position = vec4(aPos, 0.0, 1.0);
}

@type fragment
#version 450 core

in vec2 vUV;
out vec4 FragColor;

// AutoGL built-ins
uniform float iTime;
uniform vec2  iResolution;
uniform vec4  iMouse;   // (mx, my, clickX, clickY)

void main() {
    // 화면 좌표를 0~1로 정규화
    vec2 uv = gl_FragCoord.xy / iResolution;

    // 시간 기반 움직이는 그라디언트
    float t = iTime * 0.5;
    vec3 color = vec3(
        0.5 + 0.5 * sin(t + uv.x * 5.0),
        0.5 + 0.5 * sin(t + uv.y * 5.0),
        0.5 + 0.5 * sin(t)
    );

    // 클릭 위치 강조
    // iMouse.zw = 클릭 좌표
    if (iMouse.z > 0.0 || iMouse.w > 0.0) {
        vec2 click = iMouse.zw / iResolution;
        float d = distance(uv, click);

        // 클릭 중심으로 빛나는 원
        float glow = 0.2 / (d + 0.01);
        color += vec3(glow, glow * 0.5, glow * 0.2);
    }

    FragColor = vec4(color, 1.0);
}
