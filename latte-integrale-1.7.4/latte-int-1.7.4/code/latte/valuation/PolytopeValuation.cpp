/*
 * PolytopeValuation.cpp
 *
 *  Created on: Jun 25, 2010
 *      Author: Brandon Dutra and Gregory Pinto
 */

#include "PolytopeValuation.h"
#include "Perturbation.h"

using namespace std;

/**
 * If the polytpe is non-homogenized, (we have vertex-rays) we save the
 *   polytope in vertexRayCones; otherwise we have a lifted polytope-cone
 *   and we save it in polytopeAsOneCone.
 *
 */
PolytopeValuation::PolytopeValuation(Polyhedron *p, BarvinokParameters &bp) :
	parameters(bp), poly(p), vertexRayCones(NULL), polytopeAsOneCone(NULL),
			triangulatedPoly(NULL), freeVertexRayCones(0),
			freePolytopeAsOneCone(0), freeTriangulatedPoly(0),
			latticeInverse(NULL), latticeInverseDilation(NULL)

{
	numOfVars = parameters.Number_of_Variables; //keep number of origional variables.
	dimension = numOfVars;

	if (p->unbounded)
	{
		cout << "Cannot compute valuation for unbounded polyhedron."
				<< endl;
		exit(1);
	}//check the polytope is bounded.


	if (p->homogenized == false)
	{
		vertexRayCones = p->cones; //Polyhedron is a list of vertex-ray cones.
		numOfVarsOneCone = numOfVars+1;
	}//we are given vertex-tangent-cone information.
	else
	{
		polytopeAsOneCone = p->cones;//the polytope is 1 cone.
		//the rays of the cone are in the form [vertex of p, 1].
		numOfVarsOneCone = numOfVars;
	}//else the polyhedron is given by one cone, where the rays are the vertices
	//with a tailing 1 in each vertex.

	//cout << "PolytopeValuation::num of var one cone=" << numOfVarsOneCone << endl;
	//cout << "PolytopeValuation::numovvars" << numOfVars << endl;
	//cout << "constructor start" << endl;
	//printListCone(vertexRayCones, numOfVars);
	//cout << "constructor end of vertex-ray, start of lifted cone" << endl;
	//printListCone(polytopeAsOneCone, numOfVarsOneCone);
	//cout << "constructor end" << endl;

	srand(time(0));
}//constructor


/**
 * Deletes triangulated polytope/cones if this class constructed them in the first place.
 */
PolytopeValuation::~PolytopeValuation()
{
	//don't free vertexRayCones, because we did not make them!
	if (polytopeAsOneCone && freePolytopeAsOneCone)
		freeListCone(polytopeAsOneCone);
	if (triangulatedPoly && freeTriangulatedPoly)
		freeListCone(triangulatedPoly);
}

/**
 *  Takes the vertex-ray representation of the polytope, extracts the vertex information,
 *  then then creates one cone with the vertex at the origin and who's rays are integer
 *  multiple of the vertex with a leading 1. Finally,
 *
 *  Example: if the polytope has vertex { (3, 3/4), (5, 1/2), (1/2, 1/2)} then the new cone
 *  will have vertex (0 0 0) and integer rays
 *  (3, 3/4, 1)*4, (5, 1/2, 1)*2, (1/2, 1/2, 1)*2
 *
 *
 *
 */
void PolytopeValuation::convertToOneCone()
{
	if (polytopeAsOneCone)
		return; //already did computation, or we where given a homo. polytope in the constructor.
	if (triangulatedPoly)
		return; //don't care about converting to one cone for triangulation because already triangulated!
	if (!vertexRayCones)
	{
		cout << "PolytopeValuation::convertToOneCone vertexRayCones* is not defined" << endl;
		exit(1);
	}//error.

	assert(numOfVars +1 == numOfVarsOneCone);

	listCone * oneCone = new listCone();
	oneCone->coefficient = 1;
	oneCone->determinant = 0;
	oneCone->subspace_generators = NULL;
	oneCone->dual_determinant = 0;
	oneCone->facets = NULL;
	oneCone->equalities = NULL;
	oneCone->latticePoints = NULL;
	oneCone->rest = NULL;

	//set to zero vector of numofvars + 1 size.
	oneCone->vertex = new Vertex();
	oneCone->vertex->vertex = new rationalVector(numOfVars + 1);

	//oneCone->rays = new listVector;
	//oneCone->rays->rest = 0;

	//now add the vertex to the rays list with a ending 1: (old poly cone vertex,1).
	//The first entry in masterList should be ignored because masterList->first = masterList->rest->first.
	listVector * masterList = new listVector;

	for (listCone * currentCone = vertexRayCones; currentCone; currentCone
			= currentCone->rest)
	{
		vec_ZZ buildRay; //buildRay = [old-vertex,1]
		ZZ nume, denom;
		buildRay.SetLength(numOfVars + 1);

		ZZ scaleFactor; //scaleRationalVectorToInteger() sets scaleFactor.
		vec_ZZ integerVertex = scaleRationalVectorToInteger(
				currentCone->vertex->vertex, numOfVars, scaleFactor);

		buildRay[numOfVars] = scaleFactor; // = 1 * scaleFactor.
		for (int i = 0; i < numOfVars; ++i)
		{
			buildRay[i] = integerVertex[i];//copy the old vertex over.
		}//for i

		//cout << buildRay << endl;

		masterList->first = buildRay;
		masterList = appendVectorToListVector(buildRay, masterList);
	}//for currentCone

	//cout << "END  BUILDING THE RAYS" << endl;

	oneCone->rest = 0;
	oneCone->rays = masterList->rest; //ignore masterList->first, so just return the rest and NOT masterList.
	masterList->rest = 0;
	freeListVector(masterList); //delete the dummy head only.

	polytopeAsOneCone = oneCone; //finally, save the result.
	freePolytopeAsOneCone = 1; //delete this object in the deconstructor.
}//convertToOneCone

/**
 * Dilates the polytope by computing new_vertex = old_vertex * factor, and overriding the
 * polytopeAsOneCone
 *
 * The original polytope is NOT lost (you can recover it by dividing by the last element in each ray)
 */
void PolytopeValuation::dilatePolytopeOneCone(const ZZ & factor)
{
	//each ray is in the form [v1, .., v2, a], where a could or could not be one.
	//we will mult the ray vector by factor/a. If factor is the lcm of all the a's, then
	//the new vertex is [integer, 1].
	for(listVector * v = polytopeAsOneCone->rays; v; v = v->rest)
	{
		//long divide(ZZ& q, const ZZ& a, const ZZ& b);
		// if b | a, sets q = a/b and returns 1; otherwise returns 0.
		ZZ newFactor;
		assert(divide(newFactor, factor, v->first[numOfVarsOneCone-1]));
		v->first *= newFactor; //scalor-vector mult.
		v->first[numOfVarsOneCone-1] = factor; //here, we lose the org. polytope.
	}//for eacy vertex saved in the ray.
}

