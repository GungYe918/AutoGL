#!/bin/bash

# 사용법 체크
if [ -z "$1" ]; then
    echo "Usage: ./error.sh <파일이름>"
    exit 1
fi

FILE="$1"

if [ ! -f "$FILE" ]; then
    echo "Error: file '$FILE' not found."
    exit 1
fi

echo "// $FILE"
cat "$FILE"
