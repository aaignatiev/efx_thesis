// This is a -*- C++ -*- header file.
   
/* Polyhedron.h -- Representation of a polyhedron.

   Copyright 2006, 2007 Matthias Koeppe

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

#ifndef POLYHEDRON__H
#define POLYHEDRON__H

#include "cone.h"
#include "cone_consumer.h"
#include "print.h"

class Polyhedron {
public:
  int numOfVars;		/* Number of variables, including
				   homogenization */
  bool homogenized;  /* If true, CONES represent the homogenization of
			the polyhedron.  Otherwise, CONES represent
			the supporting cones of all vertices of the
			polyhedron. */
  bool dualized;	       /* Whether CONES have been dualized. */
  bool unbounded;	       /* Whether the polyhedron is
				  unbounded. */
  listCone *cones;
  ConeTransducer *projecting_up_transducer;   /* If non-NULL, a cone
						 transducer that
						 "projects up" cones
						 into original space" */

  Polyhedron(const Polyhedron & p):numOfVars(p.numOfVars), homogenized(p.homogenized),
		  dualized(p.dualized), unbounded(p.unbounded)
  {
	  cones = copyListCone(p.cones);//make a deep copy of the list of cones.
	  projecting_up_transducer = p.projecting_up_transducer; //not sure if a deep copy is neded or not.Brandon Nov 13 2010.
  }//copy constructor.

  Polyhedron() : numOfVars(0), homogenized(false), dualized(false),
		 unbounded(false), cones(0), projecting_up_transducer(0) {}
  ~Polyhedron() { freeListCone(cones); }

  //good for debugging.
  void printPolyhedron()
  {
	  cerr << "numofvars   : " << numOfVars << endl;
	  cerr << "homogenized : " << homogenized << endl;
	  cerr << "dualized    : " << dualized << endl;
	  cerr << "unbounded   : " << unbounded << endl;
	  printListCone(cones, numOfVars);
  }
};

#endif
