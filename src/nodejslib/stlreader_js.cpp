#include <sstream>
#include "stlreader.hpp"
#include "stlsurf.hpp"
#include "stlreader_js.hpp"
#include "stlsurf_js.hpp"

Napi::FunctionReference STLReaderJS::constructor;

Napi::Object STLReaderJS::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "STLReader", {
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("STLReader", func);
    return exports;
}

STLReaderJS::STLReaderJS(const Napi::CallbackInfo &info) : Napi::ObjectWrap<STLReaderJS>(info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    int length = info.Length();
    if (length == 1)
    {
        Napi::TypeError::New(env, "Provide at least 2 arguments").ThrowAsJavaScriptException();
    }
    Napi::String filepath = info[0].As<Napi::String>();
    STLSurfJS *surface = Napi::ObjectWrap<STLSurfJS>::Unwrap(info[1].As<Napi::Object>());
    ocl::STLSurf *surfaceInstance = surface->GetInternalInstance();
    std::wstring filepathWstring;
    std::string filepathStr = filepath.Utf8Value();
    filepathWstring.assign(filepathStr.begin(), filepathStr.end());
    ocl::STLReader(filepathWstring, *surfaceInstance);
}