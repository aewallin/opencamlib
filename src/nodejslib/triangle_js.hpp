#include "binding_utils.hpp"
#include "triangle.hpp"

class TriangleJS : public Napi::ObjectWrap<TriangleJS>
{
  public:
    static bool IsInstance(const Napi::Value& value) {
        return value.IsObject() && value.As<Napi::Object>().InstanceOf(constructor.Value());
    }
    static TriangleJS* UnwrapChecked(const Napi::Value& value) {
        if (!IsInstance(value)) throw Napi::TypeError::New(value.Env(), "Expected TriangleJS");
        return Napi::ObjectWrap<TriangleJS>::Unwrap(value.As<Napi::Object>());
    }

    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    TriangleJS(const Napi::CallbackInfo &info);
    ocl::Triangle *GetInternalInstance(const Napi::CallbackInfo &info);

  private:
    static Napi::FunctionReference constructor;
    std::unique_ptr<ocl::Triangle> actualClass_;
};