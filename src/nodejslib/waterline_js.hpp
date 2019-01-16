#include <napi.h>
#include "waterline.hpp"

class WaterlineJS : public Napi::ObjectWrap<WaterlineJS>
{
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    WaterlineJS(const Napi::CallbackInfo &info);
    ocl::Waterline* GetInternalInstance();
    void setZ(const Napi::CallbackInfo &info);
    void setSTL(const Napi::CallbackInfo &info);
    void setCutter(const Napi::CallbackInfo &info);
    void setSampling(const Napi::CallbackInfo &info);
    void run(const Napi::CallbackInfo &info);
    Napi::Value getLoops(const Napi::CallbackInfo &info);
  private:
    static Napi::FunctionReference constructor;
    ocl::Waterline actualClass_;
};