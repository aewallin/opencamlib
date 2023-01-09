#include "stlsurf.hpp"
#include "waterline.hpp"
#include <napi.h>

class WaterlineJS : public Napi::ObjectWrap<WaterlineJS> {
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    WaterlineJS(const Napi::CallbackInfo &info);
    void setZ(const Napi::CallbackInfo &info);
    void setSTL(const Napi::CallbackInfo &info);
    void setCylCutter(const Napi::CallbackInfo &info);
    void setBallCutter(const Napi::CallbackInfo &info);
    void setBullCutter(const Napi::CallbackInfo &info);
    void setConeCutter(const Napi::CallbackInfo &info);
    void setSampling(const Napi::CallbackInfo &info);
    void run(const Napi::CallbackInfo &info);
    Napi::Value getLoops(const Napi::CallbackInfo &info);

  private:
    static Napi::FunctionReference constructor;
    ocl::Waterline *actualClass_;
};