/**
 * Dilates the polytope by computing new_vertex = old_vertex * factor, and overriding the
 * vertex-ray cones.
 *
 * The original polytope is lost.
 */
void PolytopeValuation::dilatePolytopeVertexRays(const RationalNTL & factor)
{
	for (listCone * cone = vertexRayCones; cone; cone = cone->rest)
	{
		cone->vertex->vertex->scalarMultiplication(factor.getNumerator(),
				factor.getDenominator());
	}//for every vertex.
}//dilatePolytope




/*
 * Dilates the polynomial by setting x_i -->dilationFactor * x_i
 * Then converts this new polynomial to linear forms.
 * If the input polynomial has a constant term, it is returned (constant terms are not converted to linear forms)
 */
void PolytopeValuation::dilatePolynomialToLinearForms(linFormSum &linearForms, const monomialSum& originalPolynomial, const ZZ &dilationFactor, RationalNTL & constantMonomial)
{
	constantMonomial = 0; //assume the originalPolynomial is homogeneous, we will update this if needed.
	int numberConstantTerms = 0; //used for error checking.

	//Goal: find new polynomial.
	//define the new polynomial.
	monomialSum transformedPolynomial; //updated coefficients.
	transformedPolynomial.termCount = 0;
	transformedPolynomial.varCount = originalPolynomial.varCount;

	//make a loader for the transformed polynomial and make an iterator for the original polynomial.
	MonomialLoadConsumer<RationalNTL>* transformedPolynomialLoader =
			new MonomialLoadConsumer<RationalNTL> ();
	transformedPolynomialLoader->setMonomialSum(transformedPolynomial);
	BTrieIterator<RationalNTL, int>* originalPolynomialIterator =
			new BTrieIterator<RationalNTL, int> ();
	originalPolynomialIterator->setTrie(originalPolynomial.myMonomials,
			originalPolynomial.varCount);
	originalPolynomialIterator->begin();

	term<RationalNTL, int>* originalMonomial;
	RationalNTL coefficient, totalDegree;
	//loop over the original polynomial, and insert the updated monomial into the transformedPolynomial
	for (originalMonomial = originalPolynomialIterator->nextTerm(); originalMonomial; originalMonomial
			= originalPolynomialIterator->nextTerm())
	{
		long totalDegree = 0;
		coefficient = originalMonomial->coef;

		//find the total degree of the monomial.
		for (int currentPower = 0; currentPower < originalMonomial->length; ++currentPower)
			totalDegree += originalMonomial->exps[currentPower];
		//cout << "factor^degree = " << dilationFactor << "^" << originalMonomial->degree << endl;
		//cout << "length = " << originalMonomial->length << endl;
		coefficient.div(power(dilationFactor, totalDegree));

		if (totalDegree == 0)
		{
			constantMonomial += coefficient;
			++numberConstantTerms; //count the number of constant terms we have. This should be one.
		}
		else
		{
			transformedPolynomialLoader->ConsumeMonomial(coefficient,
					originalMonomial->exps);
		}//insert the non-constant monomial
	}//for every term in the originalPolynomial

	assert(originalPolynomial.termCount == transformedPolynomial.termCount + numberConstantTerms
			&& originalPolynomial.varCount == transformedPolynomial.varCount );
	assert(numberConstantTerms <= 1);


	//make an iterator for the transformed polynomial and decompose it into linear forms.
	BTrieIterator<RationalNTL, int>* transformedPolynomialIterator =
			new BTrieIterator<RationalNTL, int> ();
	linearForms.termCount = 0;
	linearForms.varCount = transformedPolynomial.varCount;

	//at this point, transformedPolynomial could be empty if originalPolynomial only contained a constant term.
	if ( transformedPolynomial.termCount == 0)
	{
		; //do nothing, we are already returning the constant via the constantMonomial parameter.
	}//if originalPolynomial only had a constant term
	else
	{
		transformedPolynomialIterator->setTrie(transformedPolynomial.myMonomials,
				transformedPolynomial.varCount);
		decompose(transformedPolynomialIterator, linearForms);
		destroyMonomials(transformedPolynomial);
	}//else, decompost it to linear forms.


	delete transformedPolynomialLoader;
	delete originalPolynomialIterator;
	delete transformedPolynomialIterator;
}


/**
 *
 */
void PolytopeValuation::dilateLinearForms(linFormSum &linearForms, const linFormSum & originalLinearForms, const ZZ & dilationFactor, RationalNTL &constantMonomial)
{

	BTrieIterator<RationalNTL, ZZ>* linearFormsItr =
			new BTrieIterator<RationalNTL, ZZ> ();
	linearFormsItr->setTrie(originalLinearForms.myForms, originalLinearForms.varCount);
	linearFormsItr->begin();



	FormLoadConsumer<RationalNTL>* transformedFormLoader =
				new FormLoadConsumer<RationalNTL> ();
	transformedFormLoader->setFormSum(linearForms);
	transformedFormLoader->setDimension(originalLinearForms.varCount);

	term<RationalNTL, ZZ>* lform;
	vec_ZZ exp;
	exp.SetLength(originalLinearForms.varCount);

	//loop over the original polynomial, and insert the updated monomial into the transformedPolynomial
	for (lform = linearFormsItr->nextTerm(); lform; lform
			= linearFormsItr->nextTerm())
	{

		//for(int i = 0; i < lform->length; ++i)
		//	cout << "i:exp=" << i << "], " << lform->exps[i] << endl;
		//cout << "degree=" << lform->degree << endl;
		//cout << "coef  =" << lform->coef << endl;

		if (lform->degree != 0)
		{
			(lform->coef).div(power(dilationFactor, lform->degree));

			for(int i = 0; i < exp.length(); ++i)
				exp[i] = lform->exps[i]; //convert ZZ* to vec_ZZ.

			transformedFormLoader->ConsumeLinForm(lform->coef, lform->degree, exp);
		}
		else
		{
			constantMonomial += lform->coef;
		}
	}//for every term in the originalPolynomial

	delete linearFormsItr;
	delete transformedFormLoader;

}//dilateLinearForms


/**
 * Computes n!, n >= 0.
 */
ZZ PolytopeValuation::factorial(const int n)
{
	ZZ product;
	product = 1;
	for (int i = n; i > 1; --i)
		product *= i;
	return product;
}//factorial


ZZ PolytopeValuation::findDilationFactorOneCone() const
{
	ZZ dilationFactor;
	dilationFactor = 1;

	assert(polytopeAsOneCone);

	for(listVector * v = polytopeAsOneCone->rays; v; v = v->rest)
	{
		//a ray is in the form [v1, .., vn a], where a = lcm(the org. vertices), so just find the
		//lcm of all the a's.
		dilationFactor = lcm(dilationFactor, v->first[numOfVarsOneCone-1]);
	}

	return dilationFactor;
}


