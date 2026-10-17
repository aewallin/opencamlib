#include "pathdropcutter_js.hpp"
#include "point.hpp"
#include "clpoint.hpp"
#include "path_js.hpp"
#include "stlsurf_js.hpp"
#include "cylcutter_js.hpp"
#include "ballcutter_js.hpp"
#include "bullcutter_js.hpp"
#include "conecutter_js.hpp"

Napi::FunctionReference PathDropCutterJS::constructor;

void PathDropCutterJS::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "PathDropCutter", {
        InstanceMethod("setSTL", &PathDropCutterJS::setSTL),
        InstanceMethod("setPath", &PathDropCutterJS::setPath),
        InstanceMethod("setCylCutter", &PathDropCutterJS::setCylCutter),
        InstanceMethod("setBallCutter", &PathDropCutterJS::setBallCutter),
        InstanceMethod("setBullCutter", &PathDropCutterJS::setBullCutter),
        InstanceMethod("setConeCutter", &PathDropCutterJS::setConeCutter),
        InstanceMethod("setSampling", &PathDropCutterJS::setSampling),
        InstanceMethod("getCLPoints", &PathDropCutterJS::getCLPoints),
        InstanceMethod("setZ", &PathDropCutterJS::setZ),
        InstanceMethod("dispose", &PathDropCutterJS::dispose),
        InstanceMethod("run", &PathDropCutterJS::run)
    });
    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("PathDropCutter", func);
}

PathDropCutterJS::PathDropCutterJS(const Napi::CallbackInfo &info) : Napi::ObjectWrap<PathDropCutterJS>(info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    this->actualClass_ = std::make_unique<ocl::PathDropCutter>();
}

void PathDropCutterJS::setSTL(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    STLSurfJS *sjs = STLSurfJS::UnwrapChecked(info[0]);
    ocl::STLSurf *surface = sjs->GetInternalInstance(info);
    nativeCall(info.Env(), [&] { this->actualClass_->setSTL(*surface); });
    surface_ref = Napi::Persistent(info[0].As<Napi::Object>());
}

void PathDropCutterJS::setPath(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    PathJS *pjs = PathJS::UnwrapChecked(info[0]);
    ocl::Path *path = pjs->GetInternalInstance(info);
    nativeCall(info.Env(), [&] { this->actualClass_->setPath(path); });
    path_ref = Napi::Persistent(info[0].As<Napi::Object>());
}

void PathDropCutterJS::setCylCutter(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    CylCutterJS *cjs = CylCutterJS::UnwrapChecked(info[0]);
    ocl::CylCutter *cutter = cjs->GetInternalInstance(info);
    nativeCall(info.Env(), [&] { this->actualClass_->setCutter(cutter); });
    cutter_ref = Napi::Persistent(info[0].As<Napi::Object>());
}

void PathDropCutterJS::setBallCutter(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    BallCutterJS *cjs = BallCutterJS::UnwrapChecked(info[0]);
    ocl::BallCutter *cutter = cjs->GetInternalInstance(info);
    nativeCall(info.Env(), [&] { this->actualClass_->setCutter(cutter); });
    cutter_ref = Napi::Persistent(info[0].As<Napi::Object>());
}

void PathDropCutterJS::setBullCutter(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    BullCutterJS *cjs = BullCutterJS::UnwrapChecked(info[0]);
    ocl::BullCutter *cutter = cjs->GetInternalInstance(info);
    nativeCall(info.Env(), [&] { this->actualClass_->setCutter(cutter); });
    cutter_ref = Napi::Persistent(info[0].As<Napi::Object>());
}

void PathDropCutterJS::setConeCutter(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    ConeCutterJS *cjs = ConeCutterJS::UnwrapChecked(info[0]);
    ocl::ConeCutter *cutter = cjs->GetInternalInstance(info);
    nativeCall(info.Env(), [&] { this->actualClass_->setCutter(cutter); });
    cutter_ref = Napi::Persistent(info[0].As<Napi::Object>());
}

void PathDropCutterJS::setSampling(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::Number s = info[0].As<Napi::Number>();
    nativeCall(info.Env(), [&] { this->actualClass_->setSampling(s.DoubleValue()); });
}

void PathDropCutterJS::setZ(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::Number s = info[0].As<Napi::Number>();
    nativeCall(info.Env(), [&] { this->actualClass_->setZ(s.DoubleValue()); });
}

Napi::Value PathDropCutterJS::getCLPoints(const Napi::CallbackInfo &info)
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
    for (auto &point : points)
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

void PathDropCutterJS::run(const Napi::CallbackInfo &info)
{
    if (!actualClass_) throw Napi::Error::New(info.Env(), "Operation has been disposed");
    nativeCall(info.Env(), [&] { this->actualClass_->run(); });
}

void PathDropCutterJS::dispose(const Napi::CallbackInfo&) {
    actualClass_.reset();
    surface_ref.Reset(); cutter_ref.Reset(); path_ref.Reset();
}
