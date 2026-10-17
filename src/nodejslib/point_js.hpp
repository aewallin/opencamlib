#include "binding_utils.hpp"
#include "point.hpp"

class PointJS : public Napi::ObjectWrap<PointJS> {
 public:
    static bool IsInstance(const Napi::Value& value) {
        return value.IsObject() && value.As<Napi::Object>().InstanceOf(constructor.Value());
    }
    static PointJS* UnwrapChecked(const Napi::Value& value) {
        if (!IsInstance(value)) throw Napi::TypeError::New(value.Env(), "Expected PointJS");
        return Napi::ObjectWrap<PointJS>::Unwrap(value.As<Napi::Object>());
    }

  static Napi::Object Init(Napi::Env env, Napi::Object exports); // Init function for setting the export key to JS
  PointJS(const Napi::CallbackInfo& info); // Constructor to initialise
  ocl::Point *GetInternalInstance(const Napi::CallbackInfo &info);

private:
  static Napi::FunctionReference constructor; //reference to store the class definition that needs to be exported to JS
  Napi::Value GetX(const Napi::CallbackInfo& info);
  void SetX(const Napi::CallbackInfo& info, const Napi::Value& value);
  Napi::Value GetY(const Napi::CallbackInfo& info);
  void SetY(const Napi::CallbackInfo& info, const Napi::Value& value);
  Napi::Value GetZ(const Napi::CallbackInfo& info);
  void SetZ(const Napi::CallbackInfo& info, const Napi::Value& value);
  void Add(const Napi::CallbackInfo& info); // wrapped add function
  std::unique_ptr<ocl::Point> actualClass_; // Internal instance of point used to perform actual operations.
};