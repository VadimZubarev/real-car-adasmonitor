#include "wrapper.h"

#include <windows.h> 
#include <onnxruntime_c_api.h>
#include <string>
#include <vector>
#include <stdio.h>

static const OrtApi* g_ort = nullptr;

// ======================= INIT =======================

static void init_ort()
{
    if (!g_ort)
        g_ort = OrtGetApiBase()->GetApi(ORT_API_VERSION);
}

// ======================= UTF8 → WCHAR =======================

static std::wstring to_wstring(const char* str)
{
    if (!str) return L"";

    int size_needed = MultiByteToWideChar(
        CP_UTF8,
        0,
        str,
        -1,
        nullptr,
        0
    );

    std::wstring wstr(size_needed, 0);

    MultiByteToWideChar(
        CP_UTF8,
        0,
        str,
        -1,
        &wstr[0],
        size_needed
    );

    return wstr;
}

// ======================= CREATE SESSION =======================

extern "C"
__declspec(dllexport)
void* ort_create_session(const char* model_path)
{
    init_ort();

    OrtEnv* env = nullptr;
    OrtSessionOptions* options = nullptr;
    OrtSession* session = nullptr;

    g_ort->CreateEnv(ORT_LOGGING_LEVEL_WARNING, "wrapper", &env);
    g_ort->CreateSessionOptions(&options);

    std::wstring w_model = to_wstring(model_path);

    OrtStatus* status = g_ort->CreateSession(
        env,
        w_model.c_str(),
        options,
        &session
    );

    if (status != nullptr)
    {
        printf("Failed to create ONNX session\n");
        g_ort->ReleaseStatus(status);
        return nullptr;
    }

    OrtAllocator* allocator;
    g_ort->GetAllocatorWithDefaultOptions(&allocator);

    char* input_name;
    g_ort->SessionGetInputName(session, 0, allocator, &input_name);
    printf("INPUT NAME: %s\n", input_name);

    char* output_name;
    g_ort->SessionGetOutputName(session, 0, allocator, &output_name);
    printf("OUTPUT NAME: %s\n", output_name);

    // можно освободить (опционально)
    g_ort->AllocatorFree(allocator, input_name);
    g_ort->AllocatorFree(allocator, output_name);

    return (void*)session;
}

// ======================= DESTROY =======================

extern "C"
__declspec(dllexport)
void ort_release_session(void* session)
{
    init_ort();

    if (!session) return;

    g_ort->ReleaseSession((OrtSession*)session);
}

// ======================= PREDICT =======================

extern "C"
__declspec(dllexport)
void ort_predict(
    void* session,
    const float* input,
    float* output,
    int size
)
{
    init_ort();

    if (!session || !input || !output)
        return;

    OrtSession* sess = (OrtSession*)session;

    // ===== Memory Info =====
    OrtMemoryInfo* mem_info = nullptr;

    g_ort->CreateMemoryInfo(
        "Cpu",
        OrtArenaAllocator,
        0,
        OrtMemTypeDefault,
        &mem_info
    );

    // ===== Input tensor =====
    int64_t shape[2] = {1, size};

    OrtValue* input_tensor = nullptr;

    g_ort->CreateTensorWithDataAsOrtValue(
        mem_info,
        (void*)input,
        size * sizeof(float),
        shape,
        2,
        ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT,
        &input_tensor
    );

    const char* input_names[] = {"features"};
    const char* output_names[] = {"class_scores"};

    OrtValue* output_tensor = nullptr;

    g_ort->Run(
        sess,
        nullptr,
        input_names,
        (const OrtValue* const*)&input_tensor,
        1,
        output_names,
        1,
        &output_tensor
    );

    // ===== Extract output =====
    float* logits = nullptr;

    g_ort->GetTensorMutableData(
        output_tensor,
        (void**)&logits
    );

    // допустим у тебя 3 класса
    for (int i = 0; i < 3; i++)
        output[i] = logits[i];

    // ===== Cleanup =====
    g_ort->ReleaseValue(input_tensor);
    g_ort->ReleaseValue(output_tensor);
    g_ort->ReleaseMemoryInfo(mem_info);
}