ZZ PolytopeValuation::findDilationFactorVertexRays() const
{
	ZZ dilationFactor;
	dilationFactor = 1;

	assert(vertexRayCones);

	//loop over every vertex in the vertexRay cones.
	for (listCone * currentCone = vertexRayCones; currentCone; currentCone
			= currentCone->rest)
		for (int i = 0; i < numOfVars; ++i)
			dilationFactor = lcm(dilationFactor,
					(currentCone->vertex->vertex->denominators())[i]);

	return dilationFactor;
}


/**
 * Integrates the polynomial over the polytope.
 *
 * In the non-integer case, we compute
 *  1/a^n * \sum_{i}{coefficient_i * integral( m_i(x1/a, x2/a, ..., xn/a) dX }
 *  Where m_i is a monomial with a coefficient of 1,
 *  and where 1/(a*a*...*a) is the Jacobian of the change of variables to make the polytope have integer vertices.
 *
 *  We use the fact that if P is a polytope with rational vertices, then
 *
 *  integral over P { f(w1, w2, ..., wn) dw } = integral over P' { f(x1/a, x2/a, ..., xn/a)  * |d(w1, .., wn)/d(x1, .., xn)| dX }
 *  where |d(w1, .., wn)/d(x1, .., xn)| is the absolute value of the Jacobian given by the equations xi = wi * a
 *  and a is such that ever vertex becomes integer when mult. by a.
 *  P' is now a dilation of P such that P' has only integer vertices.
 */


RationalNTL PolytopeValuation::findIntegral(const monomialSum& polynomial, const ValuationAlgorithm algorithm)
{

	if ( algorithm == integratePolynomialAsLinearFormCone || algorithm == integratePolynomialAsLinearFormTriangulation)
		return findIntegralPolynomialToLinearForms(polynomial, algorithm);
	else if ( algorithm == integratePolynomialAsPLFTriangulation)
		return findIntegralPolynomialToPLF(polynomial);
	else
		THROW_LATTE(LattException::ie_UnexpectedIntegrationOption);


}

/**
 * Save as the other findIntegral, only this one starts with linear forms, not polynomials.
 */
RationalNTL PolytopeValuation::findIntegral(const linFormSum& originalLinearForms, const ValuationAlgorithm algorithm )
{
	linFormSum linearForms;
	RationalNTL answer;
	RationalNTL constantMonomial;


	//find the dilation factor.
	ZZ dilationFactor;

	//cout << "Integrating " << polynomial.termCount << " monomials." << endl;
	//dilate the polytope
	if ( numOfVars != numOfVarsOneCone)
	{
		dilationFactor = findDilationFactorVertexRays();
		cerr << "dilation factor = " << dilationFactor << endl;
		dilatePolytopeVertexRays(RationalNTL(dilationFactor, to_ZZ(1)));
	}//if we started with vertex-rays
	else
	{
		dilationFactor = findDilationFactorOneCone();
		cerr << "dilation factor = " << dilationFactor << endl;
		dilatePolytopeOneCone(dilationFactor);
	}//we started with the lifted polytope.


	//the input polynomial have have a constant term, but the integration functions can only
	//work with non-constant monomials. We need to remove any constant terms from the input polynomial.
	//This will be done in dilatePolynomialToLinearForms


	//dilate the polynomial..
    //after this call, linearForms is filled in, and constantMonomial is the constant term in the input polynomial.
	dilateLinearForms(linearForms, originalLinearForms, dilationFactor, constantMonomial);


	//Note the difference between lawrence and integration on how we compute the volume
	//if dilatePolytopeVertexRays was used: 1) integrate 2) volume 3) jacobian term
	//if dilatePolytopeOneCone    was used: 1) integrate 2) jacobian term 3)volume
	//This is due to the fact that dilatePolytopeVertexRays permanently dilates the polytope.
	//But dilatePolytopeOneCone records the dilation in the last slot, and so nothing is lost.
	if ( algorithm == integrateLinearFormCone)
	{
		triangulatePolytopeVertexRayCone(); //triangulate the vertex ray cones
		cerr << lengthListCone(triangulatedPoly) << " triangulations done.\n"
			 << " starting to integrate " << linearForms.termCount << " linear forms.\n";
		answer.add(findIntegralUsingLawrence(linearForms)); //finally, we are ready to do the integration!

		if ( constantMonomial != 0)
			answer.add(findVolume(volumeCone)*constantMonomial);

		answer.div(power(dilationFactor, linearForms.varCount)); //factor in the Jacobian term.
	}// if computing the integral using the lawrence style method.
	else if ( algorithm == integrateLinearFormTriangulation)
	{
		convertToOneCone(); //every vertex should be integer
		triangulatePolytopeCone(); //every tiangulated vertex is now in the form (1, a1, ..., an) such that ai \in Z.
		cerr << " starting to integrate " << linearForms.termCount << " linear forms.\n";

		answer.add(findIntegralUsingTriangulation(linearForms)); //finally, we are ready to do the integration!

		answer.div(power(dilationFactor, linearForms.varCount)); //factor in the Jacobian term.


		if ( constantMonomial != 0)
		{
			RationalNTL volume;
			volume = findVolume(volumeTriangulation)*constantMonomial;
			if (numOfVars != numOfVarsOneCone)
				volume.div(power(dilationFactor, linearForms.varCount)); //factor in the Jacobian term.
			answer.add(volume);
		}

	}//if computing the integral by triangulating to simplex polytopes.
	else
	{
		cerr << "Integration Type not known" << endl;
		THROW_LATTE(LattException::ie_UnexpectedIntegrationOption);
	}//else error.


	destroyLinForms(linearForms);
	return answer;

}//findIntegral


/**
 * Integrates the sum of products of powers of linear forms.
 * Example: 3 * (x +y +0z)^3 (-x +5y +z)^10 (x + 0y + 0z)^1
 *
 * The input polytope is dilated to have integer vertices.
 *
 * The integral is computed by triangulating the polytope and then
 *
 */
