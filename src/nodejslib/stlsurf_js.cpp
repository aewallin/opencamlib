#include "triangle.hpp"
#include "stlsurf.hpp"
#include "stlsurf_js.hpp"

Napi::FunctionReference STLSurfJS::constructor;

Napi::Object STLSurfJS::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "STLSurf", {
        InstanceMethod("getTriangles", &STLSurfJS::getTriangles)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("STLSurf", func);
    return exports;
}

STLSurfJS::STLSurfJS(const Napi::CallbackInfo &info) : Napi::ObjectWrap<STLSurfJS>(info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    actualClass_ = ocl::STLSurf();
}

ocl::STLSurf* STLSurfJS::GetInternalInstance()
{
    return &actualClass_;
}

Napi::Value STLSurfJS::getTriangles(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::Array arr = Napi::Array::New(env);
    int i = 0;
    int a = 0;
    int b = 1;
    int c = 2;
    std::list<ocl::Triangle>::iterator it;
    for (it = actualClass_.tris.begin(); it != actualClass_.tris.end(); ++it)
    {
        Napi::Array points = Napi::Array::New(env);
        Napi::Array p1 = Napi::Array::New(env);
        p1.Set(a, Napi::Number::New(env, it->p[0].x));
        p1.Set(b, Napi::Number::New(env, it->p[0].y));
        p1.Set(c, Napi::Number::New(env, it->p[0].z));
        points.Set(a, p1);
        Napi::Array p2 = Napi::Array::New(env);
        p2.Set(a, Napi::Number::New(env, it->p[1].x));
        p2.Set(b, Napi::Number::New(env, it->p[1].y));
        p2.Set(c, Napi::Number::New(env, it->p[1].z));
        points.Set(b, p2);
        Napi::Array p3 = Napi::Array::New(env);
        p3.Set(a, Napi::Number::New(env, it->p[2].x));
        p3.Set(b, Napi::Number::New(env, it->p[2].y));
        p3.Set(c, Napi::Number::New(env, it->p[2].z));
        points.Set(c, p3);
        arr.Set(i, points);
        i++;
    }
    return arr;
}
