// This is a -*- C++ -*- header file.

/* latte_4ti2_zsolve.h -- Interface to 4ti2's zsolve component
	       
   Copyright 2007 Matthias Koeppe

   This file is part of LattE.
   
   LattE is free software; you can redistribute it and/or modify it
   under the terms of the version 2 of the GNU General Public License
   as published by the Free Software Foundation.

   LattE is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with LattE; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*/

#ifndef LATTE_4TI2_ZSOLVE_H
#define LATTE_4TI2_ZSOLVE_H

#include "cone.h"

#include "zsolve/LinearSystem.hpp"

/* Create a 4ti2/zsolve LinearSystem representing the homogeneous
   inequalities given by FACETS. */
_4ti2_zsolve_::LinearSystem<int> *
facets_to_4ti2_zsolve_LinearSystem(listVector *facets,
				   listVector *equalities,
				   int numOfVars);

#endif
