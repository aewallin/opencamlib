#include <napi.h>
#include "pathdropcutter.hpp"

class PathDropCutterJS : public Napi::ObjectWrap<PathDropCutterJS>
{
  public:
    static void Init(Napi::Env env, Napi::Object exports);
    PathDropCutterJS(const Napi::CallbackInfo &info);
    ocl::PathDropCutter *GetInternalInstance();
    void setSTL(const Napi::CallbackInfo &info);
    void setPath(const Napi::CallbackInfo &info);
    void setCylCutter(const Napi::CallbackInfo &info);
    void setBallCutter(const Napi::CallbackInfo &info);
    void setBullCutter(const Napi::CallbackInfo &info);
    void setConeCutter(const Napi::CallbackInfo &info);
    void setSampling(const Napi::CallbackInfo &info);
    Napi::Value getCLPoints(const Napi::CallbackInfo &info);
    void run(const Napi::CallbackInfo &info);

  private:
    static Napi::FunctionReference constructor;
    ocl::PathDropCutter actualClass_;
};