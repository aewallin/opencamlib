#include "stlreader.hpp"
#include <napi.h>

class STLReaderJS : public Napi::ObjectWrap<STLReaderJS> {
  public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    STLReaderJS(const Napi::CallbackInfo &info);

  private:
    static Napi::FunctionReference constructor;
};