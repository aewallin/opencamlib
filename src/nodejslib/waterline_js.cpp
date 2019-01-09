#include "waterline.hpp"
#include "waterline_js.hpp"
#include "stlsurf_js.hpp"
#include "cylcutter_js.hpp"
#include "cylcutter.hpp"
#include "point.hpp"
#include "millingcutter.hpp"

// class WaterLineAsyncWorker : public Napi::AsyncWorker
// {
//   public:
//     WaterLineAsyncWorker(Napi::Function &callback, ocl::Waterline waterline) : Napi::AsyncWorker(callback) {}
//     ~WaterLineAsyncWorker() {}

//     void Execute()
//     {
//         waterline.run();
//     }

//     void OnOK()
//     {
//         Napi::HandleScope scope(Env());
//         Callback().Call({Env().Null()});
//     }

//   private:
//     ocl::Waterline waterline;
// };


Napi::FunctionReference WaterlineJS::constructor;

Napi::Object WaterlineJS::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "Waterline", {
        InstanceMethod("setZ", &WaterlineJS::setZ),
        InstanceMethod("setSTL", &WaterlineJS::setSTL),
        InstanceMethod("setCutter", &WaterlineJS::setCutter),
        InstanceMethod("setSampling", &WaterlineJS::setSampling),
        InstanceMethod("run", &WaterlineJS::run),
        InstanceMethod("getLoops", &WaterlineJS::getLoops)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("Waterline", func);
    return exports;
}

WaterlineJS::WaterlineJS(const Napi::CallbackInfo &info) : Napi::ObjectWrap<WaterlineJS>(info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    actualClass_ = ocl::Waterline();
}

ocl::Waterline *WaterlineJS::GetInternalInstance()
{
    return &actualClass_;
}

void WaterlineJS::setZ(const Napi::CallbackInfo &info)
{
    Napi::Number z = info[0].As<Napi::Number>();
    actualClass_.setZ(z.DoubleValue());
}

void WaterlineJS::setSTL(const Napi::CallbackInfo &info)
{
    STLSurfJS *sjs = Napi::ObjectWrap<STLSurfJS>::Unwrap(info[0].As<Napi::Object>());
    ocl::STLSurf *surface = sjs->GetInternalInstance();
    actualClass_.setSTL(*surface);
}

void WaterlineJS::setCutter(const Napi::CallbackInfo &info)
{
    CylCutterJS *cjs = Napi::ObjectWrap<CylCutterJS>::Unwrap(info[0].As<Napi::Object>());
    ocl::CylCutter *cutter = cjs->GetInternalInstance();
    actualClass_.setCutter(cutter);
}

void WaterlineJS::setSampling(const Napi::CallbackInfo &info)
{
    Napi::Number s = info[0].As<Napi::Number>();
    actualClass_.setSampling(s.DoubleValue());
}

void WaterlineJS::run(const Napi::CallbackInfo &info)
{
    // Napi::Function callback = info[0].As<Napi::Function>();
    // WaterLineAsyncWorker *w = new WaterLineAsyncWorker(callback, actualClass_);
    // w->Queue();
    actualClass_.run();
}

Napi::Value WaterlineJS::getLoops(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::Array result = Napi::Array::New(env);
    std::vector<std::vector<ocl::Point>> loops = actualClass_.getLoops();
    int x = 0;
    int y = 1;
    int z = 2;
    int i = 0;
    for (auto & points : loops)
    {
        for (auto &point : points)
        {
            Napi::Array pointArr = Napi::Array::New(env);
            pointArr.Set(x, Napi::Number::New(env, point.x));
            pointArr.Set(y, Napi::Number::New(env, point.y));
            pointArr.Set(z, Napi::Number::New(env, point.z));
            result.Set(i, pointArr);
            i++;
        }
    }
    return result;
}
