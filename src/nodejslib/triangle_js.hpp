#include <napi.h>
#include "triangle.hpp"

class TriangleJS : public Napi::ObjectWrap<TriangleJS>
{
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    TriangleJS(const Napi::CallbackInfo &info);
    ocl::Triangle *GetInternalInstance();

  private:
    static Napi::FunctionReference constructor;
    ocl::Triangle actualClass_;
};