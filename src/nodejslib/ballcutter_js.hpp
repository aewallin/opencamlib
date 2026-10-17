#include "binding_utils.hpp"
#include "ballcutter.hpp"

class BallCutterJS : public Napi::ObjectWrap<BallCutterJS>
{
  public:
    static bool IsInstance(const Napi::Value& value) {
        return value.IsObject() && value.As<Napi::Object>().InstanceOf(constructor.Value());
    }
    static BallCutterJS* UnwrapChecked(const Napi::Value& value) {
        if (!IsInstance(value)) throw Napi::TypeError::New(value.Env(), "Expected BallCutterJS");
        return Napi::ObjectWrap<BallCutterJS>::Unwrap(value.As<Napi::Object>());
    }

    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    BallCutterJS(const Napi::CallbackInfo &info);
    ocl::BallCutter *GetInternalInstance(const Napi::CallbackInfo &info);

  private:
    static Napi::FunctionReference constructor;
    Napi::Value str(const Napi::CallbackInfo &info);
    std::unique_ptr<ocl::BallCutter> actualClass_;
};