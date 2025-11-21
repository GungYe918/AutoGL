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

uniform float iTime;
uniform vec2  iResolution;
uniform vec4  iMouse; // (mx, my, clickX, clickY)

void main() {
    vec2 uv = gl_FragCoord.xy / iResolution;

    // 마우스 좌표 (0~1 정규화)
    vec2 mouse = iMouse.xy / iResolution;
    
    // 마우스가 아직 안 움직이면 (0,0)으로 들어오므로 비활성 처리
    if (iMouse.xy == vec2(0.0)) {
        FragColor = vec4(0.1, 0.15, 0.25, 1.0); // 기본 파란 배경
        return;
    }

    // 거리 계산
    float d = distance(uv, mouse);

    // 반지름 효과 범위
    float radius = 0.4;

    // d가 멀수록 0, 가까울수록 1
    float glow = smoothstep(radius, 0.0, d);

    // 보라색
    vec3 purple = vec3(0.8, 0.3, 1.0);

    // 기본 배경 (파란 계열)
    vec3 bg = vec3(0.1, 0.15, 0.25);

    // 최종 색상 혼합
    vec3 color = mix(purple, bg, glow);

    FragColor = vec4(color, 1.0);
}
