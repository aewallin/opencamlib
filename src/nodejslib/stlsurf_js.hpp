#include "binding_utils.hpp"
#include "stlsurf.hpp"

class STLSurfJS : public Napi::ObjectWrap<STLSurfJS>
{
  public:
    static bool IsInstance(const Napi::Value& value) {
        return value.IsObject() && value.As<Napi::Object>().InstanceOf(constructor.Value());
    }
    static STLSurfJS* UnwrapChecked(const Napi::Value& value) {
        if (!IsInstance(value)) throw Napi::TypeError::New(value.Env(), "Expected STLSurfJS");
        return Napi::ObjectWrap<STLSurfJS>::Unwrap(value.As<Napi::Object>());
    }

    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    STLSurfJS(const Napi::CallbackInfo &info);
    ocl::STLSurf *GetInternalInstance(const Napi::CallbackInfo &info);
    void addTriangle(const Napi::CallbackInfo &info);
    Napi::Value getTriangles(const Napi::CallbackInfo &info);
  private:
    static Napi::FunctionReference constructor;
    std::unique_ptr<ocl::STLSurf> actualClass_;
};