RationalNTL PolytopeValuation::findIntegral(const linFormProductSum& originalLinearFormProducts, ValuationAlgorithm algorithm)
{
	linFormProductSum linearFormProducts;
	RationalNTL answer;
	RationalNTL constantMonomial;

	linearFormProducts.varCount = originalLinearFormProducts.varCount;

	//find the dilation factor.
	ZZ dilationFactor;

	cout << "Integrating " << originalLinearFormProducts.myFormProducts.size() << " products of powers of linear forms." << endl;
	//dilate the polytope
	if ( numOfVars != numOfVarsOneCone)
	{
		dilationFactor = findDilationFactorVertexRays();
		cerr << "dilation factor = " << dilationFactor << endl;
		dilatePolytopeVertexRays(RationalNTL(dilationFactor, to_ZZ(1)));
	}//if we started with vertex-rays
	else
	{
		dilationFactor = findDilationFactorOneCone();
		cerr << "dilation factor = " << dilationFactor << endl;
		dilatePolytopeOneCone(dilationFactor);
	}//we started with the lifted polytope.


	//dilate the integrand..
    //after this call, linearForms is filled in, and constantMonomial is the constant term in the input polynomial.
	for(int i = 0; i < originalLinearFormProducts.myFormProducts.size(); ++i)
	{
		linFormSum linearForms;
		RationalNTL currentConstantMonomial;
		dilateLinearForms(linearForms, originalLinearFormProducts[i], dilationFactor, currentConstantMonomial);

		if ( linearForms.termCount > 0 && currentConstantMonomial != 0)
		{
			cout << "lf: " << printLinForms(originalLinearFormProducts[i]).c_str() << endl;
			THROW_LATTE_MSG(LattException::ie_BadIntegrandFormat, 1, "product of linear forms contains a constant factor");
		}

		if ( currentConstantMonomial != 0)
			constantMonomial += currentConstantMonomial;
		else
		{
			linearFormProducts.addProduct(linearForms);
		}//add the new product of linear forms.
	}


	convertToOneCone(); //every vertex should be integer
	triangulatePolytopeCone(); //every tiangulated vertex is now in the form (1, a1, ..., an) such that ai \in Z.
	cerr << " starting to integrate " << linearFormProducts.myFormProducts.size() << " product of linear forms.\n";

	for(int i = 0; i < linearFormProducts.myFormProducts.size(); ++i)
		answer.add(findIntegralProductsUsingTriangulation(linearFormProducts.myFormProducts[i])); //finally, we are ready to do the integration!

	answer.div(power(dilationFactor, linearFormProducts.varCount)); //factor in the Jacobian term.

	if ( constantMonomial != 0)
	{
		RationalNTL volume;
		volume = findVolume(volumeTriangulation)*constantMonomial;
		if (numOfVars != numOfVarsOneCone)
			volume.div(power(dilationFactor, linearFormProducts.varCount)); //factor in the Jacobian term.
		answer.add(volume);
	}


	destroyLinFormProducts(linearFormProducts);
	return answer;
}//findIntegral

/**
 * Currently, this is only used by the stokes formula. This is NOT done.
 */
RationalNTL PolytopeValuation::findIntegral(linFormSum& linearForms)
{
	RationalNTL answer;
	RationalNTL constantForm;
	//linFormSum linearForms;

	cout << "find int with linear forms called" << endl;
	//linFormSum linearForms;


	//find the dilation factor.
	ZZ dilationFactor;

	cout << "Integrating " << linearForms.termCount << " powers of linear forms." << endl;
	//dilate the polytope
	if ( numOfVars != numOfVarsOneCone)
	{
		dilationFactor = findDilationFactorVertexRays();
		if ( dilationFactor != 1)
		{
			cout << "dilation factor = " << dilationFactor << endl;
			cout << "sorry, cannot dilate polytope in stokes because cannot integrate rational linear forms" << endl;
			exit(1);
		}
	}//if we started with vertex-rays
	else
	{
		cout << "Sorry, PolytopeValuation::findIntegral(const linFormSum& linForm) only works for lawrence type methods." << endl;
		exit(1);
	}//we started with the lifted polytope.


	//we only have 1 method lawrence/stokes
	//assume linear forms is homogenous (we cannot integrate a constant in this function currently).
	triangulatePolytopeVertexRayCone(); //triangulate the vertex ray cones

	//cout << "find integral stokes" << endl;
	//printListCone(triangulatedPoly, numOfVars);
	//cout << "end finnd integral stokes" << endl;

	cout << lengthListCone(triangulatedPoly) << " triangulations done.\n"
		 << " starting to integrate " << linearForms.termCount << " linear forms.\n";
	cout << "triangulated cones";
	printListCone(triangulatedPoly, numOfVars);
	answer.add(findIntegralUsingLawrence(linearForms)); //finally, we are ready to do the integration!

	//answer.div(power(dilationFactor, polynomial.varCount)); //factor in the Jacobian term.


	destroyLinForms(linearForms);
	return answer;

}




/* computes the integral of a polytope by summing the integral of over each simplex
 * in the triangulation of the polytope.
 * @Assumes: polytope has integer vertices and it is triangulated.
 * @input: linear forms after the polynomial and polytope has been dilated
 * @return RationalNTL: the integral of the polytope over every linear form.
 *
 * Math: For each simplex, we sum the fractions
 *
 * 	<vi, l>^d+M * abs(det(matrix formed by the rays))* M!/(d+M)!
 *  --------------------------------------------------------------
 *                <r_1, -l> * <r_2, -l>*...*<r_d, -l>
 *
 * where v is a vertex,
 *       l is the linear form
 *       d is the dimension
 *       M is the power of the linear form
 *       r_i is the ith ray of the current simplex.
 *
 * If we divide by zero, we let K be an index set for r_i s.t. {<r_i, l>} contains only unique terms, and then we find
 *Residue 	(e + <v, l>)^d+M * abs(det(matrix formed by the rays))* M!/(d+M)!
 *           --------------------------------------------------------------
 *                e^(m0) \prod_{k \in K}(e + <-r_k, l>)^mk
 * Where mk = #{ i \in {1, . . . , d + 1} : vi = vk } where v_i is the simplex vertex.
 *
 * See the paper: "How to Integrate a Polynomial over a Simplex" by  V. BALDONI, N. BERLINE, J. A. DE LOERA, M. VERGNE.
 */
