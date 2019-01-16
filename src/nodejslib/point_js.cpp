#include "point.hpp"
#include "point_js.hpp"

Napi::FunctionReference PointJS::constructor;

Napi::Object PointJS::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "Point", {
    InstanceMethod("add", &PointJS::Add),
    InstanceAccessor("x", &PointJS::GetX, &PointJS::SetX)
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Point", func);
  return exports;
}

Napi::Value PointJS::GetX(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return Napi::Number::New(env, actualClass_.x);
}

void PointJS::SetX(const Napi::CallbackInfo& info, const Napi::Value& value) {
  Napi::Number x = info[0].As<Napi::Number>();
  actualClass_.x = x.DoubleValue();
}

PointJS::PointJS(const Napi::CallbackInfo& info) : Napi::ObjectWrap<PointJS>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  int length = info.Length();
  if (length == 1) {
    Napi::TypeError::New(env, "Provide at least 2 arguments").ThrowAsJavaScriptException();
  }
  if (length == 2) {
    Napi::Number x = info[0].As<Napi::Number>();    
    Napi::Number y = info[1].As<Napi::Number>();
    actualClass_ = ocl::Point(x.DoubleValue(), y.DoubleValue());
  } else if (length == 3) {
    Napi::Number x = info[0].As<Napi::Number>();    
    Napi::Number y = info[1].As<Napi::Number>();
    Napi::Number z = info[2].As<Napi::Number>();
    actualClass_ = ocl::Point(x.DoubleValue(), y.DoubleValue(), z.DoubleValue());
  }
}

ocl::Point* PointJS::GetInternalInstance() {
  return &actualClass_;
}

void PointJS::Add(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  if (info.Length() != 1) {
    Napi::TypeError::New(env, "Argument expected").ThrowAsJavaScriptException();
  }
  PointJS* pjs = Napi::ObjectWrap<PointJS>::Unwrap(info[0].As<Napi::Object>());
  ocl::Point* oclp = pjs->GetInternalInstance();
  ocl::Point p = *oclp;
  actualClass_.operator+=(ocl::Point(p.x, p.y, p.z));
}