@type compute
#version 460 core

layout(local_size_x = 1) in;

layout(std430, binding = 1) buffer InBuf {
    float inData[];
};

layout(std430, binding = 2) buffer OutBuf {
    float outData[];
};

void main() {
    uint gid = gl_GlobalInvocationID.x;
    if (gid >= 64) return;

    // 1) 입력 버퍼를 셰이더 안에서 채움
    float x = float(gid);
    inData[gid] = x;

    // 2) 그 값을 이용해 2차식 계산
    float y = x * x + 3.0 * x;

    // 3) 출력 버퍼에 기록
    outData[gid] = y;
}
