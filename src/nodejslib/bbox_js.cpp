#include "bbox.hpp"
#include "bbox_js.hpp"

Napi::FunctionReference BboxJS::constructor;

Napi::Object BboxJS::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "Bbox", {
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("Bbox", func);
    return exports;
}

BboxJS::BboxJS(const Napi::CallbackInfo &info) : Napi::ObjectWrap<BboxJS>(info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    actualClass_ = ocl::Bbox();
}

ocl::Bbox* BboxJS::GetInternalInstance()
{
    return &actualClass_;
}