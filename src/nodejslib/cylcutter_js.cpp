#include "cylcutter.hpp"
#include "cylcutter_js.hpp"

Napi::FunctionReference CylCutterJS::constructor;

Napi::Object CylCutterJS::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "CylCutter", {
        InstanceMethod("str", &CylCutterJS::str)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("CylCutter", func);
    return exports;
}

CylCutterJS::CylCutterJS(const Napi::CallbackInfo &info) : Napi::ObjectWrap<CylCutterJS>(info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    int length = info.Length();
    if (length != 2)
    {
        Napi::TypeError::New(env, "Provide 2 argument").ThrowAsJavaScriptException();
    }
    Napi::Number d = info[0].As<Napi::Number>();
    Napi::Number l = info[1].As<Napi::Number>();
    actualClass_ = ocl::CylCutter(d.DoubleValue(), l.DoubleValue());
}

Napi::Value CylCutterJS::str(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    return Napi::String::New(env, actualClass_.str());
}

ocl::CylCutter* CylCutterJS::GetInternalInstance()
{
    return &actualClass_;
}