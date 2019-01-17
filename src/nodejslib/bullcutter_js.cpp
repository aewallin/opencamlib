#include "bullcutter_js.hpp"

Napi::FunctionReference BullCutterJS::constructor;

Napi::Object BullCutterJS::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "BullCutter", {
        InstanceMethod("str", &BullCutterJS::str)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("BullCutter", func);
    return exports;
}

BullCutterJS::BullCutterJS(const Napi::CallbackInfo &info) : Napi::ObjectWrap<BullCutterJS>(info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    int length = info.Length();
    if (length != 3)
    {
        Napi::TypeError::New(env, "Provide 3 argument").ThrowAsJavaScriptException();
    }
    Napi::Number d = info[0].As<Napi::Number>();
    Napi::Number r = info[1].As<Napi::Number>();
    Napi::Number l = info[2].As<Napi::Number>();
    actualClass_ = ocl::BullCutter(d.DoubleValue(), r.DoubleValue(), l.DoubleValue());
}

Napi::Value BullCutterJS::str(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    return Napi::String::New(env, actualClass_.str());
}

ocl::BullCutter* BullCutterJS::GetInternalInstance()
{
    return &actualClass_;
}