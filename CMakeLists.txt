# CMake 最低版本要求
cmake_minimum_required(VERSION 3.16)

# 项目名称和版本
project(MeshLibDemo VERSION 1.0.0 LANGUAGES CXX)

# =============================================================================
# 注意：MeshLib 需要 C++20 标准
# =============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =============================================================================
# 设置 Qt 自动处理
# =============================================================================
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# =============================================================================
# 查找 Qt6 或 Qt5 组件
# =============================================================================
find_package(Qt6 QUIET COMPONENTS Core Gui Widgets)
if(NOT Qt6_FOUND)
    find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
    message(STATUS "Using Qt5")
else()
    message(STATUS "Using Qt6")
endif()

# =============================================================================
# MeshLib 安装路径配置
# 可通过命令行覆盖: cmake -DMESHLIB_INSTALL_DIR=C:/Your/Path ..
# =============================================================================
if(NOT DEFINED MESHLIB_INSTALL_DIR)
    set(MESHLIB_INSTALL_DIR "C:/DEV_ENV/Meshlib/install")
endif()

# 验证安装路径是否存在
if(NOT EXISTS ${MESHLIB_INSTALL_DIR})
    message(FATAL_ERROR "MeshLib 安装路径不存在: ${MESHLIB_INSTALL_DIR}\n"
                        "请通过 -DMESHLIB_INSTALL_DIR=xxx 指定正确的路径")
endif()

# =============================================================================
# 根据构建类型选择库目录
# =============================================================================
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(MESHLIB_LIB_DIR "${MESHLIB_INSTALL_DIR}/lib/Debug")
    set(MESHLIB_BIN_DIR "${MESHLIB_INSTALL_DIR}/app/Debug")
else()
    set(MESHLIB_LIB_DIR "${MESHLIB_INSTALL_DIR}/lib/Release")
    set(MESHLIB_BIN_DIR "${MESHLIB_INSTALL_DIR}/app/Release")
endif()

# 验证库目录是否存在
if(NOT EXISTS ${MESHLIB_LIB_DIR})
    message(FATAL_ERROR "MeshLib 库目录不存在: ${MESHLIB_LIB_DIR}")
endif()

message(STATUS "========================================")
message(STATUS "MeshLib Configuration:")
message(STATUS "  Install Dir: ${MESHLIB_INSTALL_DIR}")
message(STATUS "  Lib Dir: ${MESHLIB_LIB_DIR}")
message(STATUS "  Bin Dir: ${MESHLIB_BIN_DIR}")
message(STATUS "========================================")

# =============================================================================
# 收集所有源文件
# =============================================================================
set(SOURCES
    main.cpp
    MainWindow.cpp
    CutterVisualizer.cpp
    CylinderGenerator.cpp
    BooleanOperator.cpp
)

set(HEADERS
    MainWindow.h
    CutterVisualizer.h
    CylinderGenerator.h
    BooleanOperator.h
)

# =============================================================================
# 添加可执行文件
# =============================================================================
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# =============================================================================
# 配置包含目录
# =============================================================================
target_include_directories(${PROJECT_NAME} PRIVATE
    ${MESHLIB_INSTALL_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# =============================================================================
# 配置库目录
# =============================================================================
target_link_directories(${PROJECT_NAME} PRIVATE
    ${MESHLIB_LIB_DIR}
)

# =============================================================================
# 链接 MeshLib 核心库和必要的第三方库
# 注意：库的顺序很重要，被依赖的库应该放在后面
# =============================================================================
# 根据构建类型选择正确的第三方库名（Debug 版本带有后缀）
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(TBB_LIB tbb12_debug)
    set(SPDLOG_LIB spdlogd)
    set(FMT_LIB fmtd)
else()
    set(TBB_LIB tbb12)
    set(SPDLOG_LIB spdlog)
    set(FMT_LIB fmt)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    MRMesh          # MeshLib 核心库（必须）
    MRViewer        # MeshLib 查看器库
    MRMeshC         # MeshLib C API
    MRPch           # 预编译头支持
    ${TBB_LIB}      # Intel TBB 并行库
    ${SPDLOG_LIB}   # 日志库
    ${FMT_LIB}      # 格式化库
    gdi32           # Windows GDI
    opengl32        # OpenGL
)

# =============================================================================
# Windows 特定设置
# =============================================================================
if(WIN32)
    # 定义必要的宏
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _WINDOWS
        NOMINMAX        # 防止 Windows 的 min/max 宏冲突
        WIN32_LEAN_AND_MEAN
        HAS_MRVIEWER
    )

    # 添加 UTF-8 编译支持（fmt 库需要）
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
        message(STATUS "MSVC UTF-8 support enabled (/utf-8)")
    endif()

    # 设置运行时库（与 MeshLib 保持一致：动态多线程 DLL）
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_property(TARGET ${PROJECT_NAME} PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
        # Debug 模式添加调试宏
        target_compile_definitions(${PROJECT_NAME} PRIVATE _DEBUG)
    else()
        set_property(TARGET ${PROJECT_NAME} PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
        # Release 模式添加 NDEBUG
        target_compile_definitions(${PROJECT_NAME} PRIVATE NDEBUG)
    endif()
endif()

# =============================================================================
# 设置输出目录
# =============================================================================
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
)

# =============================================================================
# 复制 DLL 文件到输出目录
# =============================================================================
file(GLOB MESHLIB_DLLS "${MESHLIB_BIN_DIR}/*.dll")

if(MESHLIB_DLLS)
    message(STATUS "Found DLLs to copy:")
    foreach(dll ${MESHLIB_DLLS})
        get_filename_component(dll_name ${dll} NAME)
        message(STATUS "  - ${dll_name}")
    endforeach()

    # 添加自定义命令，在每次构建后复制 DLL
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying MeshLib DLLs to output directory..."
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${MESHLIB_DLLS}
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying MeshLib runtime DLLs"
    )
else()
    message(WARNING "No DLLs found in ${MESHLIB_BIN_DIR}")
endif()

# 复制 Qt DLLs (Windows)
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying Qt DLLs..."
        COMMAND Qt::qmake -query QT_INSTALL_BINS > qt_bin_path.txt
    )
endif()

# =============================================================================
# 配置摘要
# =============================================================================
message(STATUS "")
message(STATUS "========== Configuration Summary ==========")
message(STATUS "Project:        ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "C++ Standard:   ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "Output Dir:     ${CMAKE_BINARY_DIR}/bin")
message(STATUS "MRViewer:       ON")
message(STATUS "Qt:             ON")
message(STATUS "=========================================")
message(STATUS "")

# =============================================================================
# 使用说明
# =============================================================================
message(STATUS "Build Instructions:")
message(STATUS "  1. mkdir build && cd build")
message(STATUS "  2. cmake .. -G \"Visual Studio 17 2022\" -A x64")
message(STATUS "  3. cmake --build . --config Release")
message(STATUS "  Or open the folder in VS Code with CMake Tools extension")
