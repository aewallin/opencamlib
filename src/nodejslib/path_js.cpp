#include "path.hpp"
#include "path_js.hpp"
#include "line_js.hpp"
#include "arc_js.hpp"

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

  this->actualClass_ = std::make_unique<ocl::Path>();
}

ocl::Path *PathJS::GetInternalInstance(const Napi::CallbackInfo &info)
{
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  return this->actualClass_.get();
}

void PathJS::append(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::HandleScope scope(env);
  if (info.Length() != 1) {
    throw Napi::TypeError::New(env, "Argument expected");
  }
  nativeCall(env, [&] {
      if (ArcJS::IsInstance(info[0]))
          actualClass_->append(*ArcJS::UnwrapChecked(info[0])->GetInternalInstance(info));
      else
          actualClass_->append(*LineJS::UnwrapChecked(info[0])->GetInternalInstance(info));
  });
}
