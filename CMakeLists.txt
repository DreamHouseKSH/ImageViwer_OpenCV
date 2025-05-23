cmake_minimum_required(VERSION 3.16)
project(airphoto_viewer VERSION 0.1.0 LANGUAGES CXX)

# QML 모듈 버전 설정
set(MEASUREMENT_MODULE_MAJOR_VERSION 1)
set(MEASUREMENT_MODULE_MINOR_VERSION 0)
set(MEASUREMENT_MODULE_VERSION ${MEASUREMENT_MODULE_MAJOR_VERSION}.${MEASUREMENT_MODULE_MINOR_VERSION})

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

# libtiff 라이브러리 찾기
find_package(TIFF REQUIRED)
if(TIFF_FOUND)
    add_definitions(-DHAVE_TIFF)
    include_directories(${TIFF_INCLUDE_DIRS})
    
    message(STATUS "Found TIFF: ${TIFF_LIBRARIES}")
    message(STATUS "TIFF includes: ${TIFF_INCLUDE_DIRS}")
    message(STATUS "TIFF library: ${TIFF_LIBRARY}")
    
    # TIFF 라이브러리 링크를 위한 변수 설정
    set(TIFF_LINK_LIBRARIES ${TIFF_LIBRARIES})
    
    # macOS에서는 추가 종속성 필요
    if(APPLE)
        find_package(JPEG REQUIRED)
        find_package(ZLIB REQUIRED)
        
        # 필요한 라이브러리 추가
        list(APPEND TIFF_LINK_LIBRARIES
            ${JPEG_LIBRARIES}
            ${ZLIB_LIBRARIES}
        )
        
        # macOS에서 필요한 추가 라이브러리
        find_library(ICONV_LIBRARY iconv)
        find_library(LZMA_LIBRARY lzma)
        find_library(WEBP_LIBRARY webp)
        find_library(JPEG2000_LIBRARY jpeg)
        
        if(ICONV_LIBRARY)
            list(APPEND TIFF_LINK_LIBRARIES ${ICONV_LIBRARY})
        endif()
        if(LZMA_LIBRARY)
            list(APPEND TIFF_LINK_LIBRARIES ${LZMA_LIBRARY})
        endif()
        if(WEBP_LIBRARY)
            list(APPEND TIFF_LINK_LIBRARIES ${WEBP_LIBRARY})
        endif()
        if(JPEG2000_LIBRARY)
            list(APPEND TIFF_LINK_LIBRARIES ${JPEG2000_LIBRARY})
        endif()
    endif()
    
    # 링크 디렉토리 추가
    get_filename_component(TIFF_LIBRARY_DIR ${TIFF_LIBRARY} DIRECTORY)
    link_directories(${TIFF_LIBRARY_DIR})
    
    # 모든 타겟에 TIFF 링크 추가
    add_definitions(-DHAVE_TIFF=1)
    
    message(STATUS "TIFF link libraries: ${TIFF_LINK_LIBRARIES}")
    message(STATUS "TIFF library directory: ${TIFF_LIBRARY_DIR}")
else()
    message(FATAL_ERROR "TIFF library not found")
endif()

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

# 측정 도구 모듈 소스 파일
set(MEASUREMENT_TOOL_SOURCES
    "src/tools/MeasurementTool.cpp"
    "src/tools/MeasurementOverlay.cpp"
    "src/tools/MeasurementPlugin.cpp"
)

# QML 모듈 리소스 파일
qt_add_qml_module(airphoto_measurement
    URI "AirPhoto/Viewer/Measurement"
    VERSION ${MEASUREMENT_MODULE_MAJOR_VERSION}.${MEASUREMENT_MODULE_MINOR_VERSION}
    QML_FILES
        "qml/MeasurementTool.qml"
        "qml/MeasurementOverlay.qml"
    SOURCES
        ${MEASUREMENT_TOOL_SOURCES}
        "include/airphoto_viewer/MeasurementTool.hpp"
        "include/airphoto_viewer/MeasurementOverlay.hpp"
        "include/airphoto_viewer/MeasurementPlugin.hpp"
)

