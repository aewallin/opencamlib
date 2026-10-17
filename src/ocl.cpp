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

#include "ocl.hpp"
#include "parallel.hpp"
#include "version_string.hpp"

namespace ocl
{
  int max_threads()
  {
#ifdef OCL_WASM_THREAD_POOL_SIZE
    return OCL_WASM_THREAD_POOL_SIZE + 1;
#elif defined(_OPENMP)
    return omp_get_max_threads();
#endif
    return 1;
  }

  std::string version()
  {
    return VERSION_STRING;
  }
}