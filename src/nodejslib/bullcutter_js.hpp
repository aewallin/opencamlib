#include "binding_utils.hpp"
#include "bullcutter.hpp"

class BullCutterJS : public Napi::ObjectWrap<BullCutterJS>
{
  public:
    static bool IsInstance(const Napi::Value& value) {
        return value.IsObject() && value.As<Napi::Object>().InstanceOf(constructor.Value());
    }
    static BullCutterJS* UnwrapChecked(const Napi::Value& value) {
        if (!IsInstance(value)) throw Napi::TypeError::New(value.Env(), "Expected BullCutterJS");
        return Napi::ObjectWrap<BullCutterJS>::Unwrap(value.As<Napi::Object>());
    }

    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    BullCutterJS(const Napi::CallbackInfo &info);
    ocl::BullCutter *GetInternalInstance(const Napi::CallbackInfo &info);

  private:
    static Napi::FunctionReference constructor;
    Napi::Value str(const Napi::CallbackInfo &info);
    std::unique_ptr<ocl::BullCutter> actualClass_;
};