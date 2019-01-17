#include <napi.h>
#include "conecutter.hpp"

class ConeCutterJS : public Napi::ObjectWrap<ConeCutterJS>
{
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    ConeCutterJS(const Napi::CallbackInfo &info);
    ocl::ConeCutter* GetInternalInstance();

  private:
    static Napi::FunctionReference constructor;
    Napi::Value str(const Napi::CallbackInfo &info);
    ocl::ConeCutter actualClass_;
};