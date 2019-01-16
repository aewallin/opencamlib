#include "line.hpp"
#include "line_js.hpp"
#include "point_js.hpp"

Napi::FunctionReference LineJS::constructor;

Napi::Object LineJS::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "Line", {
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Line", func);
  return exports;
}

LineJS::LineJS(const Napi::CallbackInfo& info) : Napi::ObjectWrap<LineJS>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  int length = info.Length();
  if (length == 1) {
    Napi::TypeError::New(env, "Provide at least 2 arguments").ThrowAsJavaScriptException();
  }
  if (length == 2) {
    PointJS* p1in = Napi::ObjectWrap<PointJS>::Unwrap(info[0].As<Napi::Object>());
    PointJS* p2in = Napi::ObjectWrap<PointJS>::Unwrap(info[1].As<Napi::Object>());
    ocl::Point* p1 = p1in->GetInternalInstance();
    ocl::Point* p2 = p2in->GetInternalInstance();
    actualClass_ = ocl::Line(*p1, *p2);
  }
}

ocl::Line* LineJS::GetInternalInstance() {
  return &actualClass_;
}