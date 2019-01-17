#include "waterline_js.hpp"
#include "stlsurf_js.hpp"
#include "point.hpp"
#include "cylcutter_js.hpp"
#include "ballcutter_js.hpp"
#include "bullcutter_js.hpp"
#include "conecutter_js.hpp"

Napi::FunctionReference WaterlineJS::constructor;

Napi::Object WaterlineJS::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "Waterline", {
        InstanceMethod("setZ", &WaterlineJS::setZ),
        InstanceMethod("setSTL", &WaterlineJS::setSTL),
        InstanceMethod("setCylCutter", &WaterlineJS::setCylCutter),
        InstanceMethod("setBallCutter", &WaterlineJS::setBallCutter),
        InstanceMethod("setBullCutter", &WaterlineJS::setBullCutter),
        InstanceMethod("setConeCutter", &WaterlineJS::setConeCutter),
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

ocl::Waterline* WaterlineJS::GetInternalInstance()
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

void WaterlineJS::setCylCutter(const Napi::CallbackInfo &info)
{
    CylCutterJS *cjs = Napi::ObjectWrap<CylCutterJS>::Unwrap(info[0].As<Napi::Object>());
    ocl::CylCutter *cutter = cjs->GetInternalInstance();
    actualClass_.setCutter(cutter);
}

void WaterlineJS::setBallCutter(const Napi::CallbackInfo &info)
{
    BallCutterJS *cjs = Napi::ObjectWrap<BallCutterJS>::Unwrap(info[0].As<Napi::Object>());
    ocl::BallCutter *cutter = cjs->GetInternalInstance();
    actualClass_.setCutter(cutter);
}

void WaterlineJS::setBullCutter(const Napi::CallbackInfo &info)
{
    BullCutterJS *cjs = Napi::ObjectWrap<BullCutterJS>::Unwrap(info[0].As<Napi::Object>());
    ocl::BullCutter *cutter = cjs->GetInternalInstance();
    actualClass_.setCutter(cutter);
}

void WaterlineJS::setConeCutter(const Napi::CallbackInfo &info)
{
    ConeCutterJS *cjs = Napi::ObjectWrap<ConeCutterJS>::Unwrap(info[0].As<Napi::Object>());
    ocl::ConeCutter *cutter = cjs->GetInternalInstance();
    actualClass_.setCutter(cutter);
}

void WaterlineJS::setSampling(const Napi::CallbackInfo &info)
{
    Napi::Number s = info[0].As<Napi::Number>();
    actualClass_.setSampling(s.DoubleValue());
}

void WaterlineJS::run(const Napi::CallbackInfo &info)
{
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
    int loopI = 0;
    for (auto & loop : loops)
    {
        Napi::Array loopArr = Napi::Array::New(env);
        int pointI = 0;
        for (auto &point : loop)
        {
            Napi::Array pointArr = Napi::Array::New(env);
            pointArr.Set(x, Napi::Number::New(env, point.x));
            pointArr.Set(y, Napi::Number::New(env, point.y));
            pointArr.Set(z, Napi::Number::New(env, point.z));
            loopArr.Set(pointI, pointArr);
            pointI++;
        }
        result.Set(loopI, loopArr);
        loopI++;
    }
    return result;
}
