#include "binding_utils.hpp"
#include "waterline.hpp"
#include "stlsurf.hpp"

class WaterlineJS : public Napi::ObjectWrap<WaterlineJS>
{
  public:
    static bool IsInstance(const Napi::Value& value) {
        return value.IsObject() && value.As<Napi::Object>().InstanceOf(constructor.Value());
    }
    static WaterlineJS* UnwrapChecked(const Napi::Value& value) {
        if (!IsInstance(value)) throw Napi::TypeError::New(value.Env(), "Expected WaterlineJS");
        return Napi::ObjectWrap<WaterlineJS>::Unwrap(value.As<Napi::Object>());
    }

    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    WaterlineJS(const Napi::CallbackInfo &info);
    void setZ(const Napi::CallbackInfo &info);
    void setSTL(const Napi::CallbackInfo &info);
    void setCylCutter(const Napi::CallbackInfo &info);
    void setBallCutter(const Napi::CallbackInfo &info);
    void setBullCutter(const Napi::CallbackInfo &info);
    void setConeCutter(const Napi::CallbackInfo &info);
    void setSampling(const Napi::CallbackInfo &info);
    void dispose(const Napi::CallbackInfo &info);
    void run(const Napi::CallbackInfo &info);
    Napi::Value getLoops(const Napi::CallbackInfo &info);
  private:
    Napi::ObjectReference surface_ref, cutter_ref, path_ref;
    static Napi::FunctionReference constructor;
    std::unique_ptr<ocl::Waterline> actualClass_;
};