#include "ballcutter_js.hpp"

Napi::FunctionReference BallCutterJS::constructor;

Napi::Object BallCutterJS::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "BallCutter", {
        InstanceMethod("str", &BallCutterJS::str)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("BallCutter", func);
    return exports;
}

BallCutterJS::BallCutterJS(const Napi::CallbackInfo &info) : Napi::ObjectWrap<BallCutterJS>(info)
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
    actualClass_ = ocl::BallCutter(d.DoubleValue(), l.DoubleValue());
}

Napi::Value BallCutterJS::str(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    return Napi::String::New(env, actualClass_.str());
}

ocl::BallCutter* BallCutterJS::GetInternalInstance()
{
    return &actualClass_;
}