RationalNTL PolytopeValuation::findIntegralUsingTriangulation(linFormSum &forms) const
{
	RationalNTL answer;
	int simplicesFinished = 0;
	int totalSimplicesToIntegrate = lengthListCone(triangulatedPoly);

	if ( forms.termCount == 0)
		return RationalNTL(); //nothing to do.

	//set up itrator to loop over the linear forms.
	BTrieIterator<RationalNTL, ZZ>* linearFormIterator = new BTrieIterator<
			RationalNTL, ZZ> ();
	linearFormIterator->setTrie(forms.myForms, forms.varCount);




	for (listCone * currentCone = triangulatedPoly; currentCone; currentCone
			= currentCone->rest)
	{
		//First construct a simplex.
		simplexZZ oneSimplex;
		oneSimplex.d = forms.varCount;



		int vertexCount = 0; //the current vertex number being processed.
		oneSimplex.s.SetLength(numOfVarsOneCone);

		for (listVector * rays = currentCone->rays; rays; rays = rays->rest, ++vertexCount)
		{
			oneSimplex.s[vertexCount].SetLength(numOfVarsOneCone-1);
//			assert( rays->first[numOfVarsOneCone-1] == 1); //make sure the triangulation is such that the vertices of the original polytope is integer.

			for (int k = 0; k < numOfVarsOneCone -1; ++k)
				oneSimplex.s[vertexCount][k] = rays->first[k];//save the vertex.

		}//create the simplex. Don't copy the leading 1.

		//compute the volume of the Parallelepiped
		mat_ZZ matt;
		matt.SetDims(oneSimplex.d, oneSimplex.d);
		for (int j = 1; j <= oneSimplex.d; j++)
			matt[j - 1] = oneSimplex.s[j] - oneSimplex.s[0];
		oneSimplex.v = abs(determinant(matt));

		ZZ numerator, denominator;

		//Finally, we are ready to integrate the linear form over the simplex!
		integrateLinFormSum(numerator, denominator, linearFormIterator,
				oneSimplex);

		answer.add(numerator, denominator);
		++simplicesFinished;
		if ( simplicesFinished % 1000 == 0)
			cerr << "Finished integrating " << simplicesFinished << "/" << totalSimplicesToIntegrate << " over " << forms.termCount << " linear forms\n";
		//answer.add(numerator * coefficient.getNumerator(), denominator * coefficient.getDenominator());
	}//for every triangulated simplex.
	delete linearFormIterator;

	return answer;
}//integratePolytopeTriangulation()

/* computes the integral of a polytope using the lawrence type formula
 * We do not consider the case of unimodular cones (but it would be easy to add this in).
 * @Assumes: polytope has integer vertices and it's cones have been triangulated/are simple.
 * @input: linear forms after the polynomial and polytope has been been dilated
 * @return RationalNTL: the integral of the polytope over every linear form.
 *
 * Math: For each simple vertex-ray cone, we sum the fractions
 *
 * 	<v, l>^d+M * abs(det(matrix formed by the rays)) * M!/(d+M)!
 *  --------------------------------------------------------------
 *                <r_1, -l> * <r_2, -l>*...*<r_d, -l>
 *
 * where v is a vertex,
 *       l is the linear form
 *       d is the dimension
 *       M is the power of the linear form
 *       r_i is the ith ray of the cone.
 *
 * If we divide by zero, we then take a linear perturbation l:=l+e,
 * where e = (a1 * epsilon, .... an *epsilon) and find the constant term in the series expansion of epsilon.
 */
RationalNTL PolytopeValuation::findIntegralUsingLawrence(linFormSum &forms) const
{
	RationalNTL ans;
	int linearFormesFinished = 0;

	if(forms.termCount == 0)
		return RationalNTL(); //nothing to do.

	BTrieIterator<RationalNTL, ZZ>* linearFormIterator = new BTrieIterator<
			RationalNTL, ZZ> ();
	linearFormIterator->setTrie(forms.myForms, forms.varCount); //make iterators to loop over the lin. forms.

	RationalNTL coe;
	int j, m;
	unsigned int i;
	vec_ZZ l;
	ZZ de, numerator, denominator;
	int dim = dimension; //numOfVars;
	int numberFinished = 0;

	l.SetLength(numOfVars);
	numerator = 0;
	denominator = 0;
	linearFormIterator->begin();
	term<RationalNTL, ZZ>* temp;


	//The LinearPerturbationContainer is in charge of finding a perturbation if we divide by zero.
	//It also is in charge of integrating the linear form or calling friend functions to do this.
	LinearPerturbationContainer lpc;
	lpc.setListCones(numOfVars, triangulatedPoly);
	lpc.setLatticeInformation(latticeInverse, latticeInverseDilation);
	//cout << "lpc got past construction" << endl;


	while ((temp = linearFormIterator->nextTerm()) != 0)
	{

		//get the linear form's power and terms.
		coe = temp->coef;
		m = temp->degree; //obtain coefficient, power
		l.SetLength(temp->length); //obtain exponent vector
		for (j = 0; j < temp->length; j++)
		{
			l[j] = temp->exps[j];
		}

		//find a perturbation for l
		lpc.findPerturbation(l);
		//then integrate it!!! Note that if in the future you wanted to integrate the same
		//linear form with different powers, you would have to reset the powers in the
		// linearPerturbation data structure.
		RationalNTL integralAns = lpc.integratePolytope(m);
		//cout << "int ans after lpc " << integralAns << endl;

		de = 1;
		for (i = 1; i <= dim + m; i++)
		{
			de = de * i;
		} //de is (d+m)!. Note this is different from the factor in the paper because in the storage of a linear form, any coefficient is automatically adjusted by m!
		//cout << "times coe" << coe << ", div de=" << de << ", dim=" << dim << ", m=" << m << endl;
		integralAns.mult(coe);
		integralAns.div(de);
		//cout << "int ans after mul div" << integralAns << endl;

		ans += integralAns;

		++linearFormesFinished;
		if ( linearFormesFinished % 10000 == 0)
			cerr << "Finished integrating " << linearFormesFinished << "/" << forms.termCount << " linear forms\n";
	}//while there are more linear forms to integrate

	return ans;

}//integratePolytopeLawrence()



