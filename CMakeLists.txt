cmake_minimum_required(VERSION 3.14)
project(RealCarMonitor)

# стандарт C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OpenCV через find_package
find_package(OpenCV REQUIRED)

# объявить исполняемый файл RealCarMonitor из всех .cpp файлов в src/
set(SOURCES
    src/main.cpp
    src/obd_parser.cpp
    src/onnx_classifier.cpp
    src/dashboard.cpp
    src/dms_monitor.cpp
    src/dms_hud.cpp
)

add_executable(RealCarMonitor ${SOURCES})


# ONNX Runtime по пути C:/onnxruntime
# find_package(onnxruntime REQUIRED PATHS "C:/onnxruntime" NO_DEFAULT_PATH)

# Подключаем найденные библиотеки к основному проекту
target_link_libraries(RealCarMonitor PRIVATE 
    ${OpenCV_LIBS}
    # onnxruntime::onnxruntime
)

# Добавляем пути к заголовочным файлам
target_include_directories(RealCarMonitor PRIVATE 
    ${OpenCV_INCLUDE_DIRS}
    src
    # "C:/onnxruntime/include"
)

# подключить Google Test через FetchContent
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

FetchContent_MakeAvailable(googletest)

enable_testing()

# объявить исполняемый файл RunTests для тестов
add_executable(RunTests tests/test_obd_parser.cpp)
target_link_libraries(RunTests PRIVATE gtest_main gtest)
include(GoogleTest)
gtest_discover_tests(RunTests)

