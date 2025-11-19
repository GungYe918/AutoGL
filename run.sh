#!/bin/bash

MODE="$1"
ARG="$2"

mkdir -p build
cmake -S . -B build || exit 1
cmake --build build || exit 1

# AutoGL 실행 파일 경로
AUTOGL_EXE="./build/AutoGL/autogl"

if [ "$MODE" = "--source" ]; then
    if [ ! -f "$ARG" ]; then
        echo "Source file not found: $ARG"
        exit 1
    fi
    echo "===== Running OpenGLStudy ====="
    ./build/OpenGLStudy

elif [ "$MODE" = "--shader" ]; then
    if [ ! -f "$ARG" ]; then
        echo "Shader file not found: $ARG"
        exit 1
    fi
    echo "===== Running AutoGL Shader ====="

    if [ ! -f "$AUTOGL_EXE" ]; then
        echo "AutoGL 실행 파일이 없습니다: $AUTOGL_EXE"
        exit 1
    fi

    $AUTOGL_EXE --shader "$ARG"

else
    echo "Usage:"
    echo "  ./run.sh --source src/main.cpp"
    echo "  ./run.sh --shader shader/test.glsl"
fi
