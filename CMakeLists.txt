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

# Find required packages
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(nlohmann_json 3.4.0 REQUIRED)
find_package(TIFF REQUIRED)

# Find Exiv2 with specific components
find_package(exiv2 CONFIG REQUIRED)

# Set Exiv2 include directories
if(EXIV2_INCLUDE_DIRS)
    include_directories(${EXIV2_INCLUDE_DIRS})
elseif(EXIV2_INCLUDE_DIR)
    include_directories(${EXIV2_INCLUDE_DIR})
else()
    # Default Homebrew path for Exiv2
    include_directories(/opt/homebrew/include)
endif()

# Add Exiv2 definitions
if(EXIV2_DEFINITIONS)
    add_definitions(${EXIV2_DEFINITIONS})
endif()

# Set Exiv2 libraries
if(NOT TARGET exiv2::exiv2)
    if(EXIV2_LIBRARIES)
        set(EXIV2_LIBS ${EXIV2_LIBRARIES})
    else()
        # Default Homebrew path for Exiv2 libraries
        set(EXIV2_LIBS /opt/homebrew/lib/libexiv2.dylib)
    endif()
else()
    set(EXIV2_LIBS exiv2::exiv2)
endif()

# Set TIFF include directories
if(TIFF_INCLUDE_DIR)
    include_directories(${TIFF_INCLUDE_DIR})
endif()

# Find Qt6 components
set(QT_COMPONENTS Core Gui Widgets Qml Quick QuickControls2 Test)
find_package(Qt6 REQUIRED COMPONENTS ${QT_COMPONENTS})

# Enable automatic MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Set include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${EXIV2_INCLUDE_DIRS}
    ${TIFF_INCLUDE_DIRS}
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Gui_INCLUDE_DIRS}
    ${Qt6Widgets_INCLUDE_DIRS}
    ${Qt6Qml_INCLUDE_DIRS}
)

# Set source files
set(SOURCES
    src/main.cpp
    src/core/ImageData.cpp
    src/core/ImageMetadata.cpp
    src/core/image_loader.cpp
    src/core/tile.cpp
    src/core/tile_cache.cpp
    src/tools/MeasurementOverlay.cpp
    src/tools/MeasurementPlugin.cpp
    src/tools/MeasurementTool.cpp
    src/ui/main_window.cpp
    src/ui/tile_view.cpp
)

# Set header files
set(HEADERS
    include/airphoto_viewer/ImageData.hpp
    include/airphoto_viewer/ImageMetadata.hpp
    include/airphoto_viewer/MeasurementOverlay.hpp
    include/airphoto_viewer/MeasurementPlugin.hpp
    include/airphoto_viewer/MeasurementTool.hpp
    include/airphoto_viewer/core/image_loader.h
    include/airphoto_viewer/core/tile.h
    include/airphoto_viewer/core/tile_cache.h
    include/airphoto_viewer/ui/main_window.h
    include/airphoto_viewer/ui/tile_view.h
)

# Set UI files - currently no UI files in the project
set(UI_HEADERS
    # UI files will be added here when available
)

# Set QRC files
set(QRC_SOURCES
    resources/resources.qrc
)

# Set OpenCV include directories
if(OpenCV_INCLUDE_DIRS)
    include_directories(${OpenCV_INCLUDE_DIRS})
elseif(OpenCV_INCLUDE_DIR)
    include_directories(${OpenCV_INCLUDE_DIR})
endif()

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

# QML 모듈 관련 코드는 주석 처리
# 향후 QML 모듈이 필요할 때 주석을 해제하고 사용하세요.

# set(QML_FILES
#     "qml/MeasurementTool.qml"
#     "qml/MeasurementOverlay.qml"
# )

# qt6_add_resources(QML_RESOURCES "qml_resources"
#     PREFIX "/"
#     FILES ${QML_FILES}
# )

# set(MEASUREMENT_QML_MODULE_URI "AirPhoto/Viewer/Measurement")
# set(MEASUREMENT_QML_MODULE_VERSION ${MEASUREMENT_MODULE_MAJOR_VERSION}.${MEASUREMENT_MODULE_MINOR_VERSION})

# foreach(QML_FILE ${QML_FILES})
#     get_filename_component(QML_FILE_NAME ${QML_FILE} NAME)
#     set_source_files_properties(${QML_FILE}
#         PROPERTIES
#             QT_QML_MODULE_URI ${MEASUREMENT_QML_MODULE_URI}
#             QT_QML_MODULE_VERSION ${MEASUREMENT_QML_MODULE_VERSION}
#             QT_QML_MODULE_IMPORT_NAME "${MEASUREMENT_QML_MODULE_URI}"
#     )
# endforeach()

# 측정 도구 소스 파일
set(MEASUREMENT_TOOL_SOURCES
    "src/tools/MeasurementTool.cpp"
    "src/tools/MeasurementOverlay.cpp"
    "src/tools/MeasurementPlugin.cpp"
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
    "include/airphoto_viewer/*/*/*.hpp"
    "include/airphoto_viewer/*/*/*.h"
    "${CMAKE_CURRENT_BINARY_DIR}/airphoto_measurement_qmltyperegistrations.cpp"
)

