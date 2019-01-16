#include "path.hpp"
#include "path_js.hpp"
#include "line_js.hpp"

Napi::FunctionReference PathJS::constructor;

Napi::Object PathJS::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "Path", {
    InstanceMethod("append", &PathJS::append),
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Path", func);
  return exports;
}

PathJS::PathJS(const Napi::CallbackInfo& info) : Napi::ObjectWrap<PathJS>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  actualClass_ = ocl::Path();
}

ocl::Path* PathJS::GetInternalInstance() {
  return &actualClass_;
}

void PathJS::append(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  if (info.Length() != 1) {
    Napi::TypeError::New(env, "Argument expected").ThrowAsJavaScriptException();
  }
  LineJS* ljs = Napi::ObjectWrap<LineJS>::Unwrap(info[0].As<Napi::Object>());
  ocl::Line* ocll = ljs->GetInternalInstance();
  actualClass_.append(*ocll);
}