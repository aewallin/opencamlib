#include <napi.h>
#include "line.hpp"

class LineJS : public Napi::ObjectWrap<LineJS> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports); // Init function for setting the export key to JS
  LineJS(const Napi::CallbackInfo& info); // Constructor to initialise
  ocl::Line* GetInternalInstance();

 private:
  static Napi::FunctionReference constructor; //reference to store the class definition that needs to be exported to JS
  ocl::Line actualClass_; // Internal instance of line used to perform actual operations.
};