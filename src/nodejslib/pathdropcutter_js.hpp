#include "binding_utils.hpp"
#include "pathdropcutter.hpp"

class PathDropCutterJS : public Napi::ObjectWrap<PathDropCutterJS>
{
  public:
    static bool IsInstance(const Napi::Value& value) {
        return value.IsObject() && value.As<Napi::Object>().InstanceOf(constructor.Value());
    }
    static PathDropCutterJS* UnwrapChecked(const Napi::Value& value) {
        if (!IsInstance(value)) throw Napi::TypeError::New(value.Env(), "Expected PathDropCutterJS");
        return Napi::ObjectWrap<PathDropCutterJS>::Unwrap(value.As<Napi::Object>());
    }

    static void Init(Napi::Env env, Napi::Object exports);
    PathDropCutterJS(const Napi::CallbackInfo &info);
    void setSTL(const Napi::CallbackInfo &info);
    void setPath(const Napi::CallbackInfo &info);
    void setCylCutter(const Napi::CallbackInfo &info);
    void setBallCutter(const Napi::CallbackInfo &info);
    void setBullCutter(const Napi::CallbackInfo &info);
    void setConeCutter(const Napi::CallbackInfo &info);
    void setSampling(const Napi::CallbackInfo &info);
    void setZ(const Napi::CallbackInfo &info);
    Napi::Value getCLPoints(const Napi::CallbackInfo &info);
    void dispose(const Napi::CallbackInfo &info);
    void run(const Napi::CallbackInfo &info);

  private:
    Napi::ObjectReference surface_ref, cutter_ref, path_ref;
    static Napi::FunctionReference constructor;
    std::unique_ptr<ocl::PathDropCutter> actualClass_;
};