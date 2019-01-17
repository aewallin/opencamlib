#include <napi.h>
#include "ballcutter.hpp"

class BallCutterJS : public Napi::ObjectWrap<BallCutterJS>
{
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    BallCutterJS(const Napi::CallbackInfo &info);
    ocl::BallCutter* GetInternalInstance();

  private:
    static Napi::FunctionReference constructor;
    Napi::Value str(const Napi::CallbackInfo &info);
    ocl::BallCutter actualClass_;
};