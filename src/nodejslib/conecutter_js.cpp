#include "conecutter_js.hpp"

Napi::FunctionReference ConeCutterJS::constructor;

Napi::Object ConeCutterJS::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "ConeCutter", {
        InstanceMethod("str", &ConeCutterJS::str)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("ConeCutter", func);
    return exports;
}

ConeCutterJS::ConeCutterJS(const Napi::CallbackInfo &info) : Napi::ObjectWrap<ConeCutterJS>(info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    int length = info.Length();
    if (length != 3)
    {
        Napi::TypeError::New(env, "Provide 3 argument").ThrowAsJavaScriptException();
    }
    Napi::Number d = info[0].As<Napi::Number>();
    Napi::Number a = info[1].As<Napi::Number>();
    Napi::Number l = info[2].As<Napi::Number>();
    actualClass_ = ocl::ConeCutter(d.DoubleValue(), a.DoubleValue(), l.DoubleValue());
}

Napi::Value ConeCutterJS::str(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    return Napi::String::New(env, actualClass_.str());
}

ocl::ConeCutter* ConeCutterJS::GetInternalInstance()
{
    return &actualClass_;
}