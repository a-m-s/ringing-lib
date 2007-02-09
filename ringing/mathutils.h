// -*- C++ -*- mathutils.h - Mathematical utility functions
// Copyright (C) 2001, 2002, 2005, 2007 Martin Bright <martin@boojum.org.uk>
// and Richard Smith <richard@ex-parrot.com>

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// $Id$


#ifndef RINGING_MATH_H
#define RINGING_MATH_H

#include <ringing/common.h>

#if RINGING_HAS_PRAGMA_ONCE
#pragma once
#endif

#if RINGING_HAS_PRAGMA_INTERFACE
#pragma interface
#endif

RINGING_START_NAMESPACE
  
RINGING_USING_STD

// Declare a couple of random functions for hcf and lcm
RINGING_API int hcf(int a, int b);
inline RINGING_API int lcm(int a, int b)
{
  // Check for a couple of quick cases
  if(a == 1) return b;
  if(b == 1) return a;
  return a * b / hcf(a,b);
}

RINGING_API unsigned factorial(unsigned n);
RINGING_API unsigned fibonacci(unsigned n);

// Uniformly distributed, 0 <= random_int(max) < max
RINGING_API unsigned random_int(unsigned max);

// Returns true with probability ptrue
RINGING_API bool random_bool( double ptrue = 0.5 );

// Supply an alternative random number generator
// If rand is NULL, then just return the existing one
RINGING_API pair<int (*)(), int> random_fn( int (*randfn)(), int max_rand );


RINGING_END_NAMESPACE

#endif
