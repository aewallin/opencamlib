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
        InstanceMethod("setZ", &AdaptivePathDropCutterJS::setZ),
        InstanceMethod("getCLPoints", &AdaptivePathDropCutterJS::getCLPoints),
        InstanceMethod("dispose", &AdaptivePathDropCutterJS::dispose),
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
    this->actualClass_ = std::make_unique<ocl::AdaptivePathDropCutter>();
}

void AdaptivePathDropCutterJS::setSTL(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    STLSurfJS *sjs = STLSurfJS::UnwrapChecked(info[0]);
    ocl::STLSurf *surface = sjs->GetInternalInstance(info);
    nativeCall(info.Env(), [&] { this->actualClass_->setSTL(*surface); });
    surface_ref = Napi::Persistent(info[0].As<Napi::Object>());
}

void AdaptivePathDropCutterJS::setPath(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    PathJS *pjs = PathJS::UnwrapChecked(info[0]);
    ocl::Path *path = pjs->GetInternalInstance(info);
    nativeCall(info.Env(), [&] { this->actualClass_->setPath(path); });
    path_ref = Napi::Persistent(info[0].As<Napi::Object>());
}

void AdaptivePathDropCutterJS::setCylCutter(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    CylCutterJS *cjs = CylCutterJS::UnwrapChecked(info[0]);
    ocl::CylCutter *cutter = cjs->GetInternalInstance(info);
    nativeCall(info.Env(), [&] { this->actualClass_->setCutter(cutter); });
    cutter_ref = Napi::Persistent(info[0].As<Napi::Object>());
}

void AdaptivePathDropCutterJS::setBallCutter(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    BallCutterJS *cjs = BallCutterJS::UnwrapChecked(info[0]);
    ocl::BallCutter *cutter = cjs->GetInternalInstance(info);
    nativeCall(info.Env(), [&] { this->actualClass_->setCutter(cutter); });
    cutter_ref = Napi::Persistent(info[0].As<Napi::Object>());
}

void AdaptivePathDropCutterJS::setBullCutter(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    BullCutterJS *cjs = BullCutterJS::UnwrapChecked(info[0]);
    ocl::BullCutter *cutter = cjs->GetInternalInstance(info);
    nativeCall(info.Env(), [&] { this->actualClass_->setCutter(cutter); });
    cutter_ref = Napi::Persistent(info[0].As<Napi::Object>());
}

void AdaptivePathDropCutterJS::setConeCutter(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    ConeCutterJS *cjs = ConeCutterJS::UnwrapChecked(info[0]);
    ocl::ConeCutter *cutter = cjs->GetInternalInstance(info);
    nativeCall(info.Env(), [&] { this->actualClass_->setCutter(cutter); });
    cutter_ref = Napi::Persistent(info[0].As<Napi::Object>());
}

void AdaptivePathDropCutterJS::setSampling(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::Number s = info[0].As<Napi::Number>();
    nativeCall(info.Env(), [&] { this->actualClass_->setSampling(s.DoubleValue()); });
}

void AdaptivePathDropCutterJS::setMinSampling(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::Number s = info[0].As<Napi::Number>();
    nativeCall(info.Env(), [&] { this->actualClass_->setMinSampling(s.DoubleValue()); });
}

void AdaptivePathDropCutterJS::setZ(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::Number s = info[0].As<Napi::Number>();
    nativeCall(info.Env(), [&] { this->actualClass_->setZ(s.DoubleValue()); });
}

Napi::Value AdaptivePathDropCutterJS::getCLPoints(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::Array result = Napi::Array::New(env);
    std::vector<ocl::CLPoint> points = this->actualClass_->getPoints();
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
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    nativeCall(info.Env(), [&] { this->actualClass_->run(); });
}

void AdaptivePathDropCutterJS::dispose(const Napi::CallbackInfo&) {
    actualClass_.reset();
    surface_ref.Reset(); cutter_ref.Reset(); path_ref.Reset();
}
