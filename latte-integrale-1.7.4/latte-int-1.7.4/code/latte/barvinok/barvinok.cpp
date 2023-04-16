/* barvinok.cpp -- Barvinok's decomposition of a cone.

   Copyright 2002, 2003 Ruriko Yoshida
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

#include <list>
#include <vector>

#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <math.h>
#include <time.h>

#include "ComputeOmega.h"
#include "barvinok.h"
#include "../ramon.h"
#include "../RudyResNTL.h"
#include "rational.h"
#include "convert.h"
#include "dual.h"
#include "config.h"
#include "Irrational.h"
#include "triangulation/triangulate.h"
#ifdef HAVE_EXPERIMENTS
#include "barvinok/SubspaceAvoidingDecomposition.h"
#endif

#undef SHOWDETS
#undef SHOWCONES
//#define SHOWDETS 1
//#define SHOWCONES 
#ifdef SHOWCONES
#include "print.h"
#endif

BarvinokParameters::BarvinokParameters() :
  substitution(PolynomialSubstitution),
  decomposition(DualDecomposition),
  triangulation(
#ifdef HAVE_FORTYTWO_LIB
		RegularTriangulationWith4ti2
#else
		RegularTriangulationWithCdd
#endif
		),
  triangulation_max_height(10000),
  triangulation_bias(-1),
  nonsimplicial_subdivision(false),
  triangulation_special_cone(NULL),
  triangulation_prescribed_height_data(NULL),
  triangulation_assume_fulldim(true),
  dualization(
#ifdef HAVE_FORTYTWO_LIB
	      DualizationWith4ti2
#else
	      DualizationWithCdd
#endif
	      ),
  shortvector(LatteLLL),
  smithform(
#ifdef HAVE_LIDIA
	    LidiaSmithForm
#else
	    IlioSmithForm
#endif
	    ),
  max_determinant(0),
  File_Name(NULL),
  Number_of_Variables(0),
  Flags(0),
  Cone_Index(0),
  total_time("Total time", true),
  read_time("Time for reading and preprocessing"),
  vertices_time("Time for computing vertices and supporting cones"),
  irrationalize_time("Time for irrationalizing general cones"),
  dualize_time("Time for dualizing general cones"),
  triangulate_time("Time for triangulating cones into simplicial cones"),
  decompose_time("Time for Barvinok decomposition and residue calculation"),
  num_triangulations_with_trivial_heights(0),
  num_triangulations_with_dependent_heights(0),
  num_triangulations(0)
{}

BarvinokParameters::~BarvinokParameters()
{}

void BarvinokParameters::print_statistics(ostream &s)
{
  s << read_time << vertices_time << irrationalize_time << dualize_time
    << triangulate_time << decompose_time << total_time;
}

void BarvinokParameters::deliver_number_of_lattice_points(const ZZ &number)
{
  cerr << endl << "****  The number of lattice points is: "; cerr.flush();
  cout << number; cout.flush();
  cerr << "  ****" << endl; cerr.flush();
  cout << endl;
  ofstream out("numOfLatticePoints");
  out << number << endl;
}

void Single_Cone_Parameters::print_statistics(ostream &s)
{
  BarvinokParameters::print_statistics(s);
  s << "Total number of simplicial cones: " <<
    Total_Simplicial_Cones << endl;
  if (max_determinant != 0) {
    s << "Total number of "
      << (max_determinant == 1 ? "unimodular" : "low-index")
      << " cones: " << Total_Uni_Cones << endl;
  }
  s << "Maximum depth of the decomposition tree: " << Max_Depth  << endl;
}

void DelegatingSingleConeParameters::SetConsumer(ConeConsumer *a_consumer)
{
  consumer = a_consumer;
}

int DelegatingSingleConeParameters::ConsumeCone(listCone *cone)
{
  assert(consumer != NULL);
  return consumer->ConsumeCone(cone);    
}

 /* Note:  We are dealing with the "Row space" of the
    input matrix due to NTL. */

/**********************************************************************/
vec_ZZ CheckOmega( const mat_ZZ & U, vec_ZZ & Z){

  int m;
  m = U.NumCols();
  mat_ZZ Dummy;
  Dummy.SetDims(m + 1, m);
  Dummy[0] = Z;
  ZZ d;

  for(int i = 0; i < m; i++)
    Dummy[i + 1] = U[i];

  mat_ZZ dummy;
  image(d, Dummy, dummy);

  int flag = 1, number = 0; 

  for(int i = 0; i <= m; i++)
      if(dummy[0][i] >= 0) number++;
  if(number == (m + 1))  flag = 0;

  if(flag != 0){
  number = 0;
  for(int i = 0; i <= m; i++)
     if(dummy[0][i] <= 0) number++;
  if(number == (m + 1)) flag = 0;
  }     
  if(flag == 0){
    Z = - Z; 
  }
  Dummy.kill();
  dummy.kill();
  return Z;

}
/**********************************************************************/
 
