#include "arc_js.hpp"
#include "point_js.hpp"

Napi::FunctionReference ArcJS::constructor;
Napi::Object ArcJS::Init(Napi::Env env, Napi::Object exports) {
    auto function = DefineClass(env, "Arc", {});
    constructor = Napi::Persistent(function);
    constructor.SuppressDestruct();
    exports.Set("Arc", function);
    return exports;
}
ArcJS::ArcJS(const Napi::CallbackInfo& info) : Napi::ObjectWrap<ArcJS>(info) {
    if (info.Length() != 4 || !info[3].IsBoolean())
        throw Napi::TypeError::New(info.Env(), "Arc requires start, end, center and direction");
    auto* start = PointJS::UnwrapChecked(info[0])->GetInternalInstance(info);
    auto* end = PointJS::UnwrapChecked(info[1])->GetInternalInstance(info);
    auto* center = PointJS::UnwrapChecked(info[2])->GetInternalInstance(info);
    actualClass_ = nativeCall(info.Env(), [&] {
        return std::make_unique<ocl::Arc>(*start, *end, *center, info[3].As<Napi::Boolean>().Value());
    });
}
