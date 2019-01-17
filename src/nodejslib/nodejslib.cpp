#include <napi.h>

#include "point_js.hpp"
#include "triangle_js.hpp"
#include "bbox_js.hpp"
#include "path_js.hpp"
#include "line_js.hpp"

#include "stlsurf_js.hpp"
#include "stlreader_js.hpp"

#include "waterline_js.hpp"
#include "adaptivepathdropcutter_js.hpp"
#include "adaptivewaterline_js.hpp"

#include "cylcutter_js.hpp"
#include "ballcutter_js.hpp"
#include "bullcutter_js.hpp"
#include "conecutter_js.hpp"

using namespace Napi;

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  PointJS::Init(env, exports);
  TriangleJS::Init(env, exports);
  BboxJS::Init(env, exports);
  PathJS::Init(env, exports);
  LineJS::Init(env, exports);

  WaterlineJS::Init(env, exports);
  AdaptivePathDropCutterJS::Init(env, exports);
  AdaptiveWaterlineJS::Init(env, exports);

  STLSurfJS::Init(env, exports);
  STLReaderJS::Init(env, exports);

  CylCutterJS::Init(env, exports);
  BallCutterJS::Init(env, exports);
  BullCutterJS::Init(env, exports);
  ConeCutterJS::Init(env, exports);
  return exports;
}

NODE_API_MODULE(opencamlib, InitAll)