#include <napi.h>
#include "bullcutter.hpp"

class BullCutterJS : public Napi::ObjectWrap<BullCutterJS>
{
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    BullCutterJS(const Napi::CallbackInfo &info);
    ocl::BullCutter* GetInternalInstance();

  private:
    static Napi::FunctionReference constructor;
    Napi::Value str(const Napi::CallbackInfo &info);
    ocl::BullCutter actualClass_;
};