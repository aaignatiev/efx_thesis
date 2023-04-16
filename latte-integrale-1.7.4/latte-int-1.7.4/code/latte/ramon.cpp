/* ramon.cpp -- Helper functions

   Copyright 2002-2004 Raymond Hemmecke
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

#include <fstream>
#include "ramon.h"

/* ----------------------------------------------------------------- */
vec_ZZ createVector(int numOfVars) {
  vec_ZZ w;

  w.SetLength(numOfVars);
  return (w);
}
/* ----------------------------------------------------------------- */
vec_ZZ* createArrayVector(int numOfVectors) {
  vec_ZZ* w;

  w = new vec_ZZ[numOfVectors+1];
  if (w==0) {
    cerr << "Memory exhausted?" << endl;
    exit(1);
  }
  return (w);
}
/* ----------------------------------------------------------------- */
listVector* createListVector(vec_ZZ v) {
  return (appendVectorToListVector(v,0));
}
/* ----------------------------------------------------------------- */
 void removeListVector( listVector* p )
        {

            if( p->rest != NULL )
            {
                listVector *oldNode = p->rest;
                p->rest = p->rest->rest;  // Bypass deleted node
                delete oldNode;
            }
        }
/* ----------------------------------------------------------------- */
listVector* updateBasis(listVector *v, listVector *endBasis) {
  endBasis->rest = v;
  v->rest=0;
  endBasis = endBasis->rest;
  return (endBasis);
}
/* ----------------------------------------------------------------- */
int isVectorEqualToVector(vec_ZZ v, vec_ZZ w, int numOfVars) {
  int i;

//    if ((v==0) || (w==0)) return (0);
  for (i=0; i<numOfVars; i++) if (!(v[i]==w[i])) return (0);
  return (1);
}
/* NOTE: This function changes the LIST pointer. */ 
/* ----------------------------------------------------------------- */
int isVectorInListVector(vec_ZZ v, listVector* LIST, int numOfVars) {
  vec_ZZ w;

  while (LIST) {
    w = LIST->first;
    LIST = LIST->rest;
    if (isVectorEqualToVector(v,w,numOfVars)==1) return (1);
  }
  return (0);
}

/*
 * This function does NOT change the LIST pointer 
 */
int
isVectorInListVector(const vec_ZZ & v, listVector *list) {
   int numOfVars = v.length();
   listVector *tmp_list = list;

   while (tmp_list) {
      if (!isVectorEqualToVector(v, tmp_list->first, numOfVars)) {
         return (0);
      }
      tmp_list = tmp_list->rest;
   }
   return (1);
}

/* ----------------------------------------------------------------- */
listVector* readListVector(char *fileName, int* numOfVars) {
  int i,j,numOfVectors;
  listVector *basis, *endBasis;
  vec_ZZ b;

  /* Reads numOfVars, numOfVectors, list of vectors. */

  //setbuf(stdout,0);
  ifstream in(fileName);
  if(!in){
    cerr << "Cannot open input file in readListVector." << endl;
    exit(1);
  }

  in >> numOfVectors;
  in >> (*numOfVars);

  basis = createListVector(createVector(*numOfVars));
  endBasis = basis;

  for (i=0; i<numOfVectors; i++) {
    b=createVector(*numOfVars);
    for (j=0; j<(*numOfVars); j++) in >> b[j];
    endBasis->rest = createListVector(b);
    endBasis=endBasis->rest;
  }

/*  printf("List of vectors:\n");
  printf("================\n");
  printListVector(basis->rest,numOfVars); */

  return(basis->rest);
}
/* ----------------------------------------------------------------- */
/*
 * Testing whether or not two lists contain exactly the same
 * points. This is a debugging diagnostic function.
 */
int
isEqual(listVector *first, listVector *second) {
   int first_len = lengthListVector(first);
   int second_len = lengthListVector(second);
   listVector *tmp_list = first;
    
   if (first_len != second_len) {
      return (0);
   }
  
   /*
    * since the two lists have the same length, if every point in one list
    * is contained in the other list, the two lists must be equal.
    */ 
   while (tmp_list) {
      if (!isVectorInListVector(tmp_list->first, second)) {
         return (0);
      }
      tmp_list = tmp_list->rest;
   }
   return (1);
}
