#include "triangle.hpp"
#include "triangle_js.hpp"

Napi::FunctionReference TriangleJS::constructor;

Napi::Object TriangleJS::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "Triangle", {});
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("Triangle", func);
    return exports;
}

TriangleJS::TriangleJS(const Napi::CallbackInfo &info) : Napi::ObjectWrap<TriangleJS>(info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    actualClass_ = ocl::Triangle();
}

ocl::Triangle* TriangleJS::GetInternalInstance()
{
    return &actualClass_;
}