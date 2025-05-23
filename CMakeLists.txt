cmake_minimum_required(VERSION 3.10)
project(airphoto_viewer VERSION 0.1.0 LANGUAGES CXX)

# C++17 표준 사용
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# OpenCV 패키지 찾기
find_package(OpenCV REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# 헤더 파일 디렉토리 설정
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

# 소스 파일 추가
file(GLOB_RECURSE SOURCES 
    "src/*.cpp" 
    "src/*.hpp" 
    "src/*.h"
)

# 실행 파일 생성
add_executable(airphoto_viewer ${SOURCES})

# 라이브러리 링크
target_link_libraries(airphoto_viewer 
    PRIVATE 
    ${OpenCV_LIBS} 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets
)

# 설치 타겟 설정
install(TARGETS airphoto_viewer
    RUNTIME DESTINATION bin
)

# 테스트 디렉토리 추가 (나중에 사용)
# enable_testing()
# add_subdirectory(test)
