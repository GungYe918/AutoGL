@type compute
#version 460

layout(std430, binding = 0) buffer Data {
    float a;
    float b;
    float add;
    float sub;
    float mul;
    float divv;
};

layout(local_size_x = 1) in;

void main() {
    a = 10.0;
    b = 4.0;

    add = a + b;
    sub = a - b;
    mul = a * b;
    divv = a / b;
}
