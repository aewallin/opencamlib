#include <codecvt>
#include <locale>
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
    size_t length = info.Length();
    if (length != 2)
    {
        throw Napi::TypeError::New(env, "Provide at least 2 arguments");
    }
    Napi::String filepath = info[0].As<Napi::String>();
    STLSurfJS *surface = STLSurfJS::UnwrapChecked(info[1]);
    ocl::STLSurf *surfaceInstance = surface->GetInternalInstance(info);
    std::wstring filepathWstring;
    std::string filepathStr = filepath.Utf8Value();
    filepathWstring = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(filepathStr);
    nativeCall(info.Env(), [&] { ocl::STLReader reader(filepathWstring, *surfaceInstance); });
}