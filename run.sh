#!/bin/bash

# 사용법 체크
if [ -z "$1" ]; then
    echo "Usage: ./run.sh <src/파일.cpp>"
    exit 1
fi

# 입력받은 파일 경로
SRC_FILE="$1"

# 파일 존재 여부 확인
if [ ! -f "$SRC_FILE" ]; then
    echo "Error: file '$SRC_FILE' not found."
    exit 1
fi

# 빌드 폴더 생성
mkdir -p build

# CMake 구성
cmake -S . -B build || exit 1

# 빌드
cmake --build build || exit 1

# 실행 파일 이름은 프로젝트명(OpenGLStudy) 기본 세팅
EXEC="./build/OpenGLStudy"

# 실행 가능한지 확인
if [ ! -f "$EXEC" ]; then
    echo "Error: executable not found at $EXEC"
    exit 1
fi

# 실행
echo "===== Running ====="
$EXEC
