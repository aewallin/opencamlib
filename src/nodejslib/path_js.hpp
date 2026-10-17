#include "binding_utils.hpp"
#include "path.hpp"

class PathJS : public Napi::ObjectWrap<PathJS> {
 public:
    static bool IsInstance(const Napi::Value& value) {
        return value.IsObject() && value.As<Napi::Object>().InstanceOf(constructor.Value());
    }
    static PathJS* UnwrapChecked(const Napi::Value& value) {
        if (!IsInstance(value)) throw Napi::TypeError::New(value.Env(), "Expected PathJS");
        return Napi::ObjectWrap<PathJS>::Unwrap(value.As<Napi::Object>());
    }

  static Napi::Object Init(Napi::Env env, Napi::Object exports); // Init function for setting the export key to JS
  PathJS(const Napi::CallbackInfo& info); // Constructor to initialise
  ocl::Path *GetInternalInstance(const Napi::CallbackInfo &info);

private:
  static Napi::FunctionReference constructor; //reference to store the class definition that needs to be exported to JS
  void append(const Napi::CallbackInfo& info); // wrapped add function
  std::unique_ptr<ocl::Path> actualClass_; // Internal instance of path used to perform actual operations.
};