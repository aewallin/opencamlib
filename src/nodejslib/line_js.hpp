#include "binding_utils.hpp"
#include "line.hpp"

class LineJS : public Napi::ObjectWrap<LineJS> {
 public:
    static bool IsInstance(const Napi::Value& value) {
        return value.IsObject() && value.As<Napi::Object>().InstanceOf(constructor.Value());
    }
    static LineJS* UnwrapChecked(const Napi::Value& value) {
        if (!IsInstance(value)) throw Napi::TypeError::New(value.Env(), "Expected LineJS");
        return Napi::ObjectWrap<LineJS>::Unwrap(value.As<Napi::Object>());
    }

  static Napi::Object Init(Napi::Env env, Napi::Object exports); // Init function for setting the export key to JS
  LineJS(const Napi::CallbackInfo& info); // Constructor to initialise
  ocl::Line *GetInternalInstance(const Napi::CallbackInfo &info);

private:
  static Napi::FunctionReference constructor; //reference to store the class definition that needs to be exported to JS
  std::unique_ptr<ocl::Line> actualClass_; // Internal instance of line used to perform actual operations.
};