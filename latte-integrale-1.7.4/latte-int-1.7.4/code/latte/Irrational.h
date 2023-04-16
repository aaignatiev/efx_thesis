// This is a -*- C++ -*- header file.

/* Irrational.cpp -- Irrationalize (perturbate) cones

   Copyright 2006 Matthias Koeppe

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

#ifndef IRRATIONAL_H
#define IRRATIONAL_H

#include "cone.h"

/* Compute a new VERTEX for the simplicial CONE without changing the
   set of integer points in CONE, such that all these points are in
   the interior of CONE.  Also compute a rational number LENGTH such
   that all points X in the open cube with

                 || X - VERTEX ||_infty < LENGTH

   have the same property as VERTEX.  Return VERTEX. */	 

rationalVector *
computeConeStabilityCube(listCone *cone, int numOfVars,
			 ZZ &length_numerator, ZZ &length_denominator);

/* Move the vertex of the simplicial CONE without changing the set of
   integer points in CONE, such that all these points are in the
   relative interior of CONE.  More strongly, every CONE_2 whose apex
   is the same as that of CONE with index(CONE_2) <= index(CONE) has
   integer points only in the relative interior, not on proper faces.
*/
void
irrationalizeCone(listCone *cone, int numOfVars);

/* Likewise, for the whole list of cones. */
void
irrationalizeCones(listCone *cones, int numOfVars);

/* Return whether CONE does not contain any integer points on (the
   affine hulls of) its proper faces.
*/
bool
isConeIrrational(listCone *cone, int numOfVars);

/* Functions can throw this exception when they discover the passed
   cone was not irrational. */
struct NotIrrationalException {};

/* Check that CONE is irrational; otherwise throw a
   NotIrrationalException exception. */
void
checkConeIrrational(listCone *cone, int numOfVars);

/* Check that CONE1 and CONE1 with NEW_VERTEX contain the same integer points. */
void
assertConesIntegerEquivalent(listCone *cone1, rationalVector *new_vertex,
			     int numOfVars, const char *message);

#endif

