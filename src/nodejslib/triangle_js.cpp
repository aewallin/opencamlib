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
    size_t length = info.Length();
    if (length == 0)
    {
        this->actualClass_ = std::make_unique<ocl::Triangle>();
    }
    else if (length == 3)
    {
        PointJS *p1js = PointJS::UnwrapChecked(info[0]);
        ocl::Point *p1 = p1js->GetInternalInstance(info);
        PointJS *p2js = PointJS::UnwrapChecked(info[1]);
        ocl::Point *p2 = p2js->GetInternalInstance(info);
        PointJS *p3js = PointJS::UnwrapChecked(info[2]);
        ocl::Point *p3 = p3js->GetInternalInstance(info);
        this->actualClass_ = std::make_unique<ocl::Triangle>(*p1, *p2, *p3);
    }
    else
    {
        throw Napi::TypeError::New(env, "Provide at 3 or 0 arguments");
    }
}

ocl::Triangle *TriangleJS::GetInternalInstance(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    return this->actualClass_.get();
}