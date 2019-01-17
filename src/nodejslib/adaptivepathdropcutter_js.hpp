#include <napi.h>
#include "adaptivepathdropcutter.hpp"

class AdaptivePathDropCutterJS : public Napi::ObjectWrap<AdaptivePathDropCutterJS>
{
  public:
    static void Init(Napi::Env env, Napi::Object exports);
    AdaptivePathDropCutterJS(const Napi::CallbackInfo &info);
    ocl::AdaptivePathDropCutter *GetInternalInstance();
    void setSTL(const Napi::CallbackInfo &info);
    void setPath(const Napi::CallbackInfo &info);
    void setCylCutter(const Napi::CallbackInfo &info);
    void setBallCutter(const Napi::CallbackInfo &info);
    void setBullCutter(const Napi::CallbackInfo &info);
    void setConeCutter(const Napi::CallbackInfo &info);
    void setSampling(const Napi::CallbackInfo &info);
    void setMinSampling(const Napi::CallbackInfo &info);
    Napi::Value getCLPoints(const Napi::CallbackInfo &info);
    void run(const Napi::CallbackInfo &info);
  private:
    static Napi::FunctionReference constructor;
    ocl::AdaptivePathDropCutter actualClass_;
};