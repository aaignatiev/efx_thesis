// This is a -*- C++ -*- header file.

/* IntCombEnum.h -- Enumerate integer combinations

   Copyright 2006 Susan Margulies

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

#ifndef INT_COMB_ENUM_H
#define INT_COMB_ENUM_H

/*
 * This class enumerates all of the integer combinations of a given 
 * vector. For example, the integer combinations of [1 1] are
 * [0 0], [0 1], [1 0] and [1 1]
 */

class IntCombEnum
{  
   private:
      /* stores the maximum allowed integer for a given bit */
      int *upper_bound;
      /* current int combination */ 
      int *prev; 
      /* current int combination */ 
      int *next; 
      /* current column to increment */ 
      int cur_col; 
      /* length of vector */ 
      int len; 
      /* copy current integer combination to next */ 
      void copy_comb(int *, int *);
      /* sets zeros from column i to column j */ 
      void set_zero(int *, int, int);
      /* if the vector is the last  */ 
      int is_last();
      /* for debugging purposes  */ 
      void print_debug();

   public:

      /* constructor */
      IntCombEnum(int *, int);
      /* destructor */
      ~IntCombEnum(); 
      /*
       * decrements all upper bounds by 1. For use with counting 
       * lattice points.
       */ 
      void decrementUpperBound();
      /* get next integer combination */ 
      int *getNext();
      /* get size of integer combination */ 
      int getSize() {return len;};
};

#endif

