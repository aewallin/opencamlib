#include "binding_utils.hpp"
#include "bbox.hpp"

class BboxJS : public Napi::ObjectWrap<BboxJS>
{
  public:
    static bool IsInstance(const Napi::Value& value) {
        return value.IsObject() && value.As<Napi::Object>().InstanceOf(constructor.Value());
    }
    static BboxJS* UnwrapChecked(const Napi::Value& value) {
        if (!IsInstance(value)) throw Napi::TypeError::New(value.Env(), "Expected BboxJS");
        return Napi::ObjectWrap<BboxJS>::Unwrap(value.As<Napi::Object>());
    }

    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    BboxJS(const Napi::CallbackInfo &info);
    ocl::Bbox *GetInternalInstance(const Napi::CallbackInfo &info);

  private:
    static Napi::FunctionReference constructor;
    std::unique_ptr<ocl::Bbox> actualClass_;
};