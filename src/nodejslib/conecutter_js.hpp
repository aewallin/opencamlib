#include "binding_utils.hpp"
#include "conecutter.hpp"

class ConeCutterJS : public Napi::ObjectWrap<ConeCutterJS>
{
  public:
    static bool IsInstance(const Napi::Value& value) {
        return value.IsObject() && value.As<Napi::Object>().InstanceOf(constructor.Value());
    }
    static ConeCutterJS* UnwrapChecked(const Napi::Value& value) {
        if (!IsInstance(value)) throw Napi::TypeError::New(value.Env(), "Expected ConeCutterJS");
        return Napi::ObjectWrap<ConeCutterJS>::Unwrap(value.As<Napi::Object>());
    }

    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    ConeCutterJS(const Napi::CallbackInfo &info);
    ocl::ConeCutter *GetInternalInstance(const Napi::CallbackInfo &info);

  private:
    static Napi::FunctionReference constructor;
    Napi::Value str(const Napi::CallbackInfo &info);
    std::unique_ptr<ocl::ConeCutter> actualClass_;
};