// src/glsl_types.hpp
#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace AutoGL {

    enum class SSBOBaseType : uint8_t  {
        Float,
        Int,
        UInt,
        Vec,
        Mat,
        Struct,
        Unknown
    }; 

    struct SSBOMemberInfo {
        std::string  name;
        SSBOBaseType base;
        int          components = 1;   // vec3 = 3, float = 1
        int          arraySize = 1;    // arr[] = many, arr[10] = 10
        int          offset    = 0;    // std430 offset
        int          stride    = 0;    // std430 stride
    };

    struct SSBOTypeInfo {
        std::string rawType;     // "float", "uint", "vec3", "Foo"
        SSBOBaseType base;
        int components = 1;
        int stride     = 4;      // std430 stride
        bool isArray   = false;

        // struct일 경우
        std::vector<SSBOMemberInfo> members;

        bool isStruct()  const {  return base == SSBOBaseType::Struct;  }
        bool isMatrix()  const {  return base == SSBOBaseType::Mat;     }
        bool isVector()  const {  return base == SSBOBaseType::Vec;     }
        bool isScalar()  const {
            return base == SSBOBaseType::Float
                        || base == SSBOBaseType::Int
                        || base == SSBOBaseType::UInt;  }
    };  

    inline SSBOTypeInfo ParseSingleType(const std::string& t) {
        SSBOTypeInfo info;
        info.rawType = t;

        if (t == "float") {
            info.base = SSBOBaseType::Float;
            info.components = 1;
            info.stride = 4;
        }
        else if (t == "int") {
            info.base = SSBOBaseType::Int;
            info.components = 1;
            info.stride = 4;
        }
        else if (t == "uint") {
            info.base = SSBOBaseType::UInt;
            info.components = 1;
            info.stride = 4;
        }
        else if (t.rfind("vec", 0) == 0) {
            info.base = SSBOBaseType::Vec;
            info.components = std::stoi(t.substr(3));
            // std430: vec3도 padding되어 16바이트
            info.stride = 16;
        }
        else {
            // struct 또는 사용자 정의 타입
            info.base = SSBOBaseType::Struct;
            info.components = 0;
            // stride는 struct 완전 파싱 이후 결정됨
        }

        return info;
    }



} // namespace AutoGL