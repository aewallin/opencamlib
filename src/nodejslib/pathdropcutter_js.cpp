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
    this->actualClass_ = new ocl::PathDropCutter();
}

void PathDropCutterJS::setSTL(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    STLSurfJS *sjs = Napi::ObjectWrap<STLSurfJS>::Unwrap(info[0].As<Napi::Object>());
    ocl::STLSurf *surface = sjs->GetInternalInstance(info);
    this->actualClass_->setSTL(*surface);
}

void PathDropCutterJS::setPath(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    PathJS *pjs = Napi::ObjectWrap<PathJS>::Unwrap(info[0].As<Napi::Object>());
    ocl::Path *path = pjs->GetInternalInstance(info);
    this->actualClass_->setPath(path);
}

void PathDropCutterJS::setCylCutter(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    CylCutterJS *cjs = Napi::ObjectWrap<CylCutterJS>::Unwrap(info[0].As<Napi::Object>());
    ocl::CylCutter *cutter = cjs->GetInternalInstance(info);
    this->actualClass_->setCutter(cutter);
}

void PathDropCutterJS::setBallCutter(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    BallCutterJS *cjs = Napi::ObjectWrap<BallCutterJS>::Unwrap(info[0].As<Napi::Object>());
    ocl::BallCutter *cutter = cjs->GetInternalInstance(info);
    this->actualClass_->setCutter(cutter);
}

void PathDropCutterJS::setBullCutter(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    BullCutterJS *cjs = Napi::ObjectWrap<BullCutterJS>::Unwrap(info[0].As<Napi::Object>());
    ocl::BullCutter *cutter = cjs->GetInternalInstance(info);
    this->actualClass_->setCutter(cutter);
}

void PathDropCutterJS::setConeCutter(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    ConeCutterJS *cjs = Napi::ObjectWrap<ConeCutterJS>::Unwrap(info[0].As<Napi::Object>());
    ocl::ConeCutter *cutter = cjs->GetInternalInstance(info);
    this->actualClass_->setCutter(cutter);
}

void PathDropCutterJS::setSampling(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::Number s = info[0].As<Napi::Number>();
    this->actualClass_->setSampling(s.DoubleValue());
}

void PathDropCutterJS::setZ(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::HandleScope scope(env);
    Napi::Number s = info[0].As<Napi::Number>();
    this->actualClass_->setZ(s.DoubleValue());
}

Napi::Value PathDropCutterJS::getCLPoints(const Napi::CallbackInfo &info)
{
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
    this->actualClass_->run();
}
