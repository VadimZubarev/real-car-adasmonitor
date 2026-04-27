#pragma once

#ifdef _WIN32
#define API __declspec(dllexport)
#else
#define API
#endif

extern "C" {

API void* create_model(const char* model_path,
                       const char* norm_path);

API void destroy_model(void* handle);

API void predict(void* handle,
                 const float* input,
                 float* output,
                 int size);

}