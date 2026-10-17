#include "binding_utils.hpp"
#include "cylcutter.hpp"

class CylCutterJS : public Napi::ObjectWrap<CylCutterJS>
{
  public:
    static bool IsInstance(const Napi::Value& value) {
        return value.IsObject() && value.As<Napi::Object>().InstanceOf(constructor.Value());
    }
    static CylCutterJS* UnwrapChecked(const Napi::Value& value) {
        if (!IsInstance(value)) throw Napi::TypeError::New(value.Env(), "Expected CylCutterJS");
        return Napi::ObjectWrap<CylCutterJS>::Unwrap(value.As<Napi::Object>());
    }

    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    CylCutterJS(const Napi::CallbackInfo &info);
    ocl::CylCutter *GetInternalInstance(const Napi::CallbackInfo &info);

  private:
    static Napi::FunctionReference constructor;
    Napi::Value str(const Napi::CallbackInfo &info);
    std::unique_ptr<ocl::CylCutter> actualClass_;
};