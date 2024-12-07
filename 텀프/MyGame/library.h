#pragma once


// 해당 파일이 없다고 뜨면서 컴파일이 안된다면
// 프로젝트 - (이름)속성 - 링커 - 일반 - 추가 라이브러리 디렉터리
// .\Dependencies 입력 (. 필수)
#ifdef NDEBUG	// Release	일때
#	pragma comment (lib, "libglew32.lib")
#else			// Debug	일때
#   pragma comment (lib, "libglew32d.lib")
#endif


// 아래 헤더가 빨간줄이 뜬다면
// 프로젝트 - (이름)속성 - C/C++ - 일반 - 추가 포함 디렉터리
// .\Dependencies (. 필수)
#define GLEW_STATIC
#include "glew.h"

#define FREEGLUT_STATIC
#include "freeglut.h"

#include "glm/glm.hpp"
#include "glm/ext.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx//norm.hpp"
