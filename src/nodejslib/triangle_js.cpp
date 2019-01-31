#include "triangle.hpp"
#include "triangle_js.hpp"
#include "point_js.hpp"

Napi::FunctionReference TriangleJS::constructor;

Napi::Object TriangleJS::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "Triangle", {});
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("Triangle", func);
    return exports;
}

TriangleJS::TriangleJS(const Napi::CallbackInfo &info) : Napi::ObjectWrap<TriangleJS>(info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    int length = info.Length();
    if (length == 0)
    {
        actualClass_ = ocl::Triangle();
    }
    else if (length == 3)
    {
        PointJS *p1js = Napi::ObjectWrap<PointJS>::Unwrap(info[0].As<Napi::Object>());
        ocl::Point *p1 = p1js->GetInternalInstance();
        PointJS *p2js = Napi::ObjectWrap<PointJS>::Unwrap(info[1].As<Napi::Object>());
        ocl::Point *p2 = p2js->GetInternalInstance();
        PointJS *p3js = Napi::ObjectWrap<PointJS>::Unwrap(info[2].As<Napi::Object>());
        ocl::Point *p3 = p3js->GetInternalInstance();
        actualClass_ = ocl::Triangle(*p1, *p2, *p3);
    }
    else
    {
        Napi::TypeError::New(env, "Provide at 3 or 0 arguments").ThrowAsJavaScriptException();
    }
}

ocl::Triangle* TriangleJS::GetInternalInstance()
{
    return &actualClass_;
}