void MatrixGCD(mat_ZZ & B, long & m){
  vec_ZZ gcds;
  gcds.SetLength(m);
  for(int i = 1; i <= m; i++)
    for(int j = 1; j <= m; j++)
      if(B(i, j) != 0)
	gcds[i-1] = GCD(gcds[i-1], B(i, j));
  for(int i = 1; i <= m; i++)
    for(int j = 1; j <= m; j++)
      if(B(i, j) != 0)
	B(i, j) = B(i, j) / gcds[i-1];

}
/**********************************************************************/

/* Likewise barvinok_Single, but the cone is given as a listCone;
   the function consumes the cone. */
int
barvinok_DFS(listCone *cone, Single_Cone_Parameters *Parameters);

int
barvinok_Single(mat_ZZ B, Single_Cone_Parameters *Parameters,
		const Vertex *vertex)
{
	//cerr << "barvinok_Single Called." << endl;;
	
	long m, n;
  	m = B.NumRows();
  	n = B.NumCols();

   	if (m != n) {
	  cerr << "Input must be square (have " << m << " rows, "
	       << n << " cols). " << endl;
	  exit(2);
   	}

   	ZZ D = determinant(B);

         if( D == 0)
   	{
       		cerr << "Input must be linearly independent. " << endl;
       		exit(3);
   	}

	 Parameters->Total_Simplicial_Cones++;
	 
   	/* The following routine is to get the minimal
      	integral generators for the cone.  */

   	MatrixGCD(B, m);

   	listCone *dummy = createListCone();
   	dummy->coefficient = 1;
	dummy->determinant = D;
	dummy->vertex = new Vertex(*vertex);
	dummy->rays = transformArrayBigVectorToListVector(B, m, n);

	switch (Parameters->decomposition) {
	case BarvinokParameters::DualDecomposition:
	  // Keep the dual cones during Barvinok decomposition
	  computeDetAndFacetsOfSimplicialCone(dummy, n);
	  break;
	case BarvinokParameters::IrrationalPrimalDecomposition:
	  // Do Barvinok decomposition on the primal cones.
	  dualizeCone(dummy, Parameters->Number_of_Variables, Parameters);
	  irrationalizeCone(dummy, Parameters->Number_of_Variables);
	  break;
	case BarvinokParameters::IrrationalAllPrimalDecomposition:
	  // We already have primal cones; do Barvinok decomposition
	  // on them.
	  computeDetAndFacetsOfSimplicialCone(dummy, n);
	  break;
	default:
	  cerr << "Unknown BarvinokParameters::decomposition" << endl;
	  abort();
	}
	
	int result;
	result = barvinok_DFS(dummy, Parameters);

	return result;
}
	
/* Let GENERATOR and MAT be the same matrix, with determinant DET.
   Copy the vector Z into each row of the matrix (we are dealing with
   the row space) and compute the determinant of the resulting matrix;
   store the determinants in DETS[].  When any determinant is
   larger-or-equal than DET in absolute value, return false;
   otherwise return true.

   If NONDECREASE_OK is false (the default), stop the computation
   immediately when the result is known to be false.
*/
static bool
computeAndCheckDeterminants(const mat_ZZ &generator, const ZZ &Det,
			    const vec_ZZ &Z, int m, 
			    mat_ZZ &mat, vec_ZZ &Dets, bool nondecrease_ok = false)
{
  bool decrease = true;
  ZZ absDet = abs(Det);
  for (int i = 1; i <= m; i++) {
    /* Copy in the row */
    for(int j = 1; j <= m; j++)
      mat(i, j) = Z(j);
    /* Compute and store the determinant. */
    determinant(Dets[i - 1], mat);
    /* Restore the original row */
    for(int j = 1; j <= m; j++)
      mat(i, j) = generator(i, j);
    if (abs(Dets[i - 1]) >= absDet) {
      decrease = false;
      if (!nondecrease_ok) return false;
    }
  }
  return decrease;
}

