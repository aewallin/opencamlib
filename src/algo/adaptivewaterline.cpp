/*  $Id$
 * 
 *  Copyright (c) 2010-2011 Anders Wallin (anders.e.e.wallin "at" gmail.com).
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
#include <array>
#include <deque>
#include <vector>



#include "parallel.hpp"

#include "millingcutter.hpp"
#include "point.hpp"
#include "triangle.hpp"
#include "line.hpp"
#include "path.hpp"

#include "adaptivewaterline.hpp"
#include "weave.hpp"
#include "fiberpushcutter.hpp"


namespace ocl
{

//********   ********************** */

AdaptiveWaterline::AdaptiveWaterline() {
    subOp.clear();
    subOp.push_back(std::make_unique<FiberPushCutter>());
    subOp.push_back(std::make_unique<FiberPushCutter>());
    subOp[0]->setXDirection();
    subOp[1]->setYDirection();
    nthreads = detail::default_threads();
    sampling = 1.0;
    min_sampling = 0.1;
    cosLimit = 0.999;
}

AdaptiveWaterline::~AdaptiveWaterline() {}

void AdaptiveWaterline::run() {
    adaptive_sampling_run();
    weave_process(); // in base-class Waterline
}

void AdaptiveWaterline::run2() {
    adaptive_sampling_run();
    weave_process2(); // in base-class Waterline
}

void AdaptiveWaterline::adaptive_sampling_run() {
    validateInputs(true);
    reset();
    nCalls = 0;
    if (surf->tris.empty()) return;
    minx = surf->bb.minpt.x - 2*cutter->getRadius();
    maxx = surf->bb.maxpt.x + 2*cutter->getRadius();
    miny = surf->bb.minpt.y - 2*cutter->getRadius();
    maxy = surf->bb.maxpt.y + 2*cutter->getRadius();
    const Line line(Point(minx,miny,zh), Point(maxx,maxy,zh));
    const LineSpan linespan(line);
    if (detail::thread_count(nthreads, 4) > 1) {
        adaptive_sampling_batch(linespan);
        return;
    }
    const Point first = linespan.getPoint(0.0), last = linespan.getPoint(1.0);
    // Keep the depth-first traversal for serial builds: it needs only a stack
    // of intermediate fibers, and provides the same ordered output as batching.
    nCalls = detail::parallel_sum(2, 1, [&](std::size_t direction) {
        if (direction == 0) {
            xfibers.clear();
            Fiber start(Point(minx,first.y,zh), Point(maxx,first.y,zh));
            Fiber stop(Point(minx,last.y,zh), Point(maxx,last.y,zh));
            subOp[0]->run(start);
            subOp[0]->run(stop);
            xfibers.push_back(start);
            xfiber_adaptive_sample(&linespan, 0.0, 1.0, start, stop);
        } else {
            yfibers.clear();
            Fiber start(Point(first.x,miny,zh), Point(first.x,maxy,zh));
            Fiber stop(Point(last.x,miny,zh), Point(last.x,maxy,zh));
            subOp[1]->run(start);
            subOp[1]->run(stop);
            yfibers.push_back(start);
            yfiber_adaptive_sample(&linespan, 0.0, 1.0, start, stop);
        }
        return static_cast<unsigned int>(subOp[direction]->getCalls());
    });
}

void AdaptiveWaterline::adaptive_sampling_batch(const Span& span) {
    struct Sample {
        double t;
        unsigned int direction;
        Fiber fiber;
    };
    struct Segment {std::size_t start, stop, mid;};
    // Stable storage lets neighboring segments share their already evaluated
    // endpoints without copying interval/contact data or recalculating fibers.
    std::deque<Sample> samples;
    std::array<std::vector<std::size_t>, 2> retained;
    const auto append = [&](double t, unsigned int direction) {
        const Point p = span.getPoint(t);
        samples.push_back({t, direction, direction == 0
            ? Fiber(Point(minx,p.y,zh), Point(maxx,p.y,zh))
            : Fiber(Point(p.x,miny,zh), Point(p.x,maxy,zh))});
        return samples.size() - 1;
    };
    const auto evaluate = [&](std::size_t first) {
        return detail::parallel_sum(samples.size() - first, nthreads, [&](std::size_t offset) {
            auto& sample = samples[first + offset];
            const auto& operation = static_cast<const FiberPushCutter&>(*subOp[sample.direction]);
            return operation.pushCutter2(sample.fiber);
        });
    };
    std::vector<Segment> pending;
    for (unsigned int direction = 0; direction < 2; ++direction) {
        const auto start = append(0.0, direction), stop = append(1.0, direction);
        retained[direction].push_back(start);
        pending.push_back({start, stop, 0});
    }
    nCalls = evaluate(0);
    while (!pending.empty()) {
        const auto first = samples.size();
        std::vector<Segment> active;
        active.reserve(pending.size());
        for (auto segment : pending) {
            const auto& start = samples[segment.start];
            const auto& stop = samples[segment.stop];
            const double mid = start.t + (stop.t - start.t) / 2.0;
            if (mid <= start.t || mid >= stop.t) {
                retained[start.direction].push_back(segment.stop);
            } else {
                segment.mid = append(mid, start.direction);
                active.push_back(segment);
            }
        }
        // All workers read immutable geometry and write their own midpoint.
        // Reduce counters after joining; no shared FiberPushCutter counters.
        nCalls += evaluate(first);
        pending.clear();
        for (const auto& segment : active) {
            const auto& start = samples[segment.start];
            const auto& stop = samples[segment.stop];
            const auto& mid = samples[segment.mid];
            const double step = start.direction == 0
                ? fabs(start.fiber.p1.y - stop.fiber.p1.y)
                : fabs(start.fiber.p1.x - stop.fiber.p1.x);
            bool subdivide = step > sampling;
            if (!subdivide) {
                if (flat(start.fiber, mid.fiber, stop.fiber))
                    retained[start.direction].push_back(segment.stop);
                else
                    subdivide = step > min_sampling;
                // Match the recursive sampler: a non-flat segment at minimum
                // spacing contributes no stop fiber.
            }
            if (subdivide) {
                pending.push_back({segment.start, segment.mid, 0});
                pending.push_back({segment.mid, segment.stop, 0});
            }
        }
    }
    for (unsigned int direction = 0; direction < 2; ++direction) {
        auto& indices = retained[direction];
        std::sort(indices.begin(), indices.end(), [&](std::size_t a, std::size_t b) {
            return samples[a].t < samples[b].t;
        });
        auto& fibers = direction == 0 ? xfibers : yfibers;
        fibers.reserve(indices.size());
        for (auto index : indices) fibers.push_back(samples[index].fiber);
    }
}


