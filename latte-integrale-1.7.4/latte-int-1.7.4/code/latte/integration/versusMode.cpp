#include "newIntegration.h"
#include "multiply.h"
#include "PolyTrie.h"
#include "PolyRep.h"
#include "../timing.h"
#include <iostream>
#include <fstream>
#include <climits>
#include <cstring>
#include <NTL/ZZ.h>

using namespace std;

//old multiplication
template<class T>
void _multiply(_monomialSum& first, _monomialSum& second, _monomialSum& result,
		int* min, int* max)
{
	//cout << "Old multiply" << endl;
	eBlock* firstExp = first.eHead;
	cBlock<T>* firstCoef = first.cHead;
	eBlock* secondExp;
	cBlock<T>* secondCoef;

	int* exponents = new int[result.varCount];
	bool valid;
	result.termCount = 0;
	for (int i = 0; i < first.termCount; i++)
	{
		if (i > 0 && i % BLOCK_SIZE == 0) //this block is done, get next one
		{
			firstExp = firstExp->next;
			firstCoef = firstCoef->next;
		}
		secondExp = second.eHead;
		secondCoef = second.cHead;
		for (int j = 0; j < second.termCount; j++)
		{
			if (j > 0 && j % BLOCK_SIZE == 0) //this block is done, get next one
			{
				secondExp = secondExp->next;
				secondCoef = secondCoef->next;
			}
			valid = true;
			for (int k = 0; k < result.varCount; k++)
			{
				exponents[k] = firstExp->data[(i % BLOCK_SIZE) * first.varCount
						+ k] + secondExp->data[(j % BLOCK_SIZE)
						* second.varCount + k];
				//if (exponents[k] < min[k] || exponents[k] > max[k]) {valid = false; break; }
			}
			if (valid) //all exponents are within range
			{
				_insertMonomial<T> (firstCoef->data[i % BLOCK_SIZE]
						* secondCoef->data[j % BLOCK_SIZE], exponents, result);
			}
		}

	}
	delete[] exponents;
}

