#include <napi.h>
#include "path.hpp"

class PathJS : public Napi::ObjectWrap<PathJS> {
 public:
  static Napi::Object Init(Napi::Env env, Napi::Object exports); // Init function for setting the export key to JS
  PathJS(const Napi::CallbackInfo& info); // Constructor to initialise
  ocl::Path* GetInternalInstance();

 private:
  static Napi::FunctionReference constructor; //reference to store the class definition that needs to be exported to JS
  void append(const Napi::CallbackInfo& info); // wrapped add function
  ocl::Path actualClass_; // Internal instance of path used to perform actual operations.
};