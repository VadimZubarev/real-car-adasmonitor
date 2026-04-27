cmake_minimum_required(VERSION 3.15)
project(onnx_wrapper)

add_library(onnx_wrapper SHARED wrapper.cpp)

set(ONNXRUNTIME_DIR "C:/onnxruntime")

target_include_directories(onnx_wrapper PRIVATE
    ${ONNXRUNTIME_DIR}/include
)

target_link_directories(onnx_wrapper PRIVATE
    ${ONNXRUNTIME_DIR}/lib
)

target_link_libraries(onnx_wrapper PRIVATE
    onnxruntime
)