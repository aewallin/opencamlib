#include <napi.h>
#include "bbox.hpp"

class BboxJS : public Napi::ObjectWrap<BboxJS>
{
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    BboxJS(const Napi::CallbackInfo &info);
    ocl::Bbox *GetInternalInstance(const Napi::CallbackInfo &info);

  private:
    static Napi::FunctionReference constructor;
    ocl::Bbox *actualClass_;
};