//main driver for integrating polynomial via decomposing into FL.
RationalNTL PolytopeValuation::findIntegralPolynomialToLinearForms(const monomialSum& polynomial, const ValuationAlgorithm algorithm)
{

	linFormSum linearForms;
	RationalNTL answer;
	RationalNTL constantMonomial;

	//find the dilation factor.
	ZZ dilationFactor;

	//cout << "Integrating " << polynomial.termCount << " monomials." << endl;
	//dilate the polytope
	if ( numOfVars != numOfVarsOneCone)
	{
		dilationFactor = findDilationFactorVertexRays();
		cerr << "dilation factor = " << dilationFactor << endl;
		dilatePolytopeVertexRays(RationalNTL(dilationFactor, to_ZZ(1)));
	}//if we started with vertex-rays
	else
	{
		dilationFactor = findDilationFactorOneCone();
		cerr << "dilation factor = " << dilationFactor << endl;
		dilatePolytopeOneCone(dilationFactor);
	}//we started with the lifted polytope.


	//the input polynomial have have a constant term, but the integration functions can only
	//work with non-constant monomials. We need to remove any constant terms from the input polynomial.
	//This will be done in dilatePolynomialToLinearForms


	//dilate the polynomial..
    //after this call, linearForms is filled in, and constantMonomial is the constant term in the input polynomial.
	dilatePolynomialToLinearForms(linearForms, polynomial, dilationFactor, constantMonomial);

	//Note the difference between lawrence and integration on how we compute the volume
	//if dilatePolytopeVertexRays was used: 1) integrate 2) volume 3) jacobian term
	//if dilatePolytopeOneCone    was used: 1) integrate 2) jacobian term 3)volume
	//This is due to the fact that dilatePolytopeVertexRays permanently dilates the polytope.
	//But dilatePolytopeOneCone records the dilation in the last slot, and so nothing is lost.
	if ( algorithm == integratePolynomialAsLinearFormCone)
	{
		triangulatePolytopeVertexRayCone(); //triangulate the vertex ray cones
		cerr << lengthListCone(triangulatedPoly) << " triangulations done.\n"
			 << " starting to integrate " << linearForms.termCount << " linear forms.\n";
		answer.add(findIntegralUsingLawrence(linearForms)); //finally, we are ready to do the integration!

		if ( constantMonomial != 0)
			answer.add(findVolume(volumeCone)*constantMonomial);

		answer.div(power(dilationFactor, polynomial.varCount)); //factor in the Jacobian term.
	}// if computing the integral using the lawrence style method.
	else if ( algorithm == integratePolynomialAsLinearFormTriangulation)
	{
		convertToOneCone(); //every vertex should be integer
		triangulatePolytopeCone(); //every tiangulated vertex is now in the form (1, a1, ..., an) such that ai \in Z.
		cerr << " starting to integrate " << linearForms.termCount << " linear forms.\n";

		answer.add(findIntegralUsingTriangulation(linearForms)); //finally, we are ready to do the integration!

		answer.div(power(dilationFactor, polynomial.varCount)); //factor in the Jacobian term.


		if ( constantMonomial != 0)
		{
			RationalNTL volume;
			volume = findVolume(volumeTriangulation)*constantMonomial;
			if (numOfVars != numOfVarsOneCone)
				volume.div(power(dilationFactor, polynomial.varCount)); //factor in the Jacobian term.
			answer.add(volume);
		}

	}//if computing the integral by triangulating to simplex polytopes.
	else
	{
		cerr << "Integration Type not known" << endl;
		THROW_LATTE(LattException::ie_UnexpectedIntegrationOption);
	}//else error.


	destroyLinForms(linearForms);
	return answer;
}//findIntegralPolynomialToLinearForms


//main driver to integrating polynomial by writing it as a product of linear forms.
RationalNTL PolytopeValuation::findIntegralPolynomialToPLF(const monomialSum& polynomial)
{
	//goal: convert the polynomail to PLF (w/o dilation) then call the correct findIntegral function.
	linFormProductSum plf;

	FormProductLoadConsumer<RationalNTL>* consumer =
			new FormProductLoadConsumer<RationalNTL> ();
	consumer->setFormProductSum(plf);

	consumer->setDimension(polynomial.varCount);



	BTrieIterator<RationalNTL, int>* polynomialIterator =
			new BTrieIterator<RationalNTL, int> ();
	polynomialIterator->setTrie(polynomial.myMonomials,	polynomial.varCount);
	polynomialIterator->begin();

	term<RationalNTL, int>* oneMonomial;

	//loop over the polynomial, and insert the plf
	vec_ZZ lForm;
	lForm.SetLength(polynomial.varCount);
	for(int i = 0; i < lForm.length(); ++i)
		lForm[i] = 0;

	vec_ZZ onesVector;
	onesVector.SetLength(polynomial.varCount);
	for(int i = 0; i < onesVector.length(); ++i)
		onesVector[i] = 1;


	int numTermsAdded;
	for (oneMonomial = polynomialIterator->nextTerm(); oneMonomial; oneMonomial
			= polynomialIterator->nextTerm())
	{
		int productIndex = consumer->initializeNewProduct();
		numTermsAdded = 0;

		for(int i = 0; i < polynomial.varCount; ++i)
		{

			if (oneMonomial->exps[i] == 0)
				continue;


			lForm[i] = 1;

			consumer->ConsumeLinForm(productIndex, oneMonomial->coef, oneMonomial->exps[i], lForm);
			oneMonomial->coef = 1; //add a real coefficient the first time only.

			++numTermsAdded;
			lForm[i] = 0;
		}

		if (numTermsAdded == 0)
		{
			consumer->ConsumeLinForm(productIndex, oneMonomial->coef, 0, onesVector);
		}//oneMonomial is a constant term.
	}//for each monomial.

	delete consumer;
	delete polynomialIterator;

	RationalNTL ans;
	ans = findIntegral(plf, integrateProductLinearFormsTriangulation);

	destroyLinFormProducts(plf);

	return ans;
}//findIntegralPolynomialToPLF




/* computes the integral of a product of powers of linear forms over each simplex
 * in the triangulation of the polytope.
 * @Assumes: polytope has integer vertices and it is triangulated.
 * @input: product of linear forms after the polytope has been dilated
 * @return RationalNTL: the integral of the polytope over every product.
 *
 * Math: integral(over simplex) <l_1, x>^m_1 ... <l_d, x>^m_D =
 *
 *               abs(det(matrix formed by the rays))* M!/(d+|M|)!
 *  --------------------------------------------------------------------------
 *  product(over j)(1 - <l_1, s_j>t_1 - <l_2, s_j>t_2 - ... - <l_D, s_j>t_d )
 *                                 \          \_these are numbers
 *                                  \_the t are symbolic.
 *
 *  where we want the coefficient of t_1^m_1 ... t_D^m_D in the polynomial
 *  expansion of the RHS,
 *
 * and where s_j is a vertex,
 *       l_i is a linear form
 *       D is any number of products
 *       M is the power vector
 *       M! = m_1! m_2! ... m_D!
 *       |M| = m_1 + m_2 + ... + m_D.
 *
 * Note that we cannot divide by zero.
 *
 * See the paper: "How to Integrate a Polynomial over a Simplex" by
 * V. BALDONI, N. BERLINE, J. A. DE LOERA, M. KOEPPE, M. VERGNE.
 */
