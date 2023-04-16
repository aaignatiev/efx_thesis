// This is a -*- C++ -*- header file.

/* triangulate.h -- Compute triangulations.

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

#ifndef TRIANGULATION_TRIANGULATE__H
#define TRIANGULATION_TRIANGULATE__H

#include <ostream>
#include "cone.h"
#include "barvinok/barvinok.h"
#include "cone_consumer.h"

BarvinokParameters::TriangulationType
triangulation_type_from_name(const char *name);

/* Parse a standard command-line option related to triangulations
   and fill the correspodingin slots in PARAMS.
   Return whether a supported option was found and handled.
*/
bool
parse_standard_triangulation_option(const char *arg,
				    BarvinokParameters *params);

void
show_standard_triangulation_options(std::ostream &stream);

/* CONE must be a full-dimensional pointed cone given by its minimal
   generators (rays).
   
   Triangulate CONE using some method specified in PARAMS, discarding
   lower-dimensional cones.  Return a linked list of freshly allocated
   cones.
   
   CONE is not consumed.
*/
listCone *
triangulateCone(listCone *cone, int numOfVars,
		BarvinokParameters *params);

/* Likewise, but feed the resulting cones one by one to the CONSUMER.
*/
void
triangulateCone(listCone *cone, int numOfVars,
		BarvinokParameters *params,
		ConeConsumer &consumer);

/* A cone consumer that triangulates cones and passes the results to a
   continuation. */

class TriangulatingConeTransducer : public ConeTransducer {
public:
  TriangulatingConeTransducer(BarvinokParameters *some_parameters);
  int ConsumeCone(listCone *cone);
private:
  BarvinokParameters *params;
};

#endif
