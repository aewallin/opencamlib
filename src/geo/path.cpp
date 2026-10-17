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


#include "path.hpp"

namespace ocl
{

Path::Path() {
}

Path::Path(const Path &p) {
    for (const Span* span : p.span_list) {
        if (!span) throw std::invalid_argument("Path contains a null span");
        append(*span);
    }
}

Path::Path(Path&& p) noexcept {
    swap(p);
}

Path& Path::operator=(Path p) {
    swap(p);
    return *this;
}

void Path::swap(Path& p) noexcept {
    span_list.swap(p.span_list);
    owned_spans.swap(p.owned_spans);
}

Path::~Path() = default;

void Path::append_owned(std::unique_ptr<Span> span) {
    if (!span) throw std::invalid_argument("Cannot append a null span");
    owned_spans.push_back(std::move(span));
    try {
        span_list.push_back(owned_spans.back().get());
    } catch (...) {
        owned_spans.pop_back();
        throw;
    }
}

void Path::append(const Line &l) {
    append_owned(std::make_unique<LineSpan>(l));
}

void Path::append(const Arc &a) {
    append_owned(std::make_unique<ArcSpan>(a));
}

void Path::append(const Span& span) { append_owned(span.clone()); }

} // end namespace
// end file path.cpp
