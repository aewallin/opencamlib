#include <napi.h>
#include "point.hpp"

class PointJS : public Napi::ObjectWrap<PointJS> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports); // Init function for setting the export key to JS
  PointJS(const Napi::CallbackInfo& info); // Constructor to initialise
  ocl::Point* GetInternalInstance();

 private:
  static Napi::FunctionReference constructor; //reference to store the class definition that needs to be exported to JS
  Napi::Value GetX(const Napi::CallbackInfo& info);
  void SetX(const Napi::CallbackInfo& info, const Napi::Value& value);
  void Add(const Napi::CallbackInfo& info); // wrapped add function
  ocl::Point actualClass_; // Internal instance of point used to perform actual operations.
};