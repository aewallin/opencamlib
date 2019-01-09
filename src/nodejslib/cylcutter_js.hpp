#include <napi.h>
#include "cylcutter.hpp"

class CylCutterJS : public Napi::ObjectWrap<CylCutterJS>
{
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    CylCutterJS(const Napi::CallbackInfo &info);
    ocl::CylCutter* GetInternalInstance();

  private:
    static Napi::FunctionReference constructor;
    Napi::Value str(const Napi::CallbackInfo &info);
    ocl::CylCutter actualClass_;
};