/* Decompose the cone spanned by GENERATOR (which has determinant DET)
   according to Barvinok's theory into M (the dimension) many CONES
   and store their determinants in DETS.

   Entries with Det[i] == 0 have Cones[i] == NULL (we don't generate
   lower-dimensional cones).
*/
bool
barvinokStep(const listCone *Cone, 
	     vector <listCone *> &Cones, vec_ZZ &Dets,
	     int m, Single_Cone_Parameters *Parameters)
{
#ifdef SHOWCONES
  cerr << "############ Decomposing: ############" << endl;
  printCone((listCone*)Cone, m);
  cerr << "************** Results: **************" << endl;
#endif
  mat_ZZ generator = createConeDecMatrix(Cone, m, m);
  mat_ZZ dual = createFacetMatrix(Cone, m, m);
  /* ComputeOmega(const mat_ZZ &, long& ) computes
     an integral vector in the parallelogram. */
  mat_ZZ mat;
  vec_ZZ Z;
  switch (Parameters->shortvector) {
  case BarvinokParameters::LatteLLL: {
    Z = ComputeOmega(generator, dual, m, 0, 0);
    Z = CheckOmega(generator, Z);
     
    mat = generator;
    // FIXME: The determinants actually do not need to be computed;
    // they are given by the vector L(index) in ComputeOmega...
    // --mkoeppe, Fri Nov 24 17:01:37 MET 2006
    bool success
      = computeAndCheckDeterminants(generator, Cone->determinant, Z,
				    m, mat, Dets);
    if (!success) {
      cerr << "Second loop... " << endl;
      Z = ComputeOmega(generator, dual, m, 2, 2);
      Z = CheckOmega(generator, Z);
      success = computeAndCheckDeterminants(generator, Cone->determinant, Z,
					    m, mat, Dets);
      assert(success);
    }
    break;
  }
  case BarvinokParameters::SubspaceAvoidingLLL: {
#ifdef HAVE_EXPERIMENTS
    Z = ComputeShortVectorAvoidingSubspace(generator, dual);
    Z = CheckOmega(generator, Z);
    mat = generator;
    bool decrease
      = computeAndCheckDeterminants(generator, Cone->determinant, Z,
				    m, mat, Dets, true);
    if (!decrease)
      return false;
#else
    cerr << "SubspaceAvoidingLLL not compiled in, sorry." << endl;
    exit(1);
#endif
    break;
  }
  default:
    assert(0);
  }
  
  for(int i = 0; i < m; i++) {
    if (Dets[i] == 0)
      Cones[i] = NULL;
    else {
      Cones[i] = createListCone();
      {
	/* Create the rays: */
	/* Copy in the row */
	for(int j = 1; j <= m; j++)
	  mat(i+1, j) = Z(j);
	Cones[i]->rays
	  = transformArrayBigVectorToListVector(mat, m, m);
	/* Restore the original row */
	for(int j = 1; j <= m; j++)
	  mat(i+1, j) = generator(i+1, j);
      }
      Cones[i]->determinant = Dets[i];
      {
	/* Compute the sign: */
	long signDet = sign(Cone->determinant);
	long signDeti = sign(Dets[i]);
	Cones[i]->coefficient = Cone->coefficient * signDet * signDeti;
      }
      Cones[i]->vertex = new Vertex(*Cone->vertex);
      computeDetAndFacetsOfSimplicialCone(Cones[i], m);
#ifdef SHOWCONES
      printCone(Cones[i], m); 
#endif
    }
  }
  return true;
}

static int
deliver_cone(listCone *C, Single_Cone_Parameters *Parameters)
{
  Parameters->Total_Uni_Cones += 1;
  if ( Parameters->Total_Uni_Cones % 1000 == 0)
    cerr << Parameters->Total_Uni_Cones
	 << (Parameters->max_determinant == 0
	     ? " simplicial cones done."
	     : (Parameters->max_determinant == 1
		? " unimodular cones done."
		: " low-index cones done."))
	 << endl;
  switch (Parameters->decomposition) {
  case BarvinokParameters::DualDecomposition:
    dualizeCone(C, Parameters->Number_of_Variables, Parameters);
    return Parameters->ConsumeCone(C);
  case BarvinokParameters::IrrationalPrimalDecomposition:
  case BarvinokParameters::IrrationalAllPrimalDecomposition:
    return Parameters->ConsumeCone(C);
  default:
    cerr << "Unknown BarvinokParameters::decomposition" << endl;
    abort();
  }
}

static ZZ
criterion_abs_det(listCone *C, Single_Cone_Parameters *Parameters)
{
  switch (Parameters->decomposition) {
  case BarvinokParameters::DualDecomposition:
    return abs(C->dual_determinant);
  case BarvinokParameters::IrrationalPrimalDecomposition:
  case BarvinokParameters::IrrationalAllPrimalDecomposition:
    return abs(C->determinant);
  default:
    cerr << "Unknown BarvinokParameters::decomposition" << endl;
    abort();
  }
}