RationalNTL PolytopeValuation::findIntegralProductsUsingTriangulation(linFormSum &forms) const
{
	RationalNTL answer;
	int simplicesFinished = 0;
	int totalSimplicesToIntegrate = lengthListCone(triangulatedPoly);

	if ( forms.termCount == 0)
		return RationalNTL(); //nothing to do.

	//set up itrator to loop over the linear forms.
	BTrieIterator<RationalNTL, ZZ>* linearFormIterator = new BTrieIterator<
			RationalNTL, ZZ> ();
	linearFormIterator->setTrie(forms.myForms, forms.varCount);



	//loop over every simplex.
	for (listCone * currentCone = triangulatedPoly; currentCone; currentCone
			= currentCone->rest)
	{
		//First construct a simplex.
		simplexZZ oneSimplex;
		oneSimplex.d = numOfVarsOneCone -1;



		int vertexCount = 0; //the current vertex number being processed.
		oneSimplex.s.SetLength(numOfVarsOneCone);

		for (listVector * rays = currentCone->rays; rays; rays = rays->rest, ++vertexCount)
		{
			oneSimplex.s[vertexCount].SetLength(numOfVarsOneCone-1);
//			assert( rays->first[numOfVarsOneCone-1] == 1); //make sure the triangulation is such that the vertices of the original polytope is integer.

			for (int k = 0; k < numOfVarsOneCone -1; ++k)
				oneSimplex.s[vertexCount][k] = rays->first[k];//save the vertex.

		}//create the simplex. Don't copy the leading 1.

		//compute the volume of the Parallelepiped
		mat_ZZ matt;
		matt.SetDims(oneSimplex.d, oneSimplex.d);
		for (int j = 1; j <= oneSimplex.d; j++)
			matt[j - 1] = oneSimplex.s[j] - oneSimplex.s[0];
		oneSimplex.v = abs(determinant(matt));

		//Finally, we are ready to integrate the products of linear forms over the simplex!
		RationalNTL integral;
		integral = integrateLinFormProducts(linearFormIterator, oneSimplex, forms.termCount);

		answer.add(integral);
		++simplicesFinished;
		if ( simplicesFinished % 1000 == 0)
			cerr << "Finished integrating " << simplicesFinished << "/" << totalSimplicesToIntegrate << " over " << forms.termCount << " linear forms\n";
		//answer.add(numerator * coefficient.getNumerator(), denominator * coefficient.getDenominator());
	}//for every triangulated simplex.
	delete linearFormIterator;

	return answer;
}//findIntegralProductsUsingTriangulation


/** Finds the volume of the current polytope using one of two methods
 *  Triangulate Method: find a triangulation and sum a bunch of determinants.
 *  Lawrence Method: Sum over every simple cone.
 */
RationalNTL PolytopeValuation::findVolume(ValuationAlgorithm algorithm)
{
	RationalNTL answer;

	if (algorithm == volumeTriangulation)
	{

		convertToOneCone();
		triangulatePolytopeCone(); //

		for (listCone * oneSimplex = triangulatedPoly; oneSimplex; oneSimplex
				= oneSimplex->rest)
			answer.add(findVolumeUsingDeterminant(oneSimplex));

		//cout << "findVolumeUsingDeterminant(): VOLUME: " << answer << endl;
	} else if (algorithm == volumeCone)
	{
		triangulatePolytopeVertexRayCone();
		answer = findVolumeUsingLawrence();

		//cout << "findVolumeUsingLawrence(): VOLUME: " << answer << endl;
	}


	return answer;

}//findVolume


/**
 * Computes the volume for one simplex.
 *
 * The volume of an n-simplex in n-dimensional space with vertices (v0, ..., vn) is the abs. value of
 * {1\over n!}\det \begin{pmatrix} v_1-v_0 & v_2-v_0& \dots & v_{n-1}-v_0 & v_n-v_0 \end{pmatrix}
 *
 * However, using facts about how the determinant does not change when adding a multiple of a col. to another col,
 * and det(A) = 1/a * det ( times one col of A by a), we can get away with doing this subtraction,
 * and we do not have to project the vertices back down ( i.e. remove the leading one/element).
 *
 * We compute the abs. value of
 * {1\over n!}  {1 \over {v_o[0]*v_1[0]*...v_n[0]}} \det \begin{pmatrix} v_0 & v_1 & v_2 & \dots & v_{n-1} & v_n \end{pmatrix}
 */
RationalNTL PolytopeValuation::findVolumeUsingDeterminant(
		const listCone * oneSimplex) const
{
	int i, numOfRays;
	mat_ZZ mat;

	vec_ZZ head;
	vec_ZZ tail;
	ZZ numerator, denominator;
	numerator = 1;
	denominator = 1;

	numOfRays = lengthListVector(oneSimplex->rays);


	mat.SetDims(numOfRays, numOfVarsOneCone);

	i = 0;
	for (listVector * currentRay = oneSimplex->rays; currentRay; currentRay
			= currentRay->rest)
	{
		for (int k = 0; k < numOfVarsOneCone; ++k)
			mat[i][k] = ((currentRay->first)[k]);
		denominator *= (currentRay->first)[numOfVarsOneCone-1];//get the scale factor from dilation.
		++i;
	}//for currentRay

	numerator = abs(determinant(mat));
	denominator *= factorial(numOfRays - 1);
	//cout << mat << " = " << determinant(mat) << "\n./." << factorial(numOfRays -1) << endl;
	return RationalNTL(numerator, denominator);
}//findDetermiantForVolume


/* computes the volume of a polytope using the lawrence forumla
 * takes into account the coefficient given to a cone when decomposed into unimodular cones
 * thus it works on all inputs
 * @input: a listCone of the cones and the nnumber of variables (dimension of the space)
 * @return RationalNTL: the volume of the polytope
 *
 * Math: For each simple vertex-ray cone, we sum the fractions
 *
 * 	<v, c>^d * det(matrix formed by the rays) * cone's coefficient
 *  --------------------------------------------------------------
 *                <-r_1, c> * <-r_2, c>*...*<-r_d, c>
 * where v is a vertex,
 *       c is a random vector,
 *       d is the dimension
 *       r_i is the ith ray of the cone.
 *
 * We use the cone's coefficient in case the cone decomposition is signed (ex, unimodular decomposition).
 */
