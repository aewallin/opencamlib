#include "adaptivepathdropcutter_js.hpp"
#include "point.hpp"
#include "clpoint.hpp"
#include "path_js.hpp"
#include "stlsurf_js.hpp"
#include "cylcutter_js.hpp"
#include "ballcutter_js.hpp"
#include "bullcutter_js.hpp"
#include "conecutter_js.hpp"

Napi::FunctionReference AdaptivePathDropCutterJS::constructor;

void AdaptivePathDropCutterJS::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "AdaptivePathDropCutter", {
        InstanceMethod("setSTL", &AdaptivePathDropCutterJS::setSTL),
        InstanceMethod("setPath", &AdaptivePathDropCutterJS::setPath),
        InstanceMethod("setCylCutter", &AdaptivePathDropCutterJS::setCylCutter),
        InstanceMethod("setBallCutter", &AdaptivePathDropCutterJS::setBallCutter),
        InstanceMethod("setBullCutter", &AdaptivePathDropCutterJS::setBullCutter),
        InstanceMethod("setConeCutter", &AdaptivePathDropCutterJS::setConeCutter),
        InstanceMethod("setSampling", &AdaptivePathDropCutterJS::setSampling),
        InstanceMethod("setMinSampling", &AdaptivePathDropCutterJS::setMinSampling),
        InstanceMethod("getCLPoints", &AdaptivePathDropCutterJS::getCLPoints),
        InstanceMethod("run", &AdaptivePathDropCutterJS::run)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("AdaptivePathDropCutter", func);
}

AdaptivePathDropCutterJS::AdaptivePathDropCutterJS(const Napi::CallbackInfo &info) : Napi::ObjectWrap<AdaptivePathDropCutterJS>(info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    // std::cout << "AdaptivePathDropCutterJS::AdaptivePathDropCutterJS()" << std::endl;
    actualClass_ = ocl::AdaptivePathDropCutter();
}

ocl::AdaptivePathDropCutter* AdaptivePathDropCutterJS::GetInternalInstance()
{
    return &actualClass_;
}

void AdaptivePathDropCutterJS::setSTL(const Napi::CallbackInfo &info)
{
    // std::cout << "AdaptivePathDropCutterJS::setSTL()" << std::endl;
    STLSurfJS *sjs = Napi::ObjectWrap<STLSurfJS>::Unwrap(info[0].As<Napi::Object>());
    ocl::STLSurf *surface = sjs->GetInternalInstance();
    actualClass_.setSTL(*surface);
}

void AdaptivePathDropCutterJS::setPath(const Napi::CallbackInfo &info)
{
    // std::cout << "AdaptivePathDropCutterJS::setPath()" << std::endl;
    PathJS *pjs = Napi::ObjectWrap<PathJS>::Unwrap(info[0].As<Napi::Object>());
    ocl::Path *path = pjs->GetInternalInstance();
    actualClass_.setPath(path);
}

void AdaptivePathDropCutterJS::setCylCutter(const Napi::CallbackInfo &info)
{
    // std::cout << "AdaptivePathDropCutterJS::setCylCutter()" << std::endl;
    CylCutterJS *cjs = Napi::ObjectWrap<CylCutterJS>::Unwrap(info[0].As<Napi::Object>());
    ocl::CylCutter *cutter = cjs->GetInternalInstance();
    actualClass_.setCutter(cutter);
}

void AdaptivePathDropCutterJS::setBallCutter(const Napi::CallbackInfo &info)
{
    // std::cout << "AdaptivePathDropCutterJS::setBallCutter()" << std::endl;
    BallCutterJS *cjs = Napi::ObjectWrap<BallCutterJS>::Unwrap(info[0].As<Napi::Object>());
    ocl::BallCutter *cutter = cjs->GetInternalInstance();
    actualClass_.setCutter(cutter);
}

void AdaptivePathDropCutterJS::setBullCutter(const Napi::CallbackInfo &info)
{
    // std::cout << "AdaptivePathDropCutterJS::setBullCutter()" << std::endl;
    BullCutterJS *cjs = Napi::ObjectWrap<BullCutterJS>::Unwrap(info[0].As<Napi::Object>());
    ocl::BullCutter *cutter = cjs->GetInternalInstance();
    actualClass_.setCutter(cutter);
}

void AdaptivePathDropCutterJS::setConeCutter(const Napi::CallbackInfo &info)
{
    // std::cout << "AdaptivePathDropCutterJS::setConeCutter()" << std::endl;
    ConeCutterJS *cjs = Napi::ObjectWrap<ConeCutterJS>::Unwrap(info[0].As<Napi::Object>());
    ocl::ConeCutter *cutter = cjs->GetInternalInstance();
    actualClass_.setCutter(cutter);
}

void AdaptivePathDropCutterJS::setSampling(const Napi::CallbackInfo &info)
{
    // std::cout << "AdaptivePathDropCutterJS::setSampling()" << std::endl;
    Napi::Number s = info[0].As<Napi::Number>();
    actualClass_.setSampling(s.DoubleValue());
}

void AdaptivePathDropCutterJS::setMinSampling(const Napi::CallbackInfo &info)
{
    // std::cout << "AdaptivePathDropCutterJS::setMinSampling()" << std::endl;
    Napi::Number s = info[0].As<Napi::Number>();
    actualClass_.setMinSampling(s.DoubleValue());
}

Napi::Value AdaptivePathDropCutterJS::getCLPoints(const Napi::CallbackInfo &info)
{
    // std::cout << "AdaptivePathDropCutterJS::getCLPoints()" << std::endl;
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::Array result = Napi::Array::New(env);
    std::vector<ocl::CLPoint> points = actualClass_.getPoints();
    int x = 0;
    int y = 1;
    int z = 2;
    int i = 0;
    // std::cout << points.size();
    for (auto & point : points)
    {
        Napi::Array pointArr = Napi::Array::New(env);
        pointArr.Set(x, Napi::Number::New(env, point.x));
        pointArr.Set(y, Napi::Number::New(env, point.y));
        pointArr.Set(z, Napi::Number::New(env, point.z));
        result.Set(i, pointArr);
        i++;
    }
    return result;
}

void AdaptivePathDropCutterJS::run(const Napi::CallbackInfo &info)
{
    // std::cout << "AdaptivePathDropCutterJS::run()" << std::endl;
    actualClass_.run();
}
