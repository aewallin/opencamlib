#include <napi.h>
#include "stlsurf.hpp"

class STLSurfJS : public Napi::ObjectWrap<STLSurfJS>
{
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    STLSurfJS(const Napi::CallbackInfo &info);
    ocl::STLSurf* GetInternalInstance();
    Napi::Value getTriangles(const Napi::CallbackInfo &info);
  private:
    static Napi::FunctionReference constructor;
    ocl::STLSurf actualClass_;
};