# UI 관련 파일 그룹 생성
source_group("UI" FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/include/airphoto_viewer/ui/main_window.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/main_window.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/airphoto_viewer/ui/tile_view.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/tile_view.cpp"
)

# 코어 헤더 파일 그룹
source_group("Core" FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/include/airphoto_viewer/core/image_loader.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/image_loader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/airphoto_viewer/core/tile.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/airphoto_viewer/core/tile_cache.h"
)

# QML 모듈 컴파일 디렉토리 추가
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/airphoto_measurement_autogen/include_
)

# Create a library for the application code
add_library(airphoto_lib
    ${SOURCES}
    ${UI_HEADERS}
)

# Set target properties for the library
target_compile_definitions(airphoto_lib PRIVATE
    AIRPHOTO_VIEWER_LIBRARY
)

# Set include directories for the library
target_include_directories(airphoto_lib
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${EXIV2_INCLUDE_DIRS}
    ${TIFF_INCLUDE_DIRS}
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Gui_INCLUDE_DIRS}
    ${Qt6Widgets_INCLUDE_DIRS}
    ${Qt6Qml_INCLUDE_DIRS}
)

# Link libraries to the library
target_link_libraries(airphoto_lib
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    ${OpenCV_LIBS}
    ${EXIV2_LIBS}
    ${TIFF_LIBRARIES}
    nlohmann_json::nlohmann_json
    ${JPEG_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${ICONV_LIBRARY}
    ${LZMA_LIBRARY}
    ${WEBP_LIBRARY}
)

# Create the main executable
add_executable(airphoto_viewer src/main.cpp)

# Set target properties for the executable
set_target_properties(airphoto_viewer PROPERTIES
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/cmake/macOS/Info.plist.in"
    MACOSX_BUNDLE_ICON_FILE "AppIcon.icns"
    MACOSX_BUNDLE_BUNDLE_NAME "AirPhotoViewer"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0.0"
    MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
    MACOSX_BUNDLE_COPYRIGHT "Copyright © 2023 AirPhotoTeam"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.airphototeam.airphotoviewer"
)

# Install the application bundle
if(APPLE)
    install(TARGETS airphoto_viewer
        BUNDLE DESTINATION .
        RUNTIME DESTINATION bin
    )
else()
    install(TARGETS airphoto_viewer
        RUNTIME DESTINATION bin
    )
endif()

# Link the library to the executable
target_link_libraries(airphoto_viewer
    PRIVATE
    airphoto_lib
)

# 리소스 디렉토리 설정
set(RESOURCES_DIR "${CMAKE_SOURCE_DIR}/resources")
if(NOT EXISTS ${RESOURCES_DIR})
    file(MAKE_DIRECTORY ${RESOURCES_DIR})
endif()

# 리소스 파일 설치
if(EXISTS ${RESOURCES_DIR})
    install(DIRECTORY ${RESOURCES_DIR}/
        DESTINATION "${CMAKE_INSTALL_PREFIX}/resources"
        COMPONENT Runtime
    )
endif()

# 테스트 실행 파일 설정
if(BUILD_TESTING)
    enable_testing()
    
    # Qt Widgets 모듈 필요
    find_package(Qt6 REQUIRED COMPONENTS Test Widgets)
    
    # 테스트에 필요한 소스 파일
    set(TEST_SOURCES
        test/test_main_window.cpp
    )
    
    # 테스트 실행 파일 생성
    add_executable(airphoto_viewer_test
        ${TEST_SOURCES}
    )
    
    # 테스트 실행 파일 링크 라이브러리
    target_link_libraries(airphoto_viewer_test
        PRIVATE
        Qt6::Test
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        ${OpenCV_LIBS}
        ${EXIV2_LIBRARIES}
        ${TIFF_LIBRARIES}
        airphoto_lib  # 메인 라이브러리 링크
    )
    
    # 테스트 추가
    add_test(NAME main_window_test
        COMMAND $<TARGET_FILE:airphoto_viewer_test>
    )
    
    # 테스트 디렉토리 포함
    target_include_directories(airphoto_viewer_test
        PRIVATE
        ${CMAKE_SOURCE_DIR}/test
        ${CMAKE_SOURCE_DIR}/include
    )
    
    # 테스트 파일 그룹
    source_group("Test" FILES
        ${TEST_SOURCES}
    )
    
    # 테스트 컴파일 플래그 설정
    if(MSVC)
        target_compile_options(airphoto_viewer_test PRIVATE /W4 /WX)
    else()
        target_compile_options(airphoto_viewer_test PRIVATE -Wall -Wextra -Werror)
    endif()
    
    message(STATUS "Test source files: ${TEST_SOURCES}")
endif()

# 디버그 정보 출력 (선택사항)
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "Exiv2 include dir: ${EXIV2_INCLUDE_DIR}")
message(STATUS "TIFF libraries: ${TIFF_LINK_LIBRARIES}")
message(STATUS "Source files: ${SOURCES}")

# QML 모듈 관련 타겟 제거로 인해 주석 처리
# install(TARGETS airphoto_measurement
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#     ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
# )

