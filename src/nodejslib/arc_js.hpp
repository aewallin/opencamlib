#pragma once
#include "binding_utils.hpp"
#include "arc.hpp"

class ArcJS : public Napi::ObjectWrap<ArcJS> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    ArcJS(const Napi::CallbackInfo& info);
    static bool IsInstance(const Napi::Value& value) {
        return value.IsObject() && value.As<Napi::Object>().InstanceOf(constructor.Value());
    }
    static ArcJS* UnwrapChecked(const Napi::Value& value) {
        if (!IsInstance(value)) throw Napi::TypeError::New(value.Env(), "Expected Arc");
        return Unwrap(value.As<Napi::Object>());
    }
    ocl::Arc* GetInternalInstance(const Napi::CallbackInfo&) { return actualClass_.get(); }
private:
    static Napi::FunctionReference constructor;
    std::unique_ptr<ocl::Arc> actualClass_;
};