RationalNTL PolytopeValuation::findVolumeUsingLawrence()
{
	RationalNTL answer;
	bool dividedByZero;

	vec_ZZ c = vec_ZZ();
	ZZ scale = ZZ();
	ZZ num = ZZ();
	ZZ denom = ZZ();
	denom = 1;
	ZZ tempNum = ZZ();
	ZZ tempDenom = ZZ();
	vec_ZZ vert = vec_ZZ();
	vec_ZZ ans = vec_ZZ();
	mat_ZZ mat;


	ZZ det = ZZ();
	RationalNTL detRational;
	detRational = 1;
	mat.SetDims(numOfVars, numOfVars);

	c.SetLength(numOfVars);


	do{
		dividedByZero = false;
		answer = 0;

		//pick the random c vector.
		for (int i = 0; i < numOfVars; i++)
			c[i] =rand() % 10000;
		for (listCone * simplicialCone = triangulatedPoly; simplicialCone; simplicialCone
				= simplicialCone->rest)
		{

			//find vertex
			vert = scaleRationalVectorToInteger(simplicialCone->vertex->vertex,
					numOfVars, tempDenom);

			//raise f(vertex) to the power of the dimension
			tempNum = vert * c;
			tempNum = power(tempNum, numOfVars);
			tempDenom = power(tempDenom, numOfVars);

			int col = 0;

			for (listVector * currentRay = simplicialCone->rays; currentRay; currentRay
				= currentRay->rest, col++)
			{
				tempDenom *= -1 * c * currentRay->first;

				//generate matrix
				for (int row = 0; row < numOfVars; row++)
				{
					mat[row][col] = currentRay->first[row];
				}//for every component of the ray

			}//for every ray in the simple cone

			//get the determinant
			//cout << "findVol Law:: dim=" << dimension << ", num of var =" << numOfVars << endl;
			if(dimension == numOfVars)
			{
				determinant(det, mat);
			}
			else
			{
				//todo: find the volume of a non-full-din polytope.
				detRational = abs(NTL::determinant((*latticeInverse) * mat));
				detRational = detRational / power(*latticeInverseDilation, latticeInverse->NumCols());
				det = detRational.getNumerator();
				cout << "this is not tested yet" << endl;
				exit(1);
			}//need to use the integer linear space basis to find the volume.

			//multiply by the absolute value of the determinant
			tempNum *= abs(det) * simplicialCone->coefficient;
			//tempDenom *= detRational.getDenominator(); //init to 1.
			//cout << "detRational = " << detRational.getNumerator() << "/" << detRational.getNumerator()<< endl;
			if ( tempDenom == 0)
			{
				cerr << "findVolumeUsingLawrence:: divided by zero, trying again" << endl;
				dividedByZero = true;
				break; //leave the for loop and the do-while loop will start over.
			}

			//add current term to the running total
			answer.add(tempNum, tempDenom);
		}//for every simple cone in the cone
	} while ( dividedByZero == true);

	ZZ one;
	one = 1;
	answer.mult(one, factorial(numOfVars));

	return answer;
}//findVolumeUsingLarence()




/**
 * Returns the lowest common multiple of a and b.
 */
ZZ PolytopeValuation::lcm(const ZZ &a, const ZZ & b)
{
	return (a * b) / GCD(a, b);
}//lcm

/**
 * Find the Lawrence rational function for volume.
 */
void PolytopeValuation::printLawrenceVolumeFunction()
{
	listCone * triangulatedCones;
	vec_ZZ vert = vec_ZZ();
	ZZ temp = ZZ();
	mat_ZZ mat;
	ZZ det;
	mat.SetDims(numOfVars, numOfVars);

	triangulatePolytopeVertexRayCone();

	cout << "( ";
	for (listCone * simplicialCone = triangulatedPoly; simplicialCone; simplicialCone
			= simplicialCone->rest)
	{
		vert = scaleRationalVectorToInteger(simplicialCone->vertex->vertex,
				parameters.Number_of_Variables, temp);
		cout << "( ";

		//dot of c and v raised to the dim power
		for (int i = 0; i < parameters.Number_of_Variables; i++)
		{
			cout << vert[i];
			if (temp != 1)
				cout << " / " << temp;
			cout << " * c" << i;
			if (i != parameters.Number_of_Variables - 1)
				cout << " + ";
		}
		cout << " ) ^ " << parameters.Number_of_Variables << " / ( ";

		//the correct sign on the denominator
		if (parameters.Number_of_Variables % 2 == 1)
			cout << "-";

		//divide by the multiplication of all the dots
		//of the rays dotted with c
		int col = 0;

		for (listVector * currentRay = simplicialCone->rays; currentRay; currentRay
				= currentRay->rest, col++)
		{
			cout << "( ";
			for (int row = 0; row < numOfVars; row++)
			{
				cout << currentRay->first[row] << " * c" << row;
				if (row != parameters.Number_of_Variables - 1)
				{
					cout << " + ";
				}
				mat[row][col] = currentRay->first[row];
			}
			cout << " )";
			if (currentRay->rest != NULL)
				cout << " * ";
		}//for every ray

		//get the determinant
		determinant(det, mat);

		//close up the denominator
		cout << " ) * ";

		//multiply by the coefficient and determinant
		cout << simplicialCone->coefficient;
		if (det != 1)
			cout << " * (" << abs(det) << ')';

		//if more cones, type the +
		if (simplicialCone->rest != NULL)
			cout << " + ";
	}//for every simple cone.

	// divide the sum by the factorial
	cout << ") / ( " << parameters.Number_of_Variables << "!";
	cout << " )" << endl;
}


void PolytopeValuation::setLatticeInverse(const mat_ZZ * _latticeInverse, const ZZ * _latticeInverseDilation)
{
	latticeInverse = _latticeInverse;
	latticeInverseDilation = _latticeInverseDilation;
}

void PolytopeValuation::setFullDimension(int d)
{
	dimension = d;
}

/**
 * Triangulates 1 cone (which encodes the polytope. See the comments for convertToOneCone()
 * to learn how the polytope is encoded in one cone.)
 */
void PolytopeValuation::triangulatePolytopeCone()
{
	if (triangulatedPoly)
		return; //all ready did computation.
	if (polytopeAsOneCone == NULL)
	{
		cout
				<< "PolytopeValuation::triangulatePolytopeCone(): there is no cone to triangulate"
				<< endl;
		exit(1);
	}
	parameters.Number_of_Variables = numOfVarsOneCone;
	triangulatedPoly = triangulateCone(polytopeAsOneCone, numOfVarsOneCone,
			&parameters);
	parameters.Number_of_Variables = numOfVars; //convert back.
	freeTriangulatedPoly = 1; //Delete this in the deconstructor.
}//triangulateCone()


/**
 * Triangulate the cones from they vertex-ray cone.
 */
void PolytopeValuation::triangulatePolytopeVertexRayCone()
{
	if (triangulatedPoly)
		return; //already did computation
/*
	cout << "vertex cone before triangulation" << endl;
	printListCone(vertexRayCones, numOfVars);
	cout << "end of vertex cone before triangulation" << endl;
//*/
//*

	listCone * currentCone = vertexRayCones;
	int numVertexCones = lengthListCone(vertexRayCones);

	while ( currentCone)
	{

		listCone* newTriangulation;

		cerr << numVertexCones-- << " cone triangulations left.\n";
		newTriangulation = triangulateCone(currentCone, numOfVars, &parameters);

		triangulatedPoly = appendListCones(newTriangulation, triangulatedPoly);

		currentCone = currentCone->rest;
	}

/*
	triangulatedPoly = decomposeCones(vertexRayCones,
			parameters.Number_of_Variables, parameters.Flags,
			parameters.File_Name, 0, false, //true = take the dual, false = use the primal cone.
			BarvinokParameters::IrrationalPrimalDecomposition);
 //*/
/*
	cout << "vertex cone after triangulation" << endl;
	printListCone(triangulatedPoly, numOfVars);
	cout << "end of vertex cone after triangulation" << endl;
//*/
	freeTriangulatedPoly = 1; //Delete this in the deconstructor.
}
