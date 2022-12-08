#include "point.hpp"
#include "point_js.hpp"

Napi::FunctionReference PointJS::constructor;

Napi::Object PointJS::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "Point", {
    InstanceMethod("add", &PointJS::Add),
    InstanceAccessor("x", &PointJS::GetX, &PointJS::SetX),
    InstanceAccessor("y", &PointJS::GetY, &PointJS::SetY),
    InstanceAccessor("z", &PointJS::GetZ, &PointJS::SetZ),
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();

  exports.Set("Point", func);
  return exports;
}

Napi::Value PointJS::GetX(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  return Napi::Number::New(env, this->actualClass_->x);
}

void PointJS::SetX(const Napi::CallbackInfo& info, const Napi::Value& value) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Napi::Number x = info[0].As<Napi::Number>();
  this->actualClass_->x = x.DoubleValue();
}

Napi::Value PointJS::GetY(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  return Napi::Number::New(env, this->actualClass_->y);
}

void PointJS::SetY(const Napi::CallbackInfo& info, const Napi::Value& value) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Napi::Number y = info[0].As<Napi::Number>();
  this->actualClass_->y = y.DoubleValue();
}

Napi::Value PointJS::GetZ(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  return Napi::Number::New(env, this->actualClass_->z);
}

void PointJS::SetZ(const Napi::CallbackInfo& info, const Napi::Value& value) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  Napi::Number z = info[0].As<Napi::Number>();
  this->actualClass_->z = z.DoubleValue();
}

PointJS::PointJS(const Napi::CallbackInfo& info) : Napi::ObjectWrap<PointJS>(info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);

  size_t length = static_cast<int>(info.Length());
  if (length == 1) {
    Napi::TypeError::New(env, "Provide at least 2 arguments").ThrowAsJavaScriptException();
  }
  if (length == 2) {
    Napi::Number x = info[0].As<Napi::Number>();
    Napi::Number y = info[1].As<Napi::Number>();
    this->actualClass_ = new ocl::Point(x.DoubleValue(), y.DoubleValue());
  } else if (length == 3) {
    Napi::Number x = info[0].As<Napi::Number>();
    Napi::Number y = info[1].As<Napi::Number>();
    Napi::Number z = info[2].As<Napi::Number>();
    this->actualClass_ = new ocl::Point(x.DoubleValue(), y.DoubleValue(), z.DoubleValue());
  }
}

ocl::Point *PointJS::GetInternalInstance(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  return this->actualClass_;
}

void PointJS::Add(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  if (info.Length() != 1) {
    Napi::TypeError::New(env, "Argument expected").ThrowAsJavaScriptException();
  }
  PointJS* pjs = Napi::ObjectWrap<PointJS>::Unwrap(info[0].As<Napi::Object>());
  ocl::Point* oclp = pjs->GetInternalInstance(info);
  ocl::Point p = *oclp;
  this->actualClass_->operator+=(ocl::Point(p.x, p.y, p.z));
}