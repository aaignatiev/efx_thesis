/* IntCombEnum.cpp -- Enumerate integer combinations

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

#include "IntCombEnum.h"
#include <stdlib.h> 
#include <string.h> 
#include <iostream>

using namespace std;

/* use initialization list which is faster */
IntCombEnum::IntCombEnum(int *u, int l) :
  upper_bound(u), cur_col(l - 1), len(l)
{
   /*
    * initialize memory for the start position and "next" position
    * set start and end position to be the zero vector.
    */
   prev = new int[l];
   next = new int[l];
   memset(prev, 0, l*sizeof(int));
   memset(next, 0, l*sizeof(int));

   //print_debug();
}

IntCombEnum::~IntCombEnum() {
   /* upper_bound is deleted by creator */ 
   delete [] prev;
   delete [] next;
}

/* sets zeros from column lower_col to column upper_col, inclusive */    
void IntCombEnum::set_zero(int *v, int lower_col, int upper_col) {
    memset(&v[lower_col], 0, (upper_col - lower_col + 1)*sizeof(int));
} 

int *IntCombEnum::getNext() {
   /* check if there are any more integer combinations */
   if (is_last()) {
      // cerr << "IntCombEnum::getNext -- found last integer combination.\n";
      return NULL;
   }
   /* set next initially equal to the prev iteration */
   copy_comb(next, prev);
   if (prev[cur_col] == upper_bound[cur_col]) {
      while (cur_col >= 0) {
         /* sets all cells after cur_col to 0 */
         set_zero(next, cur_col, (len - 1));
         /* decrement cur_col */
         cur_col--;
         if ((cur_col >= 0) && (prev[cur_col] < upper_bound[cur_col])) {
            next[cur_col]++;
            cur_col = (len - 1);
            break;
         } 
      } 
   } else {
      next[cur_col]++;
   }
   copy_comb(prev, next);
   //print_debug();

   return next;
}

void IntCombEnum::decrementUpperBound() {
   for (int i = 0; i < len; i++) {
      upper_bound[i]--;
   }
}

void IntCombEnum::copy_comb(int *dest, int *src) {
   for (int i = 0; i < len; i++) {
      dest[i] = src[i];
   }
}

int IntCombEnum::is_last() {
   int is_zero = 1; 
   /*
    * The enumerator is finished when the prev integer combination is all
    * 0s AND the active column is -1.
    */  
   for (int i = 0; i < len; i++) {
      if (prev[i] != 0) {
         is_zero = 0;
      }
   }
   if ((is_zero) && (cur_col < 0)) {
      return (1);
   }
   return (0);
}

void IntCombEnum::print_debug() {
   cerr << "IntCombEnum:: Begin print_debug...\n";
   cerr << "len = " << len << ",cur_col = " << cur_col << "\n";
   cerr << "next = ";
   for (int i = 0; i < len; i++) {
      cerr << next[i] << ",";
   }
   cerr << "\n";
   cerr << "prev = ";
   for (int i = 0; i < len; i++) {
      cerr << prev[i] << ",";
   }
   cerr << "\n";
   cerr << "upper_bound = ";
   for (int i = 0; i < len; i++) {
      cerr << upper_bound[i] << ",";
   }
   cerr << "\n";
   cerr << "IntCombEnum:: End print_debug...\n";
}
