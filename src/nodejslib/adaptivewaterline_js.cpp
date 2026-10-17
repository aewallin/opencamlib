#include "adaptivewaterline_js.hpp"
#include "stlsurf_js.hpp"
#include "point.hpp"
#include "cylcutter_js.hpp"
#include "ballcutter_js.hpp"
#include "bullcutter_js.hpp"
#include "conecutter_js.hpp"

Napi::FunctionReference AdaptiveWaterlineJS::constructor;

Napi::Object AdaptiveWaterlineJS::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "AdaptiveWaterline", {
        InstanceMethod("setZ", &AdaptiveWaterlineJS::setZ),
        InstanceMethod("setSTL", &AdaptiveWaterlineJS::setSTL),
        InstanceMethod("setCylCutter", &AdaptiveWaterlineJS::setCylCutter),
        InstanceMethod("setBallCutter", &AdaptiveWaterlineJS::setBallCutter),
        InstanceMethod("setBullCutter", &AdaptiveWaterlineJS::setBullCutter),
        InstanceMethod("setConeCutter", &AdaptiveWaterlineJS::setConeCutter),
        InstanceMethod("setSampling", &AdaptiveWaterlineJS::setSampling),
        InstanceMethod("setMinSampling", &AdaptiveWaterlineJS::setMinSampling),
        InstanceMethod("dispose", &AdaptiveWaterlineJS::dispose),
        InstanceMethod("run", &AdaptiveWaterlineJS::run),
        InstanceMethod("getLoops", &AdaptiveWaterlineJS::getLoops)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("AdaptiveWaterline", func);
    return exports;
}

AdaptiveWaterlineJS::AdaptiveWaterlineJS(const Napi::CallbackInfo &info) : Napi::ObjectWrap<AdaptiveWaterlineJS>(info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    this->actualClass_ = std::make_unique<ocl::AdaptiveWaterline>();
}

void AdaptiveWaterlineJS::setZ(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::Number z = info[0].As<Napi::Number>();
    nativeCall(info.Env(), [&] { this->actualClass_->setZ(z.DoubleValue()); });
}

void AdaptiveWaterlineJS::setSTL(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    STLSurfJS *sjs = STLSurfJS::UnwrapChecked(info[0]);
    ocl::STLSurf *surface = sjs->GetInternalInstance(info);
    nativeCall(info.Env(), [&] { this->actualClass_->setSTL(*surface); });
    surface_ref = Napi::Persistent(info[0].As<Napi::Object>());
}

void AdaptiveWaterlineJS::setCylCutter(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    CylCutterJS *cjs = CylCutterJS::UnwrapChecked(info[0]);
    ocl::CylCutter *cutter = cjs->GetInternalInstance(info);
    nativeCall(info.Env(), [&] { this->actualClass_->setCutter(cutter); });
    cutter_ref = Napi::Persistent(info[0].As<Napi::Object>());
}

void AdaptiveWaterlineJS::setBallCutter(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    BallCutterJS *cjs = BallCutterJS::UnwrapChecked(info[0]);
    ocl::BallCutter *cutter = cjs->GetInternalInstance(info);
    nativeCall(info.Env(), [&] { this->actualClass_->setCutter(cutter); });
    cutter_ref = Napi::Persistent(info[0].As<Napi::Object>());
}

void AdaptiveWaterlineJS::setBullCutter(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    BullCutterJS *cjs = BullCutterJS::UnwrapChecked(info[0]);
    ocl::BullCutter *cutter = cjs->GetInternalInstance(info);
    nativeCall(info.Env(), [&] { this->actualClass_->setCutter(cutter); });
    cutter_ref = Napi::Persistent(info[0].As<Napi::Object>());
}

void AdaptiveWaterlineJS::setConeCutter(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    ConeCutterJS *cjs = ConeCutterJS::UnwrapChecked(info[0]);
    ocl::ConeCutter *cutter = cjs->GetInternalInstance(info);
    nativeCall(info.Env(), [&] { this->actualClass_->setCutter(cutter); });
    cutter_ref = Napi::Persistent(info[0].As<Napi::Object>());
}

void AdaptiveWaterlineJS::setSampling(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::Number s = info[0].As<Napi::Number>();
    nativeCall(info.Env(), [&] { this->actualClass_->setSampling(s.DoubleValue()); });
}

void AdaptiveWaterlineJS::setMinSampling(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::Number s = info[0].As<Napi::Number>();
    nativeCall(info.Env(), [&] { this->actualClass_->setMinSampling(s.DoubleValue()); });
}

void AdaptiveWaterlineJS::run(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    nativeCall(info.Env(), [&] { this->actualClass_->run(); });
}

Napi::Value AdaptiveWaterlineJS::getLoops(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::Array result = Napi::Array::New(env);
    std::vector<std::vector<ocl::Point>> loops = this->actualClass_->getLoops();
    int x = 0;
    int y = 1;
    int z = 2;
    int loopI = 0;
    for (auto &loop : loops)
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

void AdaptiveWaterlineJS::dispose(const Napi::CallbackInfo&) {
    actualClass_.reset();
    surface_ref.Reset(); cutter_ref.Reset(); path_ref.Reset();
}
