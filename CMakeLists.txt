cmake_minimum_required(VERSION 3.16)
project(airphoto_viewer VERSION 0.1.0 LANGUAGES CXX)

# C++17 표준 사용
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# OpenCV 패키지 찾기
find_package(OpenCV REQUIRED)

# Qt6 패키지 찾기
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Exiv2 라이브러리 찾기
find_package(exiv2 REQUIRED)

# nlohmann_json 라이브러리 찾기
find_package(nlohmann_json REQUIRED)

# 헤더 파일 디렉토리 설정
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${EXIV2_INCLUDE_DIR}
)

# Qt6 관련 디렉토리 설정
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt6Widgets_EXECUTABLE_COMPILE_FLAGS}")

# 메타오브젝트 컴파일러(MOC) 설정
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 헤더 파일 디렉토리 설정
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)

# 소스 파일 추가
file(GLOB_RECURSE SOURCES 
    "src/*.cpp" 
    "src/*/*.cpp"
    "src/*/*/*.cpp"
    "include/airphoto_viewer/*.hpp"
    "include/airphoto_viewer/*.h"
    "include/airphoto_viewer/*/*.hpp"
    "include/airphoto_viewer/*/*.h"
)

# 메인 실행 파일 생성
add_executable(airphoto_viewer ${SOURCES})

# 테스트 실행 파일
set(TEST_METADATA_SOURCES
    "test/test_metadata.cpp"
    "src/core/ImageData.cpp"
    "src/core/ImageMetadata.cpp"
)

add_executable(test_metadata ${TEST_METADATA_SOURCES})

# 간단한 메타데이터 테스트
set(TEST_METADATA_SIMPLE_SOURCES
    "test/test_metadata_simple.cpp"
    "src/core/ImageData.cpp"
    "src/core/ImageMetadata.cpp"
)

add_executable(test_metadata_simple ${TEST_METADATA_SIMPLE_SOURCES})

# 라이브러리 링크
# Exiv2 라이브러리 찾기
find_package(exiv2 REQUIRED)

# nlohmann_json 라이브러리 찾기
find_package(nlohmann_json REQUIRED)

target_include_directories(airphoto_viewer 
    PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${EXIV2_INCLUDE_DIR}
)

target_link_libraries(airphoto_viewer 
    PRIVATE 
    ${OpenCV_LIBS} 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets
    exiv2lib
)

# 테스트 실행 파일 설정
target_include_directories(test_metadata
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${EXIV2_INCLUDE_DIR}
)

target_link_libraries(test_metadata
    PRIVATE
    ${OpenCV_LIBS}
    exiv2lib
    nlohmann_json::nlohmann_json
)

target_link_libraries(test_metadata_simple
    PRIVATE
    ${OpenCV_LIBS}
    exiv2lib
)

# 설치 타겟 설정
install(TARGETS airphoto_viewer
    RUNTIME DESTINATION bin
)

# 테스트 디렉토리 추가 (나중에 사용)
# enable_testing()
# add_subdirectory(test)