int barvinok_DFS(listCone *C, Single_Cone_Parameters *Parameters)
{
  if (Parameters->Current_Depth > Parameters->Max_Depth)
    Parameters->Max_Depth = Parameters->Current_Depth;
  
  ZZ absDet = criterion_abs_det(C, Parameters);
       	
  if (absDet == 0) {
    cerr << "barvinok_DFS: Det = 0." << endl;
    return 1;	
  }		     
  else {
    switch (Parameters->decomposition) {
    case BarvinokParameters::DualDecomposition:
      break;
    case BarvinokParameters::IrrationalPrimalDecomposition:
    case BarvinokParameters::IrrationalAllPrimalDecomposition:
      checkConeIrrational(C, Parameters->Number_of_Variables);
      break;
    default:
      cerr << "Unknown BarvinokParameters::decomposition";
      abort();
    }
    if (Parameters->max_determinant == 0
	   || absDet <= Parameters->max_determinant)
      return deliver_cone(C, Parameters);
  }
  
  //cerr << "barvinok_DFS: non-uni cone." << endl;
     
  int result = 1;
  long m = Parameters->Number_of_Variables;

  vec_ZZ Dets;
  Dets.SetLength(m);	     
  vector<listCone *> cones1(m);

  bool success = barvinokStep(C, cones1, Dets, m, Parameters);
  if (!success) {
    cerr << "Unable to decompose cone with index " << absDet;
    if (absDet <= 200000) { // "Emergency" max-index
      cerr << ", enumerating it." << endl;
      return deliver_cone(C, Parameters);      
    }
    else {
      cerr << ", giving up." << endl;
      exit(1);
    }
  }
  
  ZZ max;
  max = -1;

#ifdef SHOWDETS
  cerr << "Level " << Parameters->Current_Depth << ": Index " << absDet << " -> ";
#endif
  for(int i = 0; i < m; i++)
    {
      Dets[i] = abs(Dets[i]);
      if(Dets[i] > max)
	max = Dets[i];
      
      if (Dets[i] > 0) {
	Parameters->Current_Simplicial_Cones_Total ++;

#ifdef SHOWDETS
	cerr << criterion_abs_det(cones1[i], Parameters) << ", ";
#endif
	switch (Parameters->decomposition) {
	case BarvinokParameters::DualDecomposition:
	  break;
	case BarvinokParameters::IrrationalPrimalDecomposition:
	case BarvinokParameters::IrrationalAllPrimalDecomposition:
	  checkConeIrrational(cones1[i], Parameters->Number_of_Variables);
	  break;
	default:
	  cerr << "Unknown BarvinokParameters::decomposition";
	  abort();
	}
      }
    }
#ifdef SHOWDETS
  cerr << endl;
#endif

  int current;
  ZZ min;

  if (Parameters->Current_Simplicial_Cones_Total > Parameters->Max_Simplicial_Cones_Total)
    Parameters->Max_Simplicial_Cones_Total = Parameters->Current_Simplicial_Cones_Total;

  Parameters->Current_Depth++;
  
  do {
    min = max + 1;
    current = -1;
    for(int j = 0; j < m; j++) {
      if(Dets[j] < min && Dets[j] != 0)
	{
	  current = j;
	  min = Dets[j];
	}
    }
    if (current >= 0) {
      Dets[current] = 0; // mark done
      if(barvinok_DFS(cones1[current], Parameters) == -1)
	result = -1;
      Parameters->Current_Simplicial_Cones_Total--;
    }
  } while (current >= 0 && result == 1);
  Parameters->Current_Depth--;
  freeCone(C);
  return result;
}

/*
  The first step is to triangulate a cone into simplicial cones.
  Then, by using Barvinok's decomposition, we decompose each
  simplicial cone into unimodular cones.
*/

int
barvinokDecomposition_Single (listCone *cone,
			      Single_Cone_Parameters *Parameters)
{
  int status = 1;
  listCone *triang = triangulateCone(cone, Parameters->Number_of_Variables, Parameters);
  Parameters->decompose_time.start();
  listCone *t;
  for (t = triang; t!=NULL; t=t->rest) {
    int num_rays = lengthListVector(t->rays);
    assert(num_rays == Parameters->Number_of_Variables);
    mat_ZZ B = createConeDecMatrix(t, num_rays, Parameters->Number_of_Variables);
    if ((status = barvinok_Single(B, Parameters, t->vertex)) == -1)
      goto BAILOUT;
  }
 BAILOUT:
  Parameters->decompose_time.stop();
  freeListCone(triang);
  return status;
}

