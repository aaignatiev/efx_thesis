/* RegularTriangulationWith4ti2.cpp -- Triangulate with 4ti2

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

#include <iostream>

// From 4ti2:
#include "groebner/BitSet.h"
#include "groebner/VectorArrayStream.h"
#include "groebner/LatticeBasis.h"
#include "groebner/RayAlgorithm.h"
#include "groebner/HermiteAlgorithm.h"

// From LattE:
#include "latte_4ti2.h"
#include "dual.h"
#include "print.h"
#include "triangulation/RegularTriangulationWith4ti2.h"

using namespace _4ti2_;

listCone *
cone_from_ray_BitSet(vector<listVector *> &rays, const BitSet &ray_set,
		Vertex *vertex)
{
	listCone *c = createListCone();
	c->vertex = new Vertex(*vertex);
	vector<listVector *>::iterator i;
	int j;
	for (i = rays.begin(), j = 0; i != rays.end(); ++i, j++)
	{
		if (ray_set[j])
		{
			c->rays = new listVector((*i)->first, c->rays, (*i)->index_hint);
		}
	}
	return c;
}

static bool lindep_heights_p(listCone *cone, BarvinokParameters *Parameters,
		const vector<mpz_class> &heights, bool print = false)
{
	int num_rays = lengthListVector(cone->rays);
	/* Check for linearly dependent height vector. */
	VectorArray *ray_column_matrix = rays_to_transposed_4ti2_VectorArray(
			cone->rays, Parameters->Number_of_Variables,
			/* extra_rows: */1);
	if (print)
		cout << "Before: " << endl << *ray_column_matrix << endl;
	int rank = upper_triangle(*ray_column_matrix,
			Parameters->Number_of_Variables, num_rays);
	if (print)
		cout << "After: " << endl << *ray_column_matrix << endl;
	int i;
	for (i = 0; i < num_rays; i++)
		(*ray_column_matrix)[rank][i] = heights[i];
	if (print)
		cout << "With heights: " << endl << *ray_column_matrix << endl;
	int new_rank = upper_triangle(*ray_column_matrix, rank + 1, num_rays);
	if (print)
		cout << "Finally: " << endl << *ray_column_matrix << endl;
	delete ray_column_matrix;
	if (new_rank == rank)
		return true;
	else
		return false;
}

