/* maximize.cpp -- Master program

   Copyright 2002, 2003 Raymond Hemmecke, Ruriko Yoshida
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

#include <string.h>
#include <stdio.h>

#include "config.h"
#include "barvinok/dec.h"
#include "barvinok/barvinok.h"
#include "barvinok/Triangulation.h"
#include "vertices/cdd.h"
#include "genFunction/maple.h"
#include "genFunction/piped.h"
#include "cone.h"
#include "dual.h"
#include "RudyResNTL.h"
#include "Residue.h"
#include "Grobner.h"
//  #include "jesus.h"
#include "preprocess.h"
#include "print.h"
#include "ramon.h"
#include "rational.h"
#include "timing.h"
#include "flags.h"
//#include "testing.h"
#include "IntegralHull.h"
#include "ReadingFile.h"
#include "binarySearchIP.h"
#include "CheckEmpty.h"
#include "ProjectUp.h"

#include "banner.h"
#include "convert.h"
#include "latte_system.h"

#include "gnulib/progname.h"
#include "gnulib/pathmax.h"

/* ----------------------------------------------------------------- */
int main(int argc, char *argv[]) {
  set_program_name(argv[0]);

  BarvinokParameters params;
#ifdef SUN
  struct tms tms_buf;
#endif
  float z;
  int i,numOfVars,numOfAllVars, degree = 1;
  unsigned int flags = 0, print_flag = 0, output_cone = 0;
  vec_ZZ dim, v, w;
  int oldnumofvars;
  vec_ZZ *generators;
  char fileName[PATH_MAX], invocation[10000], decompose[10], equationsPresent[10],
    assumeUnimodularCones[127], dualApproach[127], taylor[127], printfile[127],
    rationalCone[127], nonneg[127], Memory_Save[127], Save_Tri[127],
    Load_Tri[127], Print[127], inthull[127], cddstyle[127], grobner[127],
    removeFiles[127], command[10000], maximum[127],  Singlecone[127], LRS[127],
    Vrepresentation[127], dilation[127], minimize[127], binary[127], interior[127];
  listVector *matrix, *equations, *inequalities, *rays, *endRays, *tmpRays, *matrixTmp;
  vec_ZZ cost;
  listVector *templistVec;
  listCone *cones, *tmp, *tmpcones;

  latte_banner(cerr);

  z=0;
  //setbuf(stdout,0);

  strcpy(invocation,"Invocation: ");
  strcat(invocation,argv[0]);
  strcat(invocation," ");
/*    strcat(invocation,argv[argc-1]); */
/*    strcat(invocation,"\n\n"); */
/*    printf(invocation); */

  strcpy(Vrepresentation,"no");
  strcpy(interior,"no");
  strcpy(dilation,"no");
  strcpy(binary,"no");
  strcpy(Singlecone,"no");
  strcpy(removeFiles,"yes");
  strcpy(grobner,"no");
  strcpy(maximum,"yes");
  strcpy(minimize,"no");
  strcpy(decompose,"yes");
  strcpy(dualApproach,"no");
  strcpy(equationsPresent,"no");
  strcpy(assumeUnimodularCones,"no");
  strcpy(printfile,"no");
  strcpy(taylor,"no");
  strcpy(rationalCone,"no");
  strcpy(nonneg, "no");
  strcpy(Memory_Save, "no");
  strcpy(Save_Tri, "no");
  strcpy(Load_Tri, "no");
  strcpy(Print, "no");
  strcpy(inthull, "no");
  strcpy(cddstyle, "no");
  strcpy(LRS, "no");

  for (i=1; i<argc-1; i++) {
    strcat(invocation,argv[i]);
    strcat(invocation," ");
    //if (strncmp(argv[i],"vrep",3)==0) strcpy(Vrepresentation,"yes"); 
    if (strncmp(argv[i],"bbs",3)==0) strcpy(binary,"yes");
    //if (strncmp(argv[i],"int",3)==0) strcpy(interior,"yes");
    //if (strncmp(argv[i],"min",3)==0) strcpy(minimize,"yes");
    // if (strncmp(argv[i],"gro",3)==0) strcpy(grobner,"yes");
    //if (strncmp(argv[i],"nodecom",3)==0) strcpy(decompose,"no");
    //if (strncmp(argv[i],"homog",3)==0) {strcpy(dualApproach,"yes"); flags |= DUAL_APPROACH;}
    if (strncmp(argv[i],"equ",3)==0) strcpy(equationsPresent,"yes");
    if (strncmp(argv[i],"uni",3)==0) strcpy(assumeUnimodularCones,"yes");
    //if (strncmp(argv[i],"simp",4)==0) {strcpy(printfile,"yes"); flags |= PRINT;}
    if(strncmp(argv[i],"file",4)==0) strcpy(Memory_Save, "no");
    if (strncmp(argv[i],"single",6)==0) strcpy(Singlecone,"yes");
    //if (strncmp(argv[i],"tay",3)==0) strcpy(taylor,"yes");
    // if (strncmp(argv[i],"ehrhartsimp",3)==0) strcpy(rationalCone,"yes");
    if (strncmp(argv[i],"+", 1) ==0) strcpy(nonneg,"yes");
    //if (strncmp(argv[i],"memsave",7)==0) strcpy (Memory_Save, "yes");
    if (strncmp(argv[i],"printcones",3)==0) strcpy (Print, "yes");
    if (strncmp(argv[i],"cdd",3)==0) strcpy (cddstyle, "yes");
    if (strncmp(argv[i],"hull",3)==0) strcpy (inthull, "yes");
    if (strncmp(argv[i],"max",3)==0) strcpy (maximum, "yes");
    if (strncmp(argv[i],"lrs",3)==0) strcpy (LRS, "yes");
    if (strncmp(argv[i],"dil",3)==0) strcpy (dilation, "yes");
    if (strncmp(argv[i],"rem",3)==0) strcpy (removeFiles, "yes");
    //if (strncmp(argv[i],"trisave",7)==0) {strcpy (Save_Tri, "yes"); flags |= SAVE;}
    //if (strncmp(argv[i],"triload",7)==0) {strcpy (Load_Tri, "yes"); flags |= LOAD;}
  }
  if(minimize[0] == 'y') strcpy(maximum, "yes");
  if(grobner[0] == 'y') strcpy(equationsPresent,"yes");
  if(binary[0] == 'y') {strcpy(maximum,"yes"); strcpy(Memory_Save, "no");}
  if(maximum[0] == 'y') strcpy(Memory_Save, "no");
  if(printfile[0] == 'y') strcpy(Memory_Save, "no");
  if(rationalCone[0] == 'y') strcpy(Memory_Save, "no");
  if(printfile[0] == 'y') print_flag = 1;
  if(taylor[0] == 'y'){
    degree = atoi(argv[argc-2]);
  }
  if(rationalCone[0] == 'y'){
    
    //HugInt digit(argv[1]);
    //conv(output_cone, digit.BigInt);
    // User can use only Mode one
    output_cone = 3;
  }
  int dilation_const = 1;

  if(dilation[0] == 'y') dilation_const = atoi(argv[argc-2]);

  if(output_cone > 3) output_cone = 0;
  flags |= (output_cone << 1);
  
  if((dualApproach[0] == 'y') && (nonneg[0] == 'y')&&(equationsPresent[0] == 'n')){
    cerr<<"You cannot use + and dua at the same time." << endl;
    exit(2);
  }

  if((cddstyle[0] == 'y') && (Vrepresentation[0] == 'y')){
    cerr << "Use not cdd style and v-representation." << endl;
    exit(2);
  }
  
  if((Memory_Save[0] == 'y') && (inthull[0] == 'y')){
    cerr<<"You cannot use int and memsave at the same time." << endl;
    exit(3);
  }

  strcat(invocation,argv[argc-1]);
  strcat(invocation,"\n\n");
  cerr << invocation;
  char costFile[PATH_MAX];
  if(maximum[0] == 'y'){
    strcpy(fileName,argv[argc-1]);
    strcpy(costFile, argv[argc - 1]);
    strcat(costFile, ".cost");
  }
  else strcpy(fileName,argv[argc-1]);
  //  cerr << fileName << " " << costFile << endl;
  if(maximum[0] == 'y') {
    ifstream ReadTest(fileName);
    if(!ReadTest){
      cerr << "Need a polytope input file." << endl;
      exit(2);
    }
    //    cerr << fileName << " " << costFile << endl;
    ifstream INCost(costFile);
    if(!INCost){
      cerr << "Need a cost input file." << endl;
      exit(3);
    }
    int costDim, dummy;
    INCost >> dummy >> costDim; 
    cost.SetLength(costDim);
    for(i = 0; i < costDim; i++)
      INCost >> cost[i]; 
  }
  //strcpy (fileName,"stdin");
  
  /* Check input file. */
  if(Vrepresentation[0] == 'n'){
    if((cddstyle[0] == 'n') && (grobner[0] == 'n') && (maximum[0] == 'n')&& (minimize[0] == 'n')){
      CheckInputFile(fileName);
      CheckLength(fileName, equationsPresent);
    }
    if(minimize[0] == 'y')  strcpy (maximum, "yes");   
 
    if((cddstyle[0] == 'n') && (grobner[0] == 'n') && (maximum[0] == 'y')){
      CheckInputFile(fileName);
      CheckLength(fileName,equationsPresent);
    }

    if(cddstyle[0] == 'y')
      { CheckInputFileCDDRep(argv[argc - 1]);
      CheckInputFileCDDRep1(argv[argc - 1]);
      CheckInputFileCDDRep3(argv[argc - 1]);
      CheckInputFileCDDRep4(argv[argc - 1]);
      }
  }else CheckInputFileVrep(fileName);
    CheckEmpty(fileName);
  //vector cost;
  /* Read problem data. */
  if((cddstyle[0] == 'n') && (Vrepresentation[0] == 'n')) CheckRed(fileName, equationsPresent, maximum, nonneg, interior, dilation, dilation_const); 

  if((cddstyle[0] == 'n') && (grobner[0] == 'n'))
    readLatteProblem(fileName,&equations,&inequalities,equationsPresent,
		     &numOfVars, nonneg, dualApproach, grobner, Vrepresentation);

  if((equationsPresent[0] == 'n') && (interior[0] == 'y'))
    Interior(inequalities);

  // if(minimize[0] == 'y') cost = -cost;
  if(cddstyle[0] == 'y'){
    int tmpoutput;
    CDDstylereadLatteProblem(fileName,&equations,&inequalities,equationsPresent,
			     &numOfVars, nonneg, dualApproach, taylor, degree,
			     rationalCone, tmpoutput, Memory_Save,
			     assumeUnimodularCones, inthull, grobner);
    output_cone = 3;
    if (dualApproach[0] == 'y') {
      flags |= DUAL_APPROACH;
    }
  }
  
  // cerr << grobner << endl;
  vec_ZZ holdCost;
  if(minimize[0] == 'y') cost = - cost;
  holdCost = cost;
  //cerr <<"Cost is: " << cost << endl;
  vec_RR holdcost_RR;
  holdcost_RR.SetLength(holdCost.length());
  for(i = 0; i < holdCost.length(); i++) conv(holdcost_RR[i], holdCost[i]);

  if(minimize[0] == 'y') holdcost_RR = - holdcost_RR;
  if(grobner[0] == 'y'){

    CheckGrobner(fileName, cddstyle);
    SolveGrobner(fileName,  nonneg, dualApproach,
		 grobner, equationsPresent, cddstyle);}
  else{
  if((dualApproach[0] == 'y') && (nonneg[0] == 'y')&&(equationsPresent[0] == 'n')){
    cerr<<"You cannot use + and dua at the same time." << endl;
    exit(2);
  }
  
  if((Memory_Save[0] == 'y') && (inthull[0] == 'y')){
    cerr<<"You cannot use int and memsave at the same time." << endl;
    exit(3);
  }
  
  if((Vrepresentation[0] == 'y') && (equationsPresent[0] == 'y')){
    cerr<<"You cannot use vrep and equ at the same time." << endl;
    exit(4);
  }

  numOfVars--;
  /* Binary seach IP*/

  if(binary[0] == 'y'){
    cerr << "The number of optimal solutions: " << binarySearch(equations, inequalities,cost, numOfVars, minimize) << endl;
    cerr << "Time: " << GetTime() << endl;
    exit(0);
  }

  numOfAllVars=numOfVars;
  mat_ZZ ProjU;
  ProjU.SetDims(numOfVars, numOfVars);
  oldnumofvars = numOfVars;
  generators=createArrayVector(numOfVars);
  if (equationsPresent[0]=='y') {
    /*    if(grobner[0] == 'y')
	  {  
     matrixTmp=Grobner(equations,inequalities,&generators,&numOfVars, &templistVec, oldnumofvars);
     
     }*/
    matrixTmp=preprocessProblem(equations,inequalities,&generators,&numOfVars, cost, ProjU, interior, dilation_const);
      templistVec = transformArrayBigVectorToListVector(ProjU, ProjU.NumCols(), ProjU.NumRows()); 
  } else {
    dilateListVector(inequalities, numOfVars, dilation_const);
    matrixTmp=inequalities;
  }
  if((dualApproach[0] == 'y')&&(equationsPresent[0]=='y')){
   matrix = TransformToDualCone(matrixTmp,numOfVars);
   }
  else {
    matrix = matrixTmp;}
/* Now matrix contains the new inequalities. */
  RR LP_OPT;
    cerr << "\nTime: " << GetTime() << " sec\n\n";
    //   cerr << "Project down cost function: " << cost << endl;
    vec_RR Rat_solution, tmp_den, tmp_num;
    mat_RR ProjU_RR;
    ProjU_RR.SetDims(ProjU.NumRows(), ProjU.NumCols());
    for(i = 0; i < ProjU.NumRows(); i++)
      for(int j = 0; j < ProjU.NumCols(); j++) conv(ProjU_RR[i][j], ProjU[i][j]);
    //cerr << ProjU << ProjU_RR << endl;
    Rat_solution.SetLength(numOfVars);
    tmp_den.SetLength(numOfVars);
    tmp_num.SetLength(numOfVars);
/* Compute vertices and edges. */
    rationalVector* LP_vertex;
  if ((dualApproach[0]=='n') && (Vrepresentation[0] == 'n')) {
    if(LRS[0] == 'n')
    tmpcones=computeVertexCones(fileName,matrix,numOfVars);
    else
      tmpcones=computeVertexConesViaLrs(fileName,matrix,numOfVars);
      if(maximum[0] == 'y'){ 
     LP_vertex = LP(matrix, cost, numOfVars);
     vec_RR Rat_cost;  Rat_cost.SetLength(numOfVars);
     for (i = 0; i < numOfVars; i++){
       conv(tmp_num[i], LP_vertex->numerators()[i]);
       conv(tmp_den[i], LP_vertex->denominators()[i]);
       Rat_solution[i] = tmp_num[i]/tmp_den[i];
       conv(Rat_cost[i], cost[i]);
     }
     if(Singlecone[0] == 'y')
       cones = CopyListCones(tmpcones, numOfVars, LP_vertex);
     else cones = tmpcones;
       if(lengthListCone(cones) == 1) 
	cerr <<"\nWe found a single vertex cone for IP.\n" << endl;
       cerr <<"A vertex which we found via LP is: " << ProjectingUpRR(ProjU_RR, Rat_solution, numOfVars) << endl;
      //printRationalVector(LP_vertex, numOfVars);
       LP_OPT = Rat_cost*Rat_solution; //cerr << cost << endl;
      cerr << "The LP optimal value is: " << holdcost_RR*ProjectingUpRR(ProjU_RR, Rat_solution, numOfVars) << endl;
      }else {cones = tmpcones;
    cerr << "\nThe polytope has " << lengthListCone(cones) << " vertices.\n";
    //system_with_error_check("rm -f numOfLatticePoints");
    cerr << endl;}
  } 

  /* Reading from the vertex representation. */

  if(Vrepresentation[0] == 'y')
     cones=computeVertexConesFromVrep(fileName,numOfVars); 

/* Compute triangulation or decomposition of each vertex cone. */

  if (dualApproach[0]=='y') {
    cones=createListCone();
    cones->vertex = new Vertex(createRationalVector(numOfVars));
    rays=createListVector(createVector(numOfVars));
    endRays=rays;
    tmpRays=matrix;
    while (tmpRays) {
      v=createVector(numOfVars);
      for (i=0; i<numOfVars; i++) v[i]=-(tmpRays->first)[i+1];
      endRays->rest=createListVector(v);
      endRays=endRays->rest;
      tmpRays=tmpRays->rest;
    }
    cones->rays=rays->rest;


	if (Memory_Save[0] == 'n' )
	{
	  cones=decomposeCones(cones,numOfVars, flags, fileName,
			       1, false,
			       BarvinokParameters::DualDecomposition); 
  	}
	else
	  {
	    decomposeCones_Single(cones, numOfVars, degree, flags, fileName,
				  1, false,
				  BarvinokParameters::DualDecomposition);
	}

 } 
  
   else 
   {

    if (assumeUnimodularCones[0]=='n') {
      if (decompose[0]=='y') 
      {
	if(Memory_Save[0] == 'n')      
	  cones=decomposeCones(cones,numOfVars, flags, fileName,
			       1, true,
			       BarvinokParameters::DualDecomposition);
	// Iterator through simplicial cones, DFS
	else
	  decomposeCones_Single(cones,numOfVars, degree, flags, fileName,
				1, true,
				BarvinokParameters::DualDecomposition);	
      }
    }
   }

/* Compute points in parallelepipeds, unless we already did using memsave version!  */


 if(Memory_Save[0] == 'n')
 {
	 cerr << "Computing the points in the Parallelepiped of the unimodular Cones." << endl;
  	tmp=cones;
	int	Cones_Processed_Count = 0;
 	while (tmp) 
	{
	  tmp->latticePoints = pointsInParallelepiped(tmp, numOfVars, &params);
	  tmp=tmp->rest;

		Cones_Processed_Count++;

		if ((Cones_Processed_Count % 1000) == 0 )
			cerr << Cones_Processed_Count << " cones processed." << endl;
  	}
}
  



  if(grobner[0] == 'y'){

 cones = ProjectUp(cones, oldnumofvars, numOfVars, templistVec);
 numOfVars = oldnumofvars;

  }
 if(Print[0] == 'y')
  printListCone(cones,numOfVars);

 if(inthull[0] == 'y')

    printListVector(IntegralHull(cones, matrix, numOfVars), numOfVars);

 
 if(maximum[0] == 'y') {
   listCone * Opt_cones;
   if(Singlecone[0] == 'n'){
   Opt_cones = CopyListCones(cones, numOfVars);
   ZZ NumOfLatticePoints; //printListCone(Opt_cones, numOfVars);
   NumOfLatticePoints = Residue(Opt_cones, numOfVars);
   cerr <<"Finished computing a rational function. " << endl;
   cerr <<"Time: " << GetTime() << " sec." << endl;
   if(IsZero(NumOfLatticePoints) == 1){
     cerr<<"Integrally empty polytope.  Check the right hand side."<< endl;
     exit(0);}
      else{
	int singleCone = 0;
	if(Singlecone[0] == 'y') singleCone = 1;
	vec_ZZ Opt_solution; 
	if(minimize[0] == 'y') holdCost = -holdCost;
	Opt_solution = SolveIP(cones, matrix, cost, numOfVars, singleCone); 
	cerr << "An optimal solution for " <<  holdCost << " is: " << ProjectingUp(ProjU, Opt_solution, numOfVars) << "." << endl;
	cerr << "The projected down opt value is: " << cost * Opt_solution << endl;
	cerr <<"The optimal value is: " << holdCost * ProjectingUp(ProjU, Opt_solution, numOfVars) << "." << endl;
	ZZ IP_OPT; IP_OPT = cost*Opt_solution;
	RR tmp_RR;
	conv(tmp_RR, cost * Opt_solution);
	// cerr << cost * Opt_solution << endl;
	if(minimize[0] == 'y') LP_OPT = - LP_OPT;
	cerr <<"The gap is: "<< abs(tmp_RR - LP_OPT) << endl;
	cerr << "Computation done." << endl;
	cerr <<"Time: " << GetTime() << " sec." << endl;
        // strcpy(command,"rm -f ");
        // strcat(command,fileName);
        // strcat(command,".ext");
        // system_with_error_check(command);

        // strcpy(command,"rm -f ");
        // strcat(command,fileName);
        // strcat(command,".cdd");
        // system_with_error_check(command);

        // strcpy(command,"rm -f ");
        // strcat(command,fileName);
        // strcat(command,".ead");
        // system_with_error_check(command);

	// if(cddstyle[0] == 'n'){
        // strcpy(command,"rm -f ");
        // strcat(command,fileName);
        // system_with_error_check(command);
	// }
	exit(0);
      }
   }
   else{
	int singleCone = 0;
	if(Singlecone[0] == 'y') singleCone = 1;
	vec_ZZ Opt_solution; 
	if(minimize[0] == 'y') holdCost = -holdCost;
	Opt_solution = SolveIP(cones, matrix, cost, numOfVars, singleCone); 
	cerr << "An optimal solution for " <<  holdCost << " is: " << ProjectingUp(ProjU, Opt_solution, numOfVars) << "." << endl;
	cerr << "The projected down opt value is: " << cost * Opt_solution << endl;
	cerr <<"The optimal value is: " << holdCost * ProjectingUp(ProjU, Opt_solution, numOfVars) << "." << endl;
	ZZ IP_OPT; IP_OPT = cost*Opt_solution;
	RR tmp_RR;
	conv(tmp_RR, cost * Opt_solution);
	// cerr << cost * Opt_solution << endl;
	if(minimize[0] == 'y') LP_OPT = - LP_OPT;
	cerr <<"The gap is: "<< abs(tmp_RR - LP_OPT) << endl;
	cerr << "Computation done." << endl;
	cerr <<"Time: " << GetTime() << " sec." << endl;
	// strcpy(command,"rm -f ");
	// strcat(command,fileName);
	// strcat(command,".ext");
	// system_with_error_check(command);

	// strcpy(command,"rm -f ");
	// strcat(command,fileName);
	// strcat(command,".cdd");
	// system_with_error_check(command);
	
	// strcpy(command,"rm -f ");
	// strcat(command,fileName);
	// strcat(command,".ead");
	// system_with_error_check(command);

	// if(cddstyle[0] == 'n'){
	// strcpy(command,"rm -f ");
	// strcat(command,fileName);
	// system_with_error_check(command);
	// }

	exit(0);
   }
 }else{
if(Memory_Save[0] == 'n')
{

	if(dualApproach[0] == 'n'){
	  cerr << "Creating generating function.\n"; 
	  //printListVector(templistVec, oldnumofvars); cerr << ProjU << endl;
	if(equationsPresent[0] == 'y'){ cones = ProjectUp(cones, oldnumofvars, numOfVars, templistVec);
	numOfVars = oldnumofvars;}

	  createGeneratingFunctionAsMapleInput(fileName,cones,numOfVars);  }
        //printListCone(cones, numOfVars);
	if(dualApproach[0] == 'n'){
	cerr << "Starting final computation.\n";
	cerr << endl << "****  The number of lattice points is: " << Residue(cones,numOfVars) << "  ****" << endl << endl;}


	if(dualApproach[0] == 'y')
	{
		cerr << "Starting final computation.\n";
		//cerr << "output_cone: " << output_cone;
  		ResidueFunction(cones,numOfVars, print_flag, degree, output_cone,
				&params);
	//  Else we have already computed the residue.
	}

}
 }

 if(rationalCone[0] == 'y') {
   string new_name = string(argv[argc - 1]) + ".rat";
   rename_with_error_check("simplify.sum", new_name);
 }

 if(printfile[0] == 'y'){
   string new_name = string(argv[argc - 1]) + ".rat";
   rename_with_error_check("func.rat", new_name);
 }
 if(removeFiles[0] == 'y'){
   
  // strcpy(command,"rm -f ");
  // strcat(command,fileName);
  // strcat(command,".ext");
  // system_with_error_check(command);
  
  // strcpy(command,"rm -f ");
  // strcat(command,fileName);
  // strcat(command,".cdd");
  // system_with_error_check(command); 
  
  // if(Memory_Save[0] == 'n'){
  //   strcpy(command,"rm -f ");
  //   strcat(command,fileName);
  //   strcat(command,".maple");
  //   system_with_error_check(command); 
  // }

  // strcpy(command,"rm -f ");
  // strcat(command,fileName);
  // strcat(command,".ead");
  // system_with_error_check(command); 

  // strcpy(command,"rm -f ");
  // strcat(command,fileName);
  // system_with_error_check(command);
  
 }
  }
 cerr << "Computation done. " << endl;
 cerr << "Time: " << GetTime() << " sec\n\n";
 
 return(0);
}
/* ----------------------------------------------------------------- */