int main(int argc, char *argv[])
{
	string line;
	int option = 0;
	if (strcmp(argv[1], "-m") == 0)
	{
		option = 1;
	} //multiply
	else if (strcmp(argv[1], "-d") == 0)
	{
		option = 2;
	} //decompose
	int dimension = -1;
	monomialSum myPoly, myProduct;
	_monomialSum _myPoly, _myProduct;
	linFormSum myForms;
	_linFormSum _myForms;
	ifstream myStream(argv[2]);
	if (!myStream.is_open())
	{
		cout << "Error opening file " << argv[2]
				<< ", please make sure it is spelled correctly." << endl;
		return 1;
	}
	int polyCount = 0;
	string testForms;
	int *low, *high;

	float myTime;
	Timer myTimer("");

	float oldMult, oldDecomp, newMult, newDecomp, oldInt, newInt;
	oldMult = oldDecomp = newMult = newDecomp = oldInt = newInt = 0.0f;

	MonomialLoadConsumer<RationalNTL>* myLoader = new MonomialLoadConsumer<
			RationalNTL> ();
	_MonomialLoadConsumer<RationalNTL>* _myLoader = new _MonomialLoadConsumer<
			RationalNTL> ();

	BTrieIterator<RationalNTL, int>* it =
			new BTrieIterator<RationalNTL, int> ();
	BTrieIterator<RationalNTL, int>* it2 =
			new BTrieIterator<RationalNTL, int> ();

	MBlockIterator<RationalNTL>* _it = new MBlockIterator<RationalNTL> ();
	MBlockIterator<RationalNTL>* _it2 = new MBlockIterator<RationalNTL> ();

	LBlockIterator<RationalNTL>* it_ = new LBlockIterator<RationalNTL> ();
	BTrieIterator<RationalNTL, ZZ>* it2_ =
			new BTrieIterator<RationalNTL, ZZ> ();

	ZZ numerator, denominator;

	while (!myStream.eof())
	{
		getline(myStream, line, '\n');
		if (!line.empty())
		{
			//cout << "current line = " << line.c_str() << endl;
			//cout << "got here 121" << endl;
			myPoly.termCount = 0;
			myLoader->setMonomialSum(myPoly);
			parseMonomials(myLoader, line);
			//cout << "after parseMonomials" << endl;
			if (myPoly.termCount == 0 || myPoly.varCount == 0)
			{
				cout << "Error: loaded invalid monomial sum." << endl;
				return 1;
			} else
			{
				myForms.varCount = myProduct.varCount = myPoly.varCount;
				if (dimension == -1)
				{
					dimension = myPoly.varCount;
				}
			}

			_myPoly.termCount = 0;
			_myLoader->setMonomialSum(_myPoly);
			_parseMonomials(_myLoader, line);

			if (_myPoly.termCount == 0 || _myPoly.varCount == 0)
			{
				cout << "Error: loaded invalid monomial sum." << endl;
				return 1;
			} else
			{
				_myForms.varCount = _myProduct.varCount = _myPoly.varCount;
			}

			//square the polynomials
			if (option == 1)
			{
				low = new int[myProduct.varCount];
				high = new int[myProduct.varCount];
				for (int i = 0; i < myProduct.varCount; i++)
				{
					low[i] = INT_MIN;
					high[i] = INT_MAX;
				}

				myTime = myTimer.get_seconds();
				myTimer.start();
				_multiply<RationalNTL> (_myPoly, _myPoly, _myProduct, low, high);
				myTimer.stop();
				myTime = myTimer.get_seconds() - myTime;
				oldMult += myTime;
				//cout << myTime << " vs. ";

				_it->setLists(_myPoly.eHead, _myPoly.cHead, _myPoly.varCount,
						_myPoly.termCount);
				_it2->setLists(_myPoly.eHead, _myPoly.cHead, _myPoly.varCount,
						_myPoly.termCount);
				myTime = myTimer.get_seconds();
				myTimer.start();
				multiply<RationalNTL> (_it, _it2, _myProduct, low, high);
				myTimer.stop();
				myTime = myTimer.get_seconds() - myTime;
				//cout << myTime << endl;
				//oldMult += myTime;
				//cout << "Old Algorithm @ dimension " << _myProduct.varCount << ":" <<  myTime << "s. " << endl;
				_destroyMonomials(_myProduct);

				it->setTrie(myPoly.myMonomials, myPoly.varCount);
				it2->setTrie(myPoly.myMonomials, myPoly.varCount);
				myTime = myTimer.get_seconds();
				myTimer.start();
				multiply<RationalNTL> (it, it2, myProduct, low, high);
				myTimer.stop();
				myTime = myTimer.get_seconds() - myTime;
				newMult += myTime;
				//cout << "New Algorithm @ dimension " << myProduct.varCount << ":" <<  myTime << "s. " << endl;
				destroyMonomials(myProduct);

				delete[] low;
				delete[] high;
			} else if (option == 2)
			{
				_myForms.termCount = 0;
				myTime = myTimer.get_seconds();
				myTimer.start();

				for (int i = 0; i < _myPoly.termCount; i++)
				{
					_decompose(_myPoly, _myForms, i);
				}
				myTimer.stop();
				myTime = myTimer.get_seconds() - myTime;
				oldDecomp += myTime;

				simplexZZ mySimplex;
				getline(myStream, line, '\n');
				convertToSimplex(mySimplex, line);

				it_->setLists(_myForms.lHead, _myForms.cHead,
						_myForms.varCount, _myForms.termCount);
				myTime = myTimer.get_seconds();
				myTimer.start();
				integrateLinFormSum(numerator, denominator, it_, mySimplex);
				myTimer.stop();
				oldInt += (myTimer.get_seconds() - myTime);

				_destroyLinForms(_myForms);

				myForms.termCount = 0;
				it->setTrie(myPoly.myMonomials, myPoly.varCount);
				myTime = myTimer.get_seconds();
				myTimer.start();
				decompose(it, myForms);
				myTimer.stop();
				myTime = myTimer.get_seconds() - myTime;
				newDecomp += myTime;

				it2_->setTrie(myForms.myForms, myForms.varCount);
				myTime = myTimer.get_seconds();
				myTimer.start();
				integrateLinFormSum(numerator, denominator, it2_, mySimplex);
				myTimer.stop();
				newInt += (myTimer.get_seconds() - myTime);

				destroyLinForms(myForms);
			}
		}
	}

	delete myLoader;
	delete _myLoader;
	delete it;
	delete it2;
	delete _it;
	delete _it2;
	delete it_;
	delete it2_;
	myStream.close();
	cout << "Comparing @ dimension " << dimension << " (old vs. new) " << endl;
	if (option == 1)
	{
		cout << "Squaring polynomials: " << oldMult << " vs. " << newMult
				<< endl;
	} else if (option == 2)
	{
		cout << "Decomposing: " << oldDecomp << " vs. " << newDecomp << endl
				<< "Integrating: " << oldInt << " vs. " << newInt << endl;
	}
	return 0;
}