# 메인 애플리케이션 소스 파일
file(GLOB_RECURSE SOURCES 
    "src/*.cpp" 
    "src/*/*.cpp"
    "src/*/*/*.cpp"
    "include/airphoto_viewer/*.hpp"
    "include/airphoto_viewer/*.h"
    "include/airphoto_viewer/*/*.hpp"
    "include/airphoto_viewer/*/*.h"
    "${CMAKE_CURRENT_BINARY_DIR}/airphoto_measurement_qmltyperegistrations.cpp"
)

# QML 모듈 컴파일 디렉토리 추가
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/airphoto_measurement_autogen/include_
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
    Qt6::Qml
    Qt6::Quick
    exiv2lib
    ${TIFF_LIBRARIES}
    ${TIFF_LINK_LIBRARIES}
    airphoto_measurement
)

# QML 모듈 설치 경로 설정
install(TARGETS airphoto_measurement
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# 예제 애플리케이션
add_executable(measurement_example examples/MeasurementExample.qml)
set_target_properties(measurement_example PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

target_link_libraries(measurement_example
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    airphoto_measurement
)

# QML 모듈 임포트 경로 설정
target_compile_definitions(measurement_example
    PRIVATE
    QT_QML_DEBUG
    QT_DECLARATIVE_DEBUG
)

# QML 모듈 임포트 경로 설정 (개발 중에는 빌드 디렉토리 사용)
set(QML_IMPORT_PATH "${CMAKE_CURRENT_BINARY_DIR}")
set(QML2_IMPORT_PATH "${CMAKE_CURRENT_BINARY_DIR}")
set(QT_QML_MODULES "${CMAKE_CURRENT_BINARY_DIR}")

# QML 모듈 임포트 경로를 환경 변수로 설정
set(ENV{QML2_IMPORT_PATH} "${QML_IMPORT_PATH}")
set(ENV{QT_QML_MODULES} "${QT_QML_MODULES}")

# TIFF 로더 테스트
set(TEST_TIFF_LOADER_SOURCES
    "test/test_tiff_loader.cpp"
    "src/core/ImageData.cpp"
    "src/core/ImageMetadata.cpp"
)

add_executable(test_tiff_loader ${TEST_TIFF_LOADER_SOURCES})



# 테스트 실행 파일 설정
target_include_directories(test_metadata
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${EXIV2_INCLUDE_DIR}
    ${TIFF_INCLUDE_DIR}
)

target_link_libraries(test_metadata
    PRIVATE
    ${OpenCV_LIBS}
    exiv2lib
    nlohmann_json::nlohmann_json
    ${TIFF_LIBRARIES}
    ${TIFF_LINK_LIBRARIES}
)

target_include_directories(test_metadata_simple
    PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${EXIV2_INCLUDE_DIR}
    ${TIFF_INCLUDE_DIR}
)

target_link_libraries(test_metadata_simple
    PRIVATE 
    ${OpenCV_LIBS}
    exiv2lib
    ${TIFF_LIBRARIES}
    ${TIFF_LINK_LIBRARIES}
)

target_include_directories(test_tiff_loader
    PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${EXIV2_INCLUDE_DIR}
    ${TIFF_INCLUDE_DIR}
)

target_link_libraries(test_tiff_loader
    PRIVATE 
    ${OpenCV_LIBS}
    exiv2lib
    ${TIFF_LIBRARIES}
    ${TIFF_LINK_LIBRARIES}
)

# 테스트 타겟에 추가
add_test(NAME test_tiff_loader COMMAND test_tiff_loader)

# 설치 타겟 설정
install(TARGETS airphoto_viewer
    RUNTIME DESTINATION bin
)

# 테스트 디렉토리 추가 (나중에 사용)
# enable_testing()
# add_subdirectory(test)
