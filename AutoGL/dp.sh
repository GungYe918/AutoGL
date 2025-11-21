#!/bin/bash

# 프로젝트 루트 기준으로 실행한다고 가정
# 모든 cpp/hpp 파일을 재귀적으로 찾아서 출력

find . -type f \( -name "*.cpp" -o -name "*.hpp" \) | sort | while read file; do
    echo "// $file"
    cat "$file"
    echo ""
    echo ""
done

