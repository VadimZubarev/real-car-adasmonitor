#pragma once

#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) void* ort_create_session(const char* model_path);
__declspec(dllexport) void ort_release_session(void* session);
__declspec(dllexport) void ort_predict(void* session, const float* input, float* output, int size);

#ifdef __cplusplus
}
#endif