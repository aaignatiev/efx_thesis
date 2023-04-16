// This is a -*- C++ -*- header file.

/* RegularTriangulationWithCdd.h -- Regular triangulation using CDD
	       
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

#ifndef REGULARTRIANGULATIONWITHCDD_H
#define REGULARTRIANGULATIONWITHCDD_H

#include "cone.h"
#include "barvinok/barvinok.h"
#include "cone_consumer.h"

void
triangulate_cone_with_cdd(listCone *cone,
			  BarvinokParameters *Parameters,
			  ConeConsumer &consumer);

#endif
