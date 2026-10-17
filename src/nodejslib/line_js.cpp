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

  size_t length = info.Length();
  if (length != 2) {
    throw Napi::TypeError::New(env, "Provide at least 2 arguments");
  }
  else if (length == 2)
  {
    PointJS* p1in = PointJS::UnwrapChecked(info[0]);
    PointJS* p2in = PointJS::UnwrapChecked(info[1]);
    ocl::Point* p1 = p1in->GetInternalInstance(info);
    ocl::Point* p2 = p2in->GetInternalInstance(info);
    actualClass_ = std::make_unique<ocl::Line>(*p1, *p2);
  }
}

ocl::Line* LineJS::GetInternalInstance(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  return this->actualClass_.get();
}