void AdaptiveWaterline::xfiber_adaptive_sample(const Span* span, double start_t, double stop_t, const Fiber& start_f, const Fiber& stop_f) {
    const double mid_t = start_t + (stop_t-start_t)/2.0; // mid point sample
    if (mid_t <= start_t || mid_t >= stop_t) {
        xfibers.push_back(stop_f);
        return;
    }
    //std::cout << "xfiber sample= ( " << start_t << " , " << stop_t << " ) \n";
    double mid_y = span->getPoint( mid_t ).y;
    Point mid_p1 = Point( minx, mid_y,  zh );
    Point mid_p2 = Point( maxx, mid_y,  zh );
    Fiber mid_f = Fiber( mid_p1, mid_p2 );
    subOp[0]->run( mid_f );
    double fw_step = fabs( start_f.p1.y - stop_f.p1.y ) ;
    if ( fw_step > sampling ) { // above minimum step-forward, need to sample more
        xfiber_adaptive_sample( span, start_t, mid_t , start_f, mid_f  );
        xfiber_adaptive_sample( span, mid_t  , stop_t, mid_f  , stop_f );
    } else if ( !flat(start_f,mid_f,stop_f)   ) {
        if (fw_step > min_sampling) { // not a flat segment, and we have not reached maximum sampling
            xfiber_adaptive_sample( span, start_t, mid_t , start_f, mid_f  );
            xfiber_adaptive_sample( span, mid_t  , stop_t, mid_f  , stop_f );
        }
    } else {
        xfibers.push_back(stop_f);
    }
}

void AdaptiveWaterline::yfiber_adaptive_sample(const Span* span, double start_t, double stop_t, const Fiber& start_f, const Fiber& stop_f) {
    const double mid_t = start_t + (stop_t-start_t)/2.0; // mid point sample
    if (mid_t <= start_t || mid_t >= stop_t) {
        yfibers.push_back(stop_f);
        return;
    }
    //std::cout << "yfiber sample= ( " << start_t << " , " << stop_t << " ) \n";
    double mid_x = span->getPoint( mid_t ).x;
    Point mid_p1 = Point( mid_x, miny,  zh );
    Point mid_p2 = Point( mid_x, maxy, zh );
    Fiber mid_f = Fiber( mid_p1, mid_p2 );
    subOp[1]->run( mid_f );
    double fw_step = fabs( start_f.p1.x - stop_f.p1.x ) ;
    if ( fw_step > sampling ) { // above minimum step-forward, need to sample more
        yfiber_adaptive_sample( span, start_t, mid_t , start_f, mid_f  );
        yfiber_adaptive_sample( span, mid_t  , stop_t, mid_f  , stop_f );
    } else if ( !flat(start_f,mid_f,stop_f)   ) {
        if (fw_step > min_sampling) { // not a flat segment, and we have not reached maximum sampling
            yfiber_adaptive_sample( span, start_t, mid_t , start_f, mid_f  );
            yfiber_adaptive_sample( span, mid_t  , stop_t, mid_f  , stop_f );
        }
    } else {
        yfibers.push_back(stop_f);
    }
}

// flat predicate to determine when we subdivide
bool AdaptiveWaterline::flat( const Fiber& start, const Fiber& mid, const Fiber& stop ) const {
    if ( start.size() != stop.size() ) // start, mid, and stop need to have same size()
        return false;
    else if ( start.size() != mid.size() )
        return false;
    else if ( mid.size() != stop.size() )
        return false;
    else {
        if (!start.empty() ) { // all now have same size
            assert( start.size() == stop.size() && start.size() == mid.size() );
            for (unsigned int n=0;n<start.size();++n) {
                // now check for angles between cl-points (NOTE: cl-points might not belong to same loop?)
                // however, errors here only lead to dense sampling which is harmless (but slow)
                if ( (!flat( start.upperCLPoint(n) , mid.upperCLPoint(n) , stop.upperCLPoint(n) )) )
                    return false;
                else if (!flat( start.lowerCLPoint(n) , mid.lowerCLPoint(n) , stop.lowerCLPoint(n) )) 
                    return false;
            }
        }
        return true;
    }
}


bool AdaptiveWaterline::flat(Point start_cl, Point mid_cl, Point stop_cl)  const {
    Point v1 = mid_cl-start_cl;
    Point v2 = stop_cl-mid_cl;
    v1.normalize();
    v2.normalize();
    double dotprod =  v1.dot(v2) ;
    return (dotprod>cosLimit);
}

}// end namespace
// end file adaptivewaterline.cpp
