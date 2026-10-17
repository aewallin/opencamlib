/*  $Id$
 * 
 *  Copyright (c) 2010 Anders Wallin (anders.e.e.wallin "at" gmail.com).
 *  
 *  This file is part of OpenCAMlib 
 *  (see https://github.com/aewallin/opencamlib).
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include <algorithm>
#include "parallel.hpp"

#include "millingcutter.hpp"
#include "clpoint.hpp"
#include "pointdropcutter.hpp"
#include "adaptivepathdropcutter.hpp"

namespace ocl
{

//********   ********************** */

AdaptivePathDropCutter::AdaptivePathDropCutter() {
    // std::cout << " AdaptivePathDropCutter() " << std::endl;
    cutter = NULL;
    surf = NULL;
    path = NULL;
    minimumZ = 0.0;
    subOp.clear();
    subOp.push_back(std::make_unique<PointDropCutter>()); // we delegate to PointDropCutter, who does the heavy lifting
    nthreads = subOp[0]->getThreads();
    sampling = 0.1;
    min_sampling = 0.01;
    cosLimit = 0.999;
}

AdaptivePathDropCutter::~AdaptivePathDropCutter() {
    // std::cout << " ~AdaptivePathDropCutter() " << std::endl;
    subOp.clear();
}

void AdaptivePathDropCutter::run() {
    adaptive_sampling_run();
}

void AdaptivePathDropCutter::adaptive_sampling_run() {
    validateInputs(true);
    if (!path) throw std::logic_error("Set a path before running");
    for (const Span* span : path->span_list) {
        if (!span || !std::isfinite(span->length2d()))
            throw std::invalid_argument("Path contains an invalid span");
    }
    clpoints.clear();
    const auto& dropcutter = static_cast<const PointDropCutter&>(*subOp[0]);
    nCalls = 0;
    const auto threads = detail::thread_count(nthreads, path->span_list.size());
    if (threads <= 1) {
        for (const Span* span : path->span_list)
            nCalls += sample_span(span, clpoints, dropcutter);
        return;
    }
    const std::vector<const Span*> spans(path->span_list.begin(), path->span_list.end());
    std::vector<std::vector<CLPoint>> results(spans.size());
    // Each span owns its output; concatenate in path order after the parallel work.
    nCalls = detail::parallel_sum(spans.size(), threads, [&](std::size_t i) {
        return sample_span(spans[i], results[i], dropcutter);
    });
    std::size_t size = 0;
    for (const auto& points : results) size += points.size();
    clpoints.reserve(size);
    for (const auto& points : results)
        clpoints.insert(clpoints.end(), points.begin(), points.end());
}

int AdaptivePathDropCutter::sample_span(const Span* span, std::vector<CLPoint>& points,
                                      const PointDropCutter& dropcutter) const {
    CLPoint start = span->getPoint(0.0);
    CLPoint stop = span->getPoint(1.0);
    start.z = stop.z = minimumZ;
    int calls = dropcutter.dropCutter(start);
    calls += dropcutter.dropCutter(stop);
    points.push_back(start);
    return calls + adaptive_sample(span, 0.0, 1.0, start, stop, points, dropcutter);
}

void AdaptivePathDropCutter::adaptive_sample(const Span* span, double start_t, double stop_t, const CLPoint& start_cl, const CLPoint& stop_cl) {
    const auto& dropcutter = static_cast<const PointDropCutter&>(*subOp[0]);
    nCalls += adaptive_sample(span, start_t, stop_t, start_cl, stop_cl, clpoints, dropcutter);
}

int AdaptivePathDropCutter::adaptive_sample(const Span* span, double start_t, double stop_t,
                                          const CLPoint& start_cl, const CLPoint& stop_cl,
                                          std::vector<CLPoint>& points,
                                          const PointDropCutter& dropcutter) const {
    const double mid_t = start_t + (stop_t-start_t)/2.0; // mid point sample
    if (mid_t <= start_t || mid_t >= stop_t) {
        points.push_back(stop_cl);
        return 0;
    }
    CLPoint mid_cl = span->getPoint(mid_t);
    mid_cl.z = minimumZ;
    //std::cout << " apdc sampling at " << mid_t << "\n";
    int calls = dropcutter.dropCutter(mid_cl);
    double fw_step = (stop_cl-start_cl).xyNorm();
    if ( (fw_step > sampling) || // above minimum step-forward, need to sample more
          ( (!flat(start_cl,mid_cl,stop_cl)) && (fw_step > min_sampling) ) ) { // OR not flat, and not max sampling
        calls += adaptive_sample(span, start_t, mid_t, start_cl, mid_cl, points, dropcutter);
        calls += adaptive_sample(span, mid_t, stop_t, mid_cl, stop_cl, points, dropcutter);
    } else {
        points.push_back(stop_cl);
    }
    return calls;
}

bool AdaptivePathDropCutter::flat(const CLPoint& start_cl, const CLPoint& mid_cl, const CLPoint& stop_cl) const {
    Point v1 = mid_cl-start_cl;
    Point v2 = stop_cl-mid_cl;
    v1.normalize();
    v2.normalize();
    return (v1.dot(v2) > cosLimit);
}

} // end namespace