# 예제 애플리케이션 (주석 처리)
# add_executable(measurement_example examples/MeasurementExample.qml)
# set_target_properties(measurement_example PROPERTIES
#     WIN32_EXECUTABLE TRUE
#     MACOSX_BUNDLE TRUE
# )

# target_link_libraries(measurement_example
#     PRIVATE
#     Qt6::Core
#     Qt6::Gui
#     Qt6::Qml
#     Qt6::Quick
#     Qt6::QuickControls2
# )

# QML 관련 설정 주석 처리
# target_compile_definitions(measurement_example
#     PRIVATE
#     QT_QML_DEBUG
#     QT_DECLARATIVE_DEBUG
# )

# QML 모듈 임포트 경로 설정 (개발 중에는 빌드 디렉토리 사용)
# set(QML_IMPORT_PATH "${CMAKE_CURRENT_BINARY_DIR}")
# set(QML2_IMPORT_PATH "${CMAKE_CURRENT_BINARY_DIR}")
# set(QT_QML_MODULES "${CMAKE_CURRENT_BINARY_DIR}")

# QML 모듈 임포트 경로를 환경 변수로 설정
# set(ENV{QML2_IMPORT_PATH} "${QML_IMPORT_PATH}")
# set(ENV{QT_QML_MODULES} "${QT_QML_MODULES}")

# Test executables - use unique target names and proper linking
set(TEST_METADATA_SOURCES
    "test/test_metadata.cpp"
)

add_executable(airphoto_test_metadata ${TEST_METADATA_SOURCES})
target_include_directories(airphoto_test_metadata
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${EXIV2_INCLUDE_DIRS}
    ${TIFF_INCLUDE_DIRS}
    ${Qt6Core_INCLUDE_DIRS}
)

target_link_libraries(airphoto_test_metadata
    PRIVATE
    airphoto_lib
    ${OpenCV_LIBS}
    ${EXIV2_LIBS}
    ${TIFF_LIBRARIES}
    nlohmann_json::nlohmann_json
    ${JPEG_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${ICONV_LIBRARY}
    ${LZMA_LIBRARY}
    ${WEBP_LIBRARY}
    Qt6::Core
    Qt6::Test
)

# Simple metadata test
set(TEST_METADATA_SIMPLE_SOURCES
    "test/test_metadata_simple.cpp"
)

add_executable(airphoto_test_metadata_simple ${TEST_METADATA_SIMPLE_SOURCES})
target_include_directories(airphoto_test_metadata_simple
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${EXIV2_INCLUDE_DIRS}
    ${TIFF_INCLUDE_DIRS}
    ${Qt6Core_INCLUDE_DIRS}
)

target_link_libraries(airphoto_test_metadata_simple
    PRIVATE
    airphoto_lib
    ${OpenCV_LIBS}
    ${EXIV2_LIBS}
    ${TIFF_LIBRARIES}
    nlohmann_json::nlohmann_json
    ${JPEG_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${ICONV_LIBRARY}
    ${LZMA_LIBRARY}
    ${WEBP_LIBRARY}
    Qt6::Core
    Qt6::Test
)

# TIFF loader test
set(TEST_TIFF_LOADER_SOURCES
    "test/test_tiff_loader.cpp"
)

add_executable(airphoto_test_tiff_loader ${TEST_TIFF_LOADER_SOURCES})
target_include_directories(airphoto_test_tiff_loader
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${EXIV2_INCLUDE_DIRS}
    ${TIFF_INCLUDE_DIRS}
    ${Qt6Core_INCLUDE_DIRS}
)

target_link_libraries(airphoto_test_tiff_loader
    PRIVATE
    airphoto_lib
    ${OpenCV_LIBS}
    ${EXIV2_LIBS}
    ${TIFF_LIBRARIES}
    nlohmann_json::nlohmann_json
    ${JPEG_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${ICONV_LIBRARY}
    ${LZMA_LIBRARY}
    ${WEBP_LIBRARY}
    Qt6::Core
    Qt6::Test
)

# Add tests if testing is enabled
if(BUILD_TESTING)
    add_test(NAME airphoto_test_metadata COMMAND airphoto_test_metadata)
    add_test(NAME airphoto_test_metadata_simple COMMAND airphoto_test_metadata_simple)
    add_test(NAME airphoto_test_tiff_loader COMMAND airphoto_test_tiff_loader)
    
    # Set test properties
    set_tests_properties(
        airphoto_test_metadata
        airphoto_test_metadata_simple
        airphoto_test_tiff_loader
        PROPERTIES
            ENVIRONMENT "DYLD_LIBRARY_PATH=$<TARGET_FILE_DIR:airphoto_lib>:$ENV{DYLD_LIBRARY_PATH}"
    )
endif()

# 설치 타겟 설정
if(APPLE)
    install(TARGETS airphoto_viewer
        BUNDLE DESTINATION .
        RUNTIME DESTINATION bin
    )
else()
    install(TARGETS airphoto_viewer
        RUNTIME DESTINATION bin
    )
endif()

# 테스트 디렉토리 추가 (나중에 사용)
# enable_testing()
# add_subdirectory(test)
