#include <napi.h>
#include "adaptivewaterline.hpp"

class AdaptiveWaterlineJS : public Napi::ObjectWrap<AdaptiveWaterlineJS>
{
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    AdaptiveWaterlineJS(const Napi::CallbackInfo &info);
    ocl::AdaptiveWaterline *GetInternalInstance();
    void setZ(const Napi::CallbackInfo &info);
    void setSTL(const Napi::CallbackInfo &info);
    void setCylCutter(const Napi::CallbackInfo &info);
    void setBallCutter(const Napi::CallbackInfo &info);
    void setBullCutter(const Napi::CallbackInfo &info);
    void setConeCutter(const Napi::CallbackInfo &info);
    void setSampling(const Napi::CallbackInfo &info);
    void setMinSampling(const Napi::CallbackInfo &info);
    void run(const Napi::CallbackInfo &info);
    Napi::Value getLoops(const Napi::CallbackInfo &info);

  private:
    static Napi::FunctionReference constructor;
    ocl::AdaptiveWaterline actualClass_;
};