void triangulate_cone_with_4ti2(listCone *cone, BarvinokParameters *Parameters,
		height_function_type height_function, void *height_function_data,
		ConeConsumer &consumer)
{
	Parameters->num_triangulations++;
	// Copy rays into an array, so we can index them.
	int num_rays = lengthListVector(cone->rays);
	vector<listVector *> rays_array = ray_array(cone);
	vector<mpz_class> heights(num_rays);
	/* Compute the heights. */
	int i;
	mpq_class first_height;
	bool seen_different_heights = false;
	for (i = 0; i < num_rays; i++)
	{
		mpq_class height;
		height_function(height.get_mpq_t(), rays_array[i]->first,
				height_function_data);
		if (i == 0)
			first_height = height;
		else if (first_height != height)
			seen_different_heights = true;
		heights[i] = height.get_num();
	}

	if (!seen_different_heights)
	{
		/* This will be a trivial polyhedral subdivision, so just return
		 a copy of the cone. */
		//cerr << "Trivial test: Lifting heights yield trivial polyhedral subdivision." << endl;
		Parameters->num_triangulations_with_trivial_heights++;
		consumer.ConsumeCone(copyCone(cone));
		return;
	}

	bool lindep_height = lindep_heights_p(cone, Parameters, heights, false);
	if (lindep_height)
	{
		//cerr << "Rank test: Lifting heights yield trivial polyhedral subdivision." << endl;
		Parameters->num_triangulations_with_dependent_heights++;
#ifndef DEBUG_RANKTEST
		consumer.ConsumeCone(copyCone(cone));
		return;
#endif
	}

	/* Create a matrix from the rays, with 1 extra coordinates
	 at the front for the lifting, and also slack variables.
	 (4ti2 does not use a homogenization coordinate.) */
	int lifted_dim = Parameters->Number_of_Variables + 1 + 1 + num_rays;
	BitSet *rs = new BitSet(lifted_dim);
	VectorArray *matrix = rays_to_4ti2_VectorArray(cone->rays,
			Parameters->Number_of_Variables,
			/* num_homogenization_vars: */1 + 1 + num_rays,
			/* num_extra_rows: */1);
	/* Add identity matrix for the slack variables (including a slack
	 variable for the extra ray). */
	{
		int i;
		for (i = 0; i < num_rays + 1; i++)
		{
			(*matrix)[i][i] = 1;
			rs->set(i);
		}
	}

	int I_width = num_rays + 1;

	/* Extra row: `vertical' ray -- This kills all upper facets.
	 See Verdoolaege, Woods, Bruynooghe, Cools (2005). */
	(*matrix)[num_rays][I_width] = 1;

	/* Put in the heights. */
	for (i = 0; i < num_rays; i++)
	{
		(*matrix)[i][I_width] = heights[i];
	}

	/* Output of the file -- for debugging. */
	if (Parameters->debug_triangulation)
	{
		std::ofstream file("lifted_cone_for_4ti2_triangulation");
		file << matrix->get_number() << " " << lifted_dim << endl;
		print(file, *matrix, 0, lifted_dim);
		cerr << "Created file `lifted_cone_for_4ti2_triangulation'" << endl;
	}
#if 0
	VectorArray *facets = new VectorArray(0, matrix->get_size());
	lattice_basis(*matrix, *facets); // too slow.
#else
	int numvars_plus_1 = Parameters->Number_of_Variables + 1;
	VectorArray *facets = new VectorArray(numvars_plus_1, matrix->get_size());
	/* Our matrix is of the form (I w A), so the kernel has a basis of
	 the form ((-w -A)^T I). */
	for (i = 0; i < numvars_plus_1; i++)
	{
		int j;
		for (j = 0; j < I_width; j++)
			(*facets)[i][j] = -(*matrix)[j][I_width + i];
		(*facets)[i][I_width + i] = 1;
	}
#endif
#if 0
	cerr << "Facets: " << *facets << endl;
#endif

	VectorArray* subspace = new VectorArray(0, matrix->get_size());
	RayAlgorithm algorithm;

	//Redirect cout to cerr (when calling 4ti2's triangulation function).
	streambuf * cout_strbuf(cout.rdbuf()); //keep copy of the real cout.
	if ( Parameters->debug_triangulation == false)
	{
			cout.rdbuf(cerr.rdbuf()); //change cout to cerr
	}
	algorithm.compute(*matrix, *facets, *subspace, *rs);
	cout.rdbuf(cout_strbuf); //revert cout back to cout!

	if (Parameters->debug_triangulation)
	{
		std::ofstream file("4ti2_triangulation_output");
		file << facets->get_number() << " " << lifted_dim << "\n";
		print(file, *facets, 0, lifted_dim);
		cerr << "Created file `4ti2_triangulation_output'" << endl;
	}

	/* Walk through all facets.  (Ignore all equalities in *subspace.)
	 */
	int num_cones_created = 0;
	int num_equalities = subspace->get_number();
	int num_facets = facets->get_number();
	int true_dimension = Parameters->Number_of_Variables - num_equalities;
	BitSet incidence(num_rays);
	consumer.SetNumCones(num_facets); // estimate is enough
	for (i = 0; i < num_facets; i++)
	{
		/* We ignore facets that are incident with the extra vertical
		 ray.  */
		if ((*facets)[i][I_width] != 0)
		{
			/* All other facets give a face of the triangulation. */
			/* Find incident rays.
			 They are the rays whose corresponding slack variables are
			 zero. */
			incidence.zero();
			int j;
			for (j = 0; j < num_rays; j++)
			{
				if ((*facets)[i][j] == 0)
				{
					/* Incident! */
					incidence.set(j);
				}
			}
			/* Create the cone from the incidence information. */
			listCone *c = cone_from_ray_BitSet(rays_array, incidence,
					cone->vertex);
			/* Is a cone of the triangulation -- check it is simplicial */
			int c_num_rays = incidence.count();
			if (c_num_rays > true_dimension
					&& !Parameters->nonsimplicial_subdivision)
			{
				cerr << "Found non-simplicial cone (" << c_num_rays << "rays) "
						<< "in polyhedral subdivision, triangulating it recursively."
						<< endl;
				/* In the refinement step, always fall back to using a
				 random height vector. */
				triangulate_cone_with_4ti2(c, Parameters, random_height,
						&Parameters->triangulation_max_height, consumer);
			} else if (c_num_rays < true_dimension)
			{
				cerr
						<< "Lower-dimensional cone in polyhedral subdivision, should not happen."
						<< endl;
				abort();
			} else
			{
				consumer.ConsumeCone(c);
				num_cones_created++;
			}
		}
	}

#ifdef DEBUG_RANKTEST
	if (lindep_height && num_cones_created != 1)
	{
		cerr << "Created non-trivial subdivision, though heights were dependent?!" << endl;
		cerr << "Matrix: " << endl << *matrix << endl;
		lindep_heights_p(cone, Parameters, heights, true);
		abort();
	}
#endif

	delete facets;
	delete subspace;
	delete matrix;
	delete rs;
}

void random_regular_triangulation_with_4ti2(listCone *cone,
		BarvinokParameters *Parameters, ConeConsumer &consumer)
{
	if (Parameters->triangulation_prescribed_height_data != NULL)
	{
		triangulate_cone_with_4ti2(cone, Parameters, prescribed_height,
				Parameters->triangulation_prescribed_height_data, consumer);
	} else if (Parameters->triangulation_bias >= 0)
	{
		triangulate_cone_with_4ti2(cone, Parameters, biased_random_height,
				&Parameters->triangulation_bias, consumer);
	} else
	{
		triangulate_cone_with_4ti2(cone, Parameters, random_height,
				&Parameters->triangulation_max_height, consumer);
	}
}

void refined_delone_triangulation_with_4ti2(listCone *cone,
		BarvinokParameters *Parameters, ConeConsumer &consumer)
{
	triangulate_cone_with_4ti2(cone, Parameters, delone_height, NULL, consumer);
}
