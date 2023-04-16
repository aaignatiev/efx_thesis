// This is a -*- C++ -*- header file.
   
/* ReadLatteStyle.h -- Read input data in the LattE style

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

#ifndef READLATTESTYLE__H
#define READLATTESTYLE__H

#include <fstream>
#include "latte_cddlib.h"
#include "Polyhedron.h"

/* Read a file in LattE format and return a cddlib matrix.

   If HOMOGENIZE is true, add a zero column in front.  This is useful
   for reading in 4ti2-style matrix files.

   If NONNEGATIVITY is true, add non-negativity constraints to all
   variables. 
*/
dd_MatrixPtr ReadLatteStyleMatrix(istream &f, bool vrep,
				  bool homogenize,
				  const char *file_description = "",
				  bool nonnegativity = false);
dd_MatrixPtr ReadLatteStyleMatrix(const char *fileName, bool vrep,
				  bool homogenize,
				  bool nonnegativity = false);

/* Write a cddlib matrix to a file in LattE format. */
void WriteLatteStyleMatrix(ostream &f, dd_MatrixPtr matrix);
void WriteLatteStyleMatrix(const char *fileName, dd_MatrixPtr matrix);

#endif

