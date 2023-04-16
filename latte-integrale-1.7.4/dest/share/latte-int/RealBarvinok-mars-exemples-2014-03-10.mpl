with(linalg):with(LinearAlgebra):with(combinat):
kernelopts(assertlevel=1):       ### Enable checking ASSERTions

## Define this variable to have the code check self-tests (some expensive)
## (this is for "make check", not for production code).
#
# CHECK_EXAMPLES := true:

### PROGRAM ACCOMPANYING THE ARTICLE "THREE EHRHART POLYNOMIALS".
###
### Version info: $Date$ $Revision$ 

# procedures principales:
#
# fullbarvinok(t,[[0,0],[0,1],[1,1]],2,[1,1],0);
#
# SLsimplex(t,[[-1/3, 0, 0], [0, 1, 0], [0, 0, 1],[0, 0, 0]],[[1,1,1]],[x,y,z],0); 

#read("Conebyconeapproximations_08_11_2010.mpl"):
$include "Conebyconeapproximations_08_11_2010.mpl";

# Find out whether we are to check examples.
check_examples:=proc()
    type(CHECK_EXAMPLES, boolean) and CHECK_EXAMPLES;
end:

# Like ASSERT, but with better reporting (for automatic tests).
# FIXME: Use it in all test cases instead of ASSERT.
TEST_EQUAL:=proc(a_expr, b_expr, message) local a, b;
    printf("Checking %s\n", message);
    a := eval(parse(a_expr));
    b := eval(parse(b_expr));
    if not (a = b) then
        printf("FAIL: Results differ:\n  Got:       %a\n  Should be: %a\n", a, b);
    fi:
end:

# Helper functions to write ASSERT statements.  FIXME: Update to emit
# TEST_EQUAL instead.
print_assertion:=proc(expression, funcname, testnum)
    printf("    ASSERT(%s\n           = %a,\n           \"%s test #%a\");\n", 
           expression, eval(parse(expression)), funcname, testnum);
end:
print_assertions:=proc(expression_strings, funcname) local i;
    printf("if check_examples() then\n");
    for i from 1 to nops(expression_strings) do
        print_assertion(expression_strings[i], funcname, i);
    od:
    printf("fi:\n\n");
end:

#The output is the Complement  List, within the list [a[1],..,a[d]]
GeneralComplementList:=proc(K,L)local d;d:=nops(L); #NEW
    RETURN([seq (`if` (member(L[i],K)=false, L[i], op({})),i=1..d)]);
end:
if check_examples() then
    ASSERT(GeneralComplementList([2,3],[1,2,3,7])
           = [1, 7],
           "GeneralComplementList test #1");
fi:

# Input: a a list of lenght n, , v a list of n vectors, n an integer:
# Output: a list of lenght n;
# 
# Math: we compute the vector V:= sum_i a_i v[i];
# Example:   
# 
# Same program;
# but we restrict where the list v is not empty.
# 
# 
lincomb_v:=proc(a,v) #NEW
    ASSERT(nops(a)=nops(v) and nops(v)>=1," the number of coefficients and vectors do not match");
    [seq(add(a[i]*v[i][j],i=1..nops(v)),j=1..nops(v[1]))];
end:
if check_examples() then
    ASSERT(lincomb_v([1,1],[[1,0],[0,1]],2)
           = [1,1], 
           "lincomb_v test #1");
fi:

# Input: an integer N and sigma a list of vectors in R^d:
# Output: a vector of lenght d:
#
# Math: the vector is  sum_i x_i sigma_i, where the x_i are randomly chosen with coordiantes between 1 and N:
# 
cone_random_vector:=proc(N,sigma) local R,d,randcoeff; #NEW
    R:=rand(N);
    d:=nops(sigma[1]);
    randcoeff:=random_vector(N,nops(sigma));
    [seq(add(randcoeff[i]*sigma[i][j],i=1..nops(sigma)),j=1..d)]:
end:
#cone_random_vector(10,[[sigma[1],sigma[2]],[nu[1],nu


#  Input:  L a subspace: list of  s vectors in R^d; The codimension 
# Output: a list  of
#   k=d-s vectors   in R^d ;
# Math: A basis  H_1,H_2,...,H_k of the space L^{perp};

basis_L_perp:=proc(L) local d,s,ML,VV,res; #NEW
    d:=nops(L[1]); s:=nops(L);
    ML:=Matrix(L): VV:=NullSpace(ML);
    res := [seq(convert(VV[i],list),i=1..d-s)];
    res;
end:
if check_examples() then
    ASSERT(sort(basis_L_perp([[1,1,1]]))
           = sort([[-1, 0, 1], [-1, 1, 0]]),
           "basis_L_perp test #1");
fi:

# Input: W a list of vectors in R^d;
# L a list of vectors  in R^d of lenght s;
# Oputput: a list of vectors in R^k; with k=d-s;
# 
# Math: we project the vectors in X in Lperp; with  basis H1,H2,...,Hk;
# Our list  is the list of the projections of the elements of W written in the basis Hk (computed by basis_L_perp(L) as a list):
# 

Oursmallmatrix:=proc(W,L) local s,d,HH,i,C,M,wbars,j,VV,cW; #NEW
    d:=nops(W[1]); s:=nops(L);
    C:=[];
    HH:=basis_L_perp(L);
    for i from 1 to d-s do 
        C:=[op(C),[seq(add(HH[i][k]*HH[j][k],k=1..d),j=1..(d-s))]];
    od;
    M:=Transpose(Matrix(C));
    wbars:=[];
    for j from 1 to nops(W) do
        VV:=Vector([seq(add(W[j][k]*HH[i][k],k=1..d),i=1..(d-s))]);
        cW:=convert(LinearSolve(M,VV),list); 
        wbars:=[op(wbars),cW];
    od;
    wbars;
end:
if check_examples() then
    ASSERT(sort(Oursmallmatrix([[1,0,0],[0,1,0],[0,0,1]],[[1,1,1]]))
           = sort([[-1/3, -1/3], [-1/3, 2/3], [2/3, -1/3]]),
           "Oursmallmatrix test #1");
    # compare with basis_L_perp([[1,1,1]]);
fi:

# Input: a list of N vectors in R^k;
# output; a list. Each element of the list is  [B,B_c,D,edges].
# Here B is a subset of [1,...,N]; B_c is the complement subset in [1,...,N];
# and D is a Matrix, edges is a list of vectors in R^k.
# Maths; 
# The list is [a_1,a_2,..., a_N]; 
# B is a subset of   B is a subset of [1,...,N] such that a_i, i in B are linearly independent.
# Then we express w_j with j in B_c as a vector with respect to the basis (w_i,i in B). 
# D is the matrix, edges is the list of colums of this matrix; This is a redundant information, but I kept this because I had problems in converting in lists  using only D in the next procedure after.
# # This procedure should be done using reverse search.
# 
# 
AllDictionaries:=proc(Listvectors) local LL, N,k,K,h,MM,Kc,bandc,Matrix2,Dict,edges,newbc;
    LL:=Listvectors;
    N:=nops(LL); 
    k:=nops(LL[1]);
    K:=choose(N,k); 
    bandc:=[];
    for h from 1 to nops(K) do
        MM:=Matrix([seq(LL[K[h][i]],i=1..k)]);
        if Rank(MM)=k then 
            Kc:=ComplementList(K[h],N); 
            Matrix2:=Transpose(Matrix([seq(LL[Kc[i]],i=1..nops(Kc))]));
            Dict:=MatrixMatrixMultiply(MatrixInverse(Transpose(MM)),Matrix2);
            edges:=[seq(convert(Column(Dict,u),list),u=1..N-k)];
            newbc:=[K[h],Kc,Dict,edges];
            bandc:=[op(bandc),newbc];
        else bandc:=bandc;
        fi;
    od;
    bandc;
end:
if check_examples() then
    # two Matrix objects do not compare "=".  So instead of trying to
    # use LinearAlgebra[Equal] for comparing them, we just compare
    # their print representations.
    ASSERT(sprintf("%a", AllDictionaries([[1,0],[1,0],[1,1]]))
           = sprintf("%a", [[[1, 3], [2], Matrix(2, 1, [[1],[0]]), [[1, 0]]], 
                            [[2, 3], [1], Matrix(2, 1, [[1],[0]]), [[1, 0]]]]),
           "AllDictionaries test #1");
fi;

# Input:= a list of N vectors in R^k
# Ouput:  a vector in R^k;
# Math: we compute a  vector a which is not on any hyperplane generated by some of the vectors in the list.
# Furthermore, we choose it to be in the cone generated by the list of vectors.
# 
# I did not do the choice of the "deterministic" regular vector.
# Example: randomRegularpositivevector([[1,0],[0,1],[1,1]])->[5,10];
randomRegularpositivevector:=proc(Listvectors) local LL,t,ok,w,K,f,MM,N,k,indexsigma,sigma; #NEW
    LL:=Listvectors;
    N:=nops(LL);
    k:=nops(LL[1]);
    indexsigma:=AllDictionaries(Listvectors)[1][1];
    sigma:=[seq(LL[indexsigma[i]],i=1..k)]; ###print("sigma",sigma); 
    K:=choose(N,k-1);
    t:=1; ok:=0;
    while t<=10 do 
        w:=cone_random_vector(10*t,sigma);
        f:=1; 
        while f<=nops(K) do
            MM:=Matrix([seq(LL[K[f][i]],i=1..k-1),w]);
            if Determinant(MM)<>0 
            then 
                f:=f+1;
                ok:=ok:
                ##print('ok,f',ok);
            else f:=nops(K)+1;
                ok:=ok+1;
            fi:od;
        if ok=0 then t:=11;
        else
            t:=t+1;ok:=0;
        fi;
    od:
    w;
end:
if check_examples() then
    randomRegularpositivevector([[0,0],[0,-1],[-1,-1]]): # only checks that it runs without error. output is nondeterministic.
fi:

# 
# 
# 
# Input:= a list of N vectors in R^k
# Ouput:  a list of lists: each list  is of the form [[sigma,epsilon, listsigns], [complementofsigma,A]];
# sigma is a subset of  [1,2,...,N], epsilon is a sign, listsigns is a sequence of k elements epsilon_i
# with epsilon_i a sign. complement of sigma is a subset of [1...N] (the complement of sigma),
# A is a list of N-k vectors in R^k.
# Math; sigma, epsilon will lead to generators epsilon_i w_i (i in sigma), the complement of sigma will lead to the generators 
# w_j-a_j^i w_i with j not in sigma. 
# 
coeff_cone_dec:=proc(Listvectors) local N,k,w,out,LL,sigma,M1,cw,signswonsigma,edges,coeffs,i; #NEW
    LL:=Listvectors; 
    N:=nops(LL);
    k:=nops(LL[1]);
    coeffs:=[];
    w:=randomRegularpositivevector(LL); ##print("randvector",w);
    out:= AllDictionaries(LL);
    for i from 1 to nops(out) do
        sigma:=out[i][1]; ##print(sigma);
        M1:=Matrix([seq(LL[sigma[u]],u=1..nops(sigma))]);
        cw:=convert(LinearSolve(Transpose(M1),Vector(w)),list);
        signswonsigma:=[sigma, mul(sign(cw[j]),j=1..k),[seq(sign(cw[j]),j=1..k)]];
        coeffs:=[op(coeffs),[signswonsigma,[out[i][2],out[i][4]]]];
    od;
    coeffs;
end:
if check_examples() then
    coeff_cone_dec([[1,0],[0,1],[1,1]]):  # output is nondeterministic.
    ASSERT(op(1,coeff_cone_dec([[1,0],[0,1],[1,1]]))
           = [[[1, 2], 1, [1, 1]], [[3], [[1, 1]]]], # this one SEEMS to be deterministic.
           "coeff_cone_dec test #1");
fi:

# FINALLY: 
# 
# Input: W a list of d vectors in R^d; L a list of vectors in R^d;
# Output: a list of  signed cones in R^d
# 
# Math: W represents a cone, we express the characteristic function of C as a  signed sum of cones C_a
# where the cones C_a have the following property:
# If d=k+dim(L) these cones C_a  have d-k generators in L and k other generators in W. 
# 
L_cone_dec:=proc(W,L) local d,LL,coeffs,conedec,i,generatorsonL,coe1,coe2,generatorssigma,u,a,vL,g,conesigma; #NEW
    if L=[] 
    or nops(W)=Rank(Matrix(L)) then
        RETURN([[1,W]]);
    fi;
    d:=nops(W[1]);
    LL:=Oursmallmatrix(W,L); ##print(LL); 
    coeffs:=coeff_cone_dec(LL);
    conedec:=[];
    for i from 1  to nops(coeffs) do
        generatorsonL:=[];  
        coe1:=coeffs[i][1];
        coe2:=coeffs[i][2]; 
        generatorssigma:=[seq(coe1[3][i]*primitive_vector(W[coe1[1][i]]),i=1..nops(coe1[1]))];
        for u from 1 to nops(coe2[2]) do 
            a:=[1,op(coe2[2][u])];
            vL:=[W[coe2[1][u]],seq(-W[coe1[1][i]],i=1..nops(coe1[1]))];
            g:=special_lincomb_v(a,vL,d);  
            generatorsonL:=[op(generatorsonL),primitive_vector(g)];
        od;
        conesigma:=[coe1[2],[op(generatorssigma),op(generatorsonL)]]; 
        conedec:=[op(conedec),conesigma];
    od;
end:
if check_examples() then
    ASSERT(L_cone_dec([[1,0],[0,1]],[[1,1]])
           = [[1, [[1, 0], [1, 1]]], [-1, [[0, -1], [1, 1]]]],
           "L_cone_dec test #1");
fi:

# Projections:  

# Input: s a vector in R^d with rational coordinates (or symbolic).
# W a cone in Z^d
# Output:  a vector in R^d
# 
# Math: We decompose V in lin(II) oplus lin (II_c), and here we write s=s_II+s_(II_c): Here the output is s_(II_c);

s_IIc:=proc(s,W,II) local DD,IIc,M,s_in_cone_coord,s_IIc; # NEW - note ConebyCone has the other function - s_Ispace
    DD:=[seq(i,i=1..nops(W))];
    IIc:=GeneralComplementList(II,DD);
    M:=Matrix([seq(Vector([W[i]]),i=1..nops(W))]);
    s_in_cone_coord:=convert(LinearSolve(M,Vector(s)),list);
    s_IIc:=[seq(s_in_cone_coord[IIc[k]],k=1..nops(IIc))];
    special_lincomb_v(s_IIc,[seq(W[IIc[k]],k=1..nops(IIc))],nops(W));
end:
if check_examples() then
    ASSERT(s_IIc([s1,s2],[[1,0],[0,1]],[1]) 
           = [0, s2],
           "s_IIc test #1");
fi:

# Relative volume
# 
# 
# 
# Input: W is a Cone in R^d and II is a subset of [1,..,d] of cardinal k;
# Ouput: a number;
# 
# Math; the volume of the Box(v[i], i not in II), with respect to the intersected lattice.
# Example: relativevolumeoffaceIIc([[1,1],[0,1]],[1])->1;  
# 
relativevolumeoffaceIIc:=proc(W,II) local DD,IIc,P,M,H,MM,output; # called volume_Ispace in Conebycone...
DD:=[seq(i,i=1..nops(W))]; 
IIc:=GeneralComplementList(II,DD);
if IIc=[] then output:=1; 
else P:=matrix([seq(W[IIc[i]],i=1..nops(IIc))]);
  M:=transpose(matrix(P));
   H:=ihermite(M); 
   MM:=matrix([seq(row(H,i),i=1..nops(IIc))]);
 output:=det(MM);
fi;
 output;
end:
#relativevolumeoffaceIIc([[1,0],[0,1]],[1]); 

# The 2 functions to compute S_L
# Input: s a vector in R^d;  W a "Cone" in R^d; II a subset of [1, 2,...,d];
# x a variable:

# Output: a list of two functions of x;
# Math: #We compute integral over the cone IIc of 
# exp^(csi,x) ; the answer is given as [exp (<q,x>, product of linear forms]
# Representing separately the numerator and the denominator.
# Furthermore, we enter exp as a "black box" EXP(x); later on we might want to replace it.
# Example functionI([1/2,1/2],[[1,0],[0,1]],[],x) `invalid character in short integer encoding 17 `;;
# 
# 
functionI:=proc(s,W,II,x) # called functionIb in Conebycone...
    local s_on_IIc,DD,IIc,d,T,i,y,r,out;
    d:=nops(W);
    DD:=[seq(i,i=1..d)]; 
    s_on_IIc:=s_IIc(s,W,II);
    if nops(II)=nops(W) 
    then out:=[1,1]; 
    else
    IIc:=GeneralComplementList(II,DD);
    r:=relativevolumeoffaceIIc(W,II);
    T:=1;
    for i from 1 to nops(IIc) do 
    y:=add(W[IIc[i]][j]*x[j],j=1..d);
    T:=T*y;
    od;
    T:=(-1)^(nops(IIc))*T;
    out:=[r*EXP(add(s_on_IIc[m]*x[m],m=1..d)),T];
    fi; 
    out;
end: 

# THE FUNCTION S_L for a cone.
#
# This function uses Brion-Vergne decomposition (L_cone_dec).
#
# Input: s,list,W a list of list,L a list of lists,x a list of variables.
# Output:= a  function 
# Math: the function SL

# FIXME: Should maybe be using our formal version of ceil.

function_SL:=proc(s,W,L,x)  #NEW
    local DD,i,parallel_cones,uni_cones,function_on_II,function_on_IIc,WW_projected,WW,WWW,signuni,signL,j,II,IIc,out1,out2,s_in_cone_coord,s_II_in_cone_coord,s_prime_II,M,newx,dimL,g,testrank,newP,
    s_II_in_lattice_coord,news,ProjLattice;
    DD:=[seq(i,i=1..nops(W))];
    #I added this
    if L=W then RETURN(functionI(s,W,[],x)[1]/functionI(s,W,[],x)[2]);fi;
    #up to here
    parallel_cones:=L_cone_dec(W,L);#print("parallel",parallel_cones);
    out2:=0;
    for i from 1 to nops(parallel_cones) do
        WW:=parallel_cones[i][2];
        signL:=parallel_cones[i][1];
        IIc:=[];
        II:=[];#print(Matrix(L));
        dimL:=Rank(Matrix(L));
        for g from 1 to nops(WW) do 
            testrank:=Rank(Matrix([op(L),WW[g]])); 
            if testrank=dimL 
            then IIc:=[op(IIc),g]
            else II:=[op(II),g];
            fi:
        od; 
        ASSERT(nops(IIc)=dimL,"decompositioninL_parallel is wrong");
        M:=Matrix([seq(Vector(WW[h]),h=1..nops(WW))]);
        ProjLattice := projectedlattice(WW,II);
        s_II_in_lattice_coord:=projectedvertexinbasislattice(WW,II,ProjLattice,s);  
        function_on_IIc:=functionI(s,WW,II,x);
        #from here express in terms of the basis lattice for projected cone.

        WW_projected:=projectedconeinbasislattice(WW,II,ProjLattice):
        #print(WW_projected);

        if WW_projected=[] then 
            out1:=1 
        else
            newx:=changeofcoordinates(WW,II,ProjLattice,x);
            uni_cones:=cone_dec(WW_projected):
            out1:=0;
            for j from 1 to nops(uni_cones) do 
                WWW:=uni_cones[j][3];
                signuni:=uni_cones[j][1];
                ASSERT(abs(uni_cones[j][2])=1, "decomposition not unimodular");
                newP:=MatrixInverse(Transpose(Matrix(WWW))):
                news:=convert(Multiply(newP,Vector(s_II_in_lattice_coord)),list);
                s_prime_II:=[seq(ceil(news[f]),f=1..nops(news))];
                function_on_II:=functionS(s_prime_II,WWW,newx);
                out1:=out1+
                signuni*function_on_II[1]/function_on_II[2];
            od:
        fi;
        out2:=out2+out1*function_on_IIc[1]/function_on_IIc[2]*signL;
    od:
    out2;
end:
if check_examples() then
    TEST_EQUAL("function_SL([1/2,0,0],[[-1,0,1],[-1,2,0],[0,0,1]],[[-1,2,0]],[x1,x2,x3])",
               "(2*TODD(1, (1/2)*x1)*TODD(0, -x1+x3)/(x1*(-x1+x3))-2*TODD(0, x3)*TODD(-1, -(1/2)*x1)/(x3*x1))*EXP(0)/(x1-2*x2)",
               "function_SL test #1");
    TEST_EQUAL("function_SL([0,0,0],[[1,0,0],[0,2,1],[0,0,1]],[[1,1,1]],[x1,x2,x3])",
               " TODD(0,x1)*TODD(0,2*x2+x3)/x1/(2*x2+x3)*EXP(0)/(-x1-x2-x3)\
                +TODD(0,-x1)*TODD(0,-x3)/x1/x3*EXP(0)/(-x1-x2-x3)\
                -(-TODD(0,x3)*TODD(0,x2)/x3/x2+TODD(0,2*x2+x3)*TODD(0,x2)/(2*x2+x3)/x2)*EXP(0)/(-x1-x2-x3)",
               "function_SL test #2");
fi;


# Input: W a cone, L a linear space,x a variable.
# Output: a list of linear forms.
#  Math: this is  the forms in denominator of the big func.
linindenom:=proc(W,L,x) # Conebycone has function of same name with different interface, implementation
    local YY,i,
    parallel_cones,VDD,IIc,II,dimL,g,testrank,WW,newx,d,a,z,cc,
    WW_projected,uni_cones,t,cleanYY,r,ProjLattice;
    VDD:=[seq(i,i=1..nops(W))];
    d:=nops(W);
    parallel_cones:=L_cone_dec(W,L);
    YY:={};
    for i from 1 to nops(parallel_cones) do
        WW:=parallel_cones[i][2];# We should know I, II
        IIc:=[];
        II:=[];
        dimL:=Rank(Matrix(L));
        for g from 1 to nops(WW) do 
            testrank:=Rank(Matrix([op(L),WW[g]])); 
            if testrank=dimL 
            then IIc:=[op(IIc),g];
            else II:=[op(II),g];
            fi:
        od; 
        ASSERT(nops(IIc)=dimL,"decompositioninL_parallel is wrong");
        for a from 1 to nops(IIc) do
            YY:={op(YY),add(WW[IIc[a]][j]*x[j],j=1..d)};
        od;
        ProjLattice := projectedlattice(WW,II);
        WW_projected:=projectedconeinbasislattice(WW,II,ProjLattice):
        newx:=changeofcoordinates(WW,II,ProjLattice,x); 
        ##print("newx,WW_projected",newx,WW_projected);
        uni_cones:=cone_dec(WW_projected):
        ##print("i,unicones,newx",i,uni_cones,newx,YY);
        for z from 1 to nops(uni_cones) do
            cc:=uni_cones[z][3]; 
            for t from 1 to nops(cc) do
                YY:={op(YY), add(cc[t][s]*newx[s],s=1..nops(newx))}
            od;
        od;
    od;
    ##print(YY);
    cleanYY:={};
    for r from 1 to nops(YY) do
        if member(-YY[r],cleanYY)=false then
            cleanYY:={op(cleanYY),YY[r]}
        fi;
    od;
    cleanYY;
end:

# The Valuation S_L for a simplex.
# Input: S a list of a list, L a list of list, x a list of variables
# Output: a function
# Math: the function SL as  above computed for a simplex
function_SL_simplex:=proc(S,L,x) local F,W,i; #NEW
    F:=0;
    for i from 1 to nops(S) do 
        W:=[seq(primitive_vector(S[j]-S[i]),j=1..i-1),seq(primitive_vector(S[j]-S[i]),j=i+1..nops(S))];
        F:=F+function_SL(S[i],W,L,x);
    od:
    F:
end:
if check_examples() then
    ASSERT(function_SL_simplex([[0,0,0],[2,0,0],[0,2,0],[0,0,2]],[[1,0,0],[0,1,0]],[x1,x2,x3])
           = 
           TODD(0, x3)*EXP(0)/(x3*x1*x2)
           -TODD(0, -x1+x3)*EXP(2*x1)/((-x1+x3)*x1*(-x1+x2))
           -TODD(0, -x2+x3)*EXP(2*x2)/((-x2+x3)*x2*(x1-x2))
           -TODD(-2, -x3)*EXP(0)/(x3*x1*x2)
           -TODD(-2, x1-x3)*EXP(2*x1)/((x1-x3)*x1*(-x1+x2))
           -TODD(-2, x2-x3)*EXP(2*x2)/((x2-x3)*x2*(x1-x2)),
           "function_SL_simplex test #1");
    betedim1:=subs({TODD=Todd,EXP=exp},function_SL_simplex([[0],[1]],[],[x1])):
    ASSERT(betedim1 = 1/(1-exp(x1))+exp(x1)/(1-exp(-x1)),
           "function_SL_simplex test #2");
    Sbete2:=[[0,0+2/10],[1,2/10],[0,2+2/10]]:
    check2:=subs({TODD=Todd,EXP=exp},function_SL_simplex(Sbete2,[[1,0]],[x1,x2])):
    ASSERT(check2
           = 
           -exp(x2)/(1-exp(x2))/x1
           +exp(-1/2*x1+x2)/(1-exp(-1/2*x1+x2))*exp(11/10*x1)/x1-exp(2*x2)/(1-exp(-x2))/x1
           +exp(-x1+2*x2)/(1-exp(1/2*x1-x2))*exp(11/10*x1)/x1,
           "function_SL_simplex test #3");
    ASSERT(series(subs({x1=t, x2=2*t}, check2), t=0)
           = series(7/10+357/200*t+14497/6000*t^2+O(t^3),t,3),
           "function_SL_simplex test #4");
    #SA:=[[0,0],[0,4/10],[1,1]]; SB:=[[0,4/10],[1,1],[1,1+4/10]];
    check3:=subs({TODD=Todd,EXP=exp},function_SL_simplex([[0,0,0],[2,0,0],[0,2,0],[0,0,2]],[[1,0,0],[0,1,0]],[x1,x2,x3])):
    SERT:=subs({x1=t,x2=5*t,x3=17*t},check3):
    ASSERT(series(SERT,t=0,20)
           = series(5/2+35/2*t+2669/24*t^2+24751/40*t^3+2043437/720*t^4+860803/80*t^5+1388954957/40320*t^6+1644819689/17280*t^7+839078112749/3628800*t^8+606308842781/1209600*t^9+94023346728169/95800320*t^10+888577719521/506880*t^11+251068287087746861/87178291200*t^12+272851326521580617/62270208000*t^13+11812811856176186503/1902071808000*t^14+57479858988540487151/6974263296000*t^15+O(t^16),t,16),
           "function_SL_simplex test #5");
fi:


# Regular vector

# Input, t a variable, alpha= a list of lenght k of  linear forms l_i of  x1,x2,...,x_n,
#   n a number ; 
# The ouptut is a vector v such that  l_i(v) is not zero for all i
# 
regular:=proc(t,alpha,n) #NEW
    local ok,p,i,pp,v,j,s,deg,newP,P,PP,out:
    ok:=0;
    P:=1;
    for i from 1 to nops(alpha) do
        P:=P*alpha[i];
    od;
    v:=[seq(t^i,i=0..n-1)];
    for j from 1 to n do 
        P:=subs(x[j]=t^(j-1),P);
    od:
    deg:=degree(P);
    s:=1; 
    while ok=0 and s<=deg+1 do
        newP:=subs(t=s,P); ##print(newP);
        if newP<>0 
        then out:=[v,s,subs(t=s,v)];
            ok:=1;
        else s:=s+1;
        fi;
    od:
    out[3];
end:
if check_examples() then
    ASSERT(regular(t,{-x[1],-x[1]+x[2]},2)
           = [1, 2],
           "regular test #1");
fi:


denomWL:=proc(W,L) local xx,alpha; #NEW
    xx:=[seq(x[i],i=1..nops(W))];
    alpha:=linindenom(W,L,xx); #print("alpha",alpha);
end:
if check_examples() then
    ASSERT(denomWL([[1,-1,0],[0,2,1],[0,0,1]],[[1,1,1]])
           = {x[3], -x[2], x[1]-x[2], 2*x[2]+x[3], x[1]+x[2]+x[3], 1/2*x[1]-1/2*x[2]+1/2*x[3]},
           "denomWL test #1");
fi:


# DEFORMATION OF FUNCTION S_L
# 

# We want to compute SL at xi=ell^M.To do this we need to deform 
#defSLell([1/2,1/2],[[1,0],[0,1]],[[1,-1]], [x1,x2],[3,2]);
# The input is a  s a vertex,W a cone, L a linear space , ell a list with numeric coefficients.
# The output is a function of delta, epsilon;
# Math: we substitute xi[j]=delta*(ell[j]+epsilon*reg[j]) in the function SL
# 
# FIXME: delta, epsilon should be parameters or at least quoted.
#
defSLell:=proc(s,W,L,ell,reg) local xx,defell,ff; #NEW
    #reg:=regularWL(W,L); #print("reg",reg);
    xx:=[seq(x[i],i=1..nops(W))];
    defell:=[seq(delta*(ell[j]+epsilon*reg[j]),j=1..nops(W))];
    ff:=function_SL(s,W,L,defell);
    ##print(ff,defell);
    ff:=eval(subs({TODD=Todd,EXP=exp},ff));
    ff;
end:
if check_examples() then
    ASSERT(defSLell([1/2,1/2],[[1,0],[0,1]],[[1,-1]], [x1,x2],[3,2])
           = 
           exp(delta*(x1+3*epsilon))*exp(-(1/2)*delta*(x1+3*epsilon)+(1/2)*delta*(x2+2*epsilon))/((1-exp(delta*(x1+3*epsilon)))*(delta*(x1+3*epsilon)-delta*(x2+2*epsilon)))
           +exp(delta*(x2+2*epsilon))*exp((1/2)*delta*(x1+3*epsilon)-(1/2)*delta*(x2+2*epsilon))/((1-exp(delta*(x2+2*epsilon)))*(-delta*(x1+3*epsilon)+delta*(x2+2*epsilon))),
           "defSLell test #1");
    #series(SLell([1/2,1/2],[[1,0],[1,2]],[[1,1]], [1,-1]),delta=0,3);
    #function_SL([1/2,1/2],[[1,0],[0,1]],[[1,-1]], [x1,x2]);
fi:

deftruncatedSL:=proc(s,W,L,ell,reg,M) local SS,cc; #NEW
    #SS:=subs({epsilon=0},SLell(s,W,L,ell,reg)); #print(SLell(s,W,L,ell));
    ##print("SS",SS);
    ##print(series(SS,delta=0,M+nops(W)+2));
    cc:=convert(series(defSLell(s,W,L,ell,reg),delta=0,M+nops(W)+2),polynom);
    coeff(series(cc,epsilon=0,nops(W)+2),epsilon,0);
end:
if check_examples() then
    ASSERT(deftruncatedSL([1/2,1/2],[[1,0],[1,2]],[[1,1]],[1,1],[3,7],4)
           = 
           2/3/delta^2+2/3/delta+1/2+5/18*delta+1/72*delta^3+11/5040*delta^4+11/120*delta^2, 
           "deftruncatedSL test #1");
fi:

# The function S_L for a simplex: we want to compute  function S_L for a simplex on the linear form ell.
regularSL:=proc(S,L) local i,W,reg,xx,alpha; #NEW
    xx:=[seq(x[i],i=1..nops(S)-1)];
    alpha:={};
    for i from 1 to nops(S) do 
        W:=[seq(primitive_vector(S[j]-S[i]),j=1..i-1),seq(primitive_vector(S[j]-S[i]),j=i+1..nops(S))];
        alpha:={op(alpha),op(denomWL(W,L,xx))};
    od:
    reg:=regular(t,alpha,nops(S)-1);
end:

function_SL_simplex_ell:=proc(S,L,ell,M) local reg,F,W,i;
    F:=0;
    reg:=regularSL(S,L);
    for i from 1 to nops(S) do 
        W:=[seq(primitive_vector(S[j]-S[i]),j=1..i-1),seq(primitive_vector(S[j]-S[i]),j=i+1..nops(S))];
        F:=F+deftruncatedSL(S[i],W,L,ell,reg,M);
    od:
    coeff(F,delta,M):
end:
if check_examples() then
    Sbete2:=[[0,0],[1,0],[0,1]]:
    ASSERT(function_SL_simplex_ell(Sbete2,[[1,0]],[0,0],0)
           = 1,
           "function_SL_simplex_ell test #1");
    # dilated standard simplex of dimension n, dilation t.
    Sbetedilated:=proc(n,t) local ze, S,j,zej; 
        ze:=[seq(0,i=1..n)];
        S:=[ze];
        for j from 1 to n do 
            zej:=subsop(j=t,ze);
            S:=[op(S),zej];
        od;
    end:
    ASSERT(Sbetedilated(3,2) = [[0, 0, 0], [2, 0, 0], [0, 2, 0], [0, 0, 2]],
           "Sbetedilated test #1");
    ASSERT(function_SL_simplex_ell(Sbetedilated(2,2),[[1,2]],[0,0],0)
           = 2, 
           "function_SL_simplex_ell test #2");
    S1:=[[0,0],[1,0],[1,2]]:
    ASSERT(function_SL_simplex_ell(S1,[[1,0]],[1,1],1)
           = 11/8,
           "function_SL_simplex_ell test #3");
fi:

#  Dilated polytope

## This function is semi-inert:  If the result would still be symbolic, keep
## it as the original expression "ourfrac(t)".  
## Call value() on the result to force expansion to floor().
## This may be necessary to allow full simplification.
ourfrac := proc(t) # NEW VERSION
    local our;
    our := t - floor(t);
    if type(our, numeric) 
    then our;
    else 'ourfrac'(t); # keep it symbolic (until the next "eval" (which feeds it back in here) or "value").
    fi:
end:
## The fully evaluated version of the above.
## Either use eval(subs({ourfrac=bonfrac}, ...)) or value(...).
bonfrac := t -> t - floor(t):
`value/ourfrac` := t -> t - floor(t):

if check_examples() then
    ASSERT(ourfrac('t') = 'ourfrac'('t'), "ourfrac test #1");
    ASSERT((ourfrac(t) assuming t::integer) = 0, "ourfrac test #2");
    ASSERT(ourfrac(1/3) = 1/3, "ourfrac test #3");
    ASSERT(ourfrac(-1/3) = 2/3, "ourfrac test #4");
    ASSERT(ourfrac(sqrt(2)) = 'ourfrac'(sqrt(2)), "ourfrac test #5");
    ASSERT(simplify(value(1/72*Pi^2+1/6*(-ourfrac(1/3*Pi)+3/2-ourfrac(-1/6*Pi))*Pi+1/2*ourfrac(-1/6*Pi)^2-3/2*ourfrac(1/3*Pi)-3/2*ourfrac(-1/6*Pi)+1/2*ourfrac(1/3*Pi)^2+1+ourfrac(-1/6*Pi)*ourfrac(1/3*Pi))) = 1, "ourfrac test #6");
    ASSERT(simplify(eval(subs({ourfrac=bonfrac}, 1/72*Pi^2+1/6*(-ourfrac(1/3*Pi)+3/2-ourfrac(-1/6*Pi))*Pi+1/2*ourfrac(-1/6*Pi)^2-3/2*ourfrac(1/3*Pi)-3/2*ourfrac(-1/6*Pi)+1/2*ourfrac(1/3*Pi)^2+1+ourfrac(-1/6*Pi)*ourfrac(1/3*Pi)))) = 1, "ourfrac test #7");
fi:

## Instead of using ourfrac(t), we now write MOD(t, 1), just as in
## Conebycone.  (Comment out this line if you want to see "ourfrac" in the
# output instead.)
ourfrac := t -> MOD(t, 1):

# We are dilating the cone by t and compute the function SL
# t is the variable that appears inside frac expressions.
# T is the variable that corresponds to polynomial degree.
tfunction_SL:=proc(t,T,s,W,L,x) local st,DD,i,parallel_cones,uni_cones,function_on_II,function_on_IIc,WW_projected,WW,WWW,signuni,signL,j,II,IIc,out1,out2,s_in_cone_coord,s_II_in_cone_coord,s_prime_II,M,newx,dimL,g,testrank,newP,
    s_II_in_lattice_coord,news,zerost,s_small_move,ProjLattice;
    DD:=[seq(i,i=1..nops(W))];
    parallel_cones:=L_cone_dec(W,L):
    ##print("parallel_cones",parallel_cones);
    out2:=0; #listgene:={};
    for i from 1 to nops(parallel_cones) do
        WW:=parallel_cones[i][2];
        signL:=parallel_cones[i][1];
        IIc:=[];
        II:=[];
        dimL:=Rank(Matrix(L));
        for g from 1 to nops(WW) do 
            testrank:=Rank(Matrix([op(L),WW[g]])); 
            if testrank=dimL 
            then IIc:=[op(IIc),g];
            else II:=[op(II),g];
            fi:
        od; 
        ASSERT(nops(IIc)=dimL,"decompositioninL_parallel is wrong");
        M:=Matrix([seq(Vector(WW[h]),h=1..nops(WW))]);
        ProjLattice := projectedlattice(WW,II);
        s_II_in_lattice_coord:=projectedvertexinbasislattice(WW,II,ProjLattice,s);
        #st:=[seq(t*s[i],i=1..nops(s))]; 
        zerost:=[seq(0,i=1..nops(s))];   
        function_on_IIc:=functionI(zerost,WW,II,x);
        #print("functionInt",function_on_IIc);
        #from here express in terms of the basis lattice for projected cone.
        WW_projected:=projectedconeinbasislattice(WW,II,ProjLattice):
        if WW_projected=[] then 
            out1:=1 else
            newx:=changeofcoordinates(WW,II,ProjLattice,x); 
            ##print("newx",newx);
            uni_cones:=cone_dec(WW_projected):
            ##print("unicones",WW,uni_cones);
            out1:=0;
            for j from 1 to nops(uni_cones) do 
                WWW:=uni_cones[j][3];
                signuni:=uni_cones[j][1];
                ASSERT(abs(uni_cones[j][2])=1, "decomposition not unimodular");
                newP:=MatrixInverse(Transpose(Matrix(WWW))):
                news:=convert(Multiply(newP,Vector(s_II_in_lattice_coord)),list);
                s_small_move:=[seq(ourfrac(t*(-numer(news[f])/denom(news[f]))),f=1..nops(news))];  #print("smallmove",s_small_move);
                function_on_II:=functionS(s_small_move,WWW,newx):
                ##print("function_on_II",function_on_II);
                out1:=out1+
                signuni*function_on_II[1]/function_on_II[2];
            od:
        fi;
        out2:=out2+out1*function_on_IIc[1]/function_on_IIc[2]*signL;
    od:
    EXP(add(T*s[i]*x[i],i=1..nops(W)))*out2;
end:
if check_examples() then
    ASSERT(tfunction_SL(t,T,[1/2,1/4,1/7],[[1,0,0],[0,1,0],[0,0,1]],[[1,0,0]],[x1,x2,x3])
           =
           -EXP(1/2*T*x1+1/4*T*x2+1/7*T*x3)*TODD(ourfrac(-1/4*t),x2)*TODD(ourfrac(-1/7*t),x3)/x2/x3*EXP(0)/x1,
           "tfunction_SL test #1");
    ASSERT(tfunction_SL(t,T,[1/2,1/2],[[0,1],[1,0]],[[1,0]],[x1,x2])
           =
           -EXP(1/2*T*x1+1/2*T*x2)*TODD(ourfrac(-1/2*t),x2)/x2*EXP(0)/x1,
           "tfunction_SL test #2");
fi:

# Input: W a cone, L a linear space,x a variable.
# Output: a list of linear forms.
#
# t is the variable that appears inside frac expressions.
# T is the variable that corresponds to polynomial degree.
tSLell:=proc(t,T,s,W,L,ell,reg) local xx,defell,ff;
    #reg:=regularWL(W,L); #print("reg",reg);
    xx:=[seq(x[i],i=1..nops(W))];
    defell:=[seq(delta*(ell[j]+epsilon*reg[j]),j=1..nops(W))];
    ff:=tfunction_SL(t,T,s,W,L,defell);
    #print(ff,defell);
    ff:=eval(subs({TODD=Todd,EXP=exp},ff)); #print(ff);
    ff;
end:
#function_SL([1/2,1/2],[[1,0],[0,1]],[[1,-1]], [x1,x2]);

# t is the variable that appears inside frac expressions.
# T is the variable that corresponds to polynomial degree.
ttruncatedSL:=proc(t,T,s,W,L,ell,reg,M) local SS,cc;
    cc:=convert(series(tSLell(t,T,s,W,L,ell,reg),delta=0,M+nops(W)+2),polynom);
    coeff(series(cc,epsilon=0,nops(W)+2),epsilon,0); 
end:
if check_examples() then
    TEST_EQUAL("simplify(ttruncatedSL(t,T,[1/2,1/2],[[1,0],[0,1]],[[1,0]],[1,1],[1,1],0))",
               "1/12*(12+12*delta*ourfrac(-1/2*t)+12*delta*T-6*delta+6*ourfrac(-1/2*t)^2*delta^2+12*T*ourfrac(-1/2*t)*delta^2+6*T^2*delta^2-6*T*delta^2-6*ourfrac(-1/2*t)*delta^2+delta^2)/delta^2",
               "ttruncatedSL test #1");
    # Example 1.3
    vertices := [[0, 0], [sqrt(2), 0], [sqrt(2), 1], [0, 1]];
    simple_vertex_cones := [[[1, 0], [0, 1]], 
                            [[-1, 0], [0, 1]], 
                            [[-1, 0], [0, -1]], 
                            [[1, 0], [0, -1]]]:
    L := []:       # we are counting lattice points.
    ell := [1, 0]: # an arbitrary linear form.
    M := 0:        # we compute the sum of ell^M.
    reg := [1, 1]: # a regular linear form.
    S := 
    TEST_EQUAL("expand(add(ttruncatedSL('t', 'T', vertices[i], simple_vertex_cones[i], L, ell, reg, M), i=1..nops(vertices)))",
               "1+T+T*2^(1/2)-ourfrac(2^(1/2)*t)+T^2*2^(1/2)+ourfrac(t)*ourfrac(2^(1/2)*t)-ourfrac(t)*T*2^(1/2)-ourfrac(t)-T*ourfrac(2^(1/2)*t)",
               "ttruncatedSL test #2");
fi:

# Simplex S (given by vertices), dilated by t.
# t may have assumptions.
SLsimplex:=proc(t,S,L,ell,M) local F,W,i,reg;
    F:=0;
    reg:=regularSL(S,L);
    for i from 1 to nops(S) do 
        W:=[seq(primitive_vector(S[j]-S[i]),j=1..i-1),seq(primitive_vector(S[j]-S[i]),j=i+1..nops(S))];
        F:=F+ttruncatedSL(t,'T',S[i],W,L,ell,reg,M);  ## introduces formal T.
    od:
    subs(T=t,simplify(coeff(F,delta,M))):  ###  Backsubst of formal T.
end:
if check_examples() then
    ASSERT(SLsimplex(t,[[0, 0], [1, 0], [1, 1]],[],[x1,x2],0)
           = 
           1/2*ourfrac(t)^2+1/2*t^2-3/2*ourfrac(t)+3/2*t+1-t*ourfrac(t),
           "SLsimplex test #1");
    #S1;
    checks:=proc(u);
        [function_SL_simplex_ell([[0, 0], [u, 0], [u, 2*u]],[],[1,1],0),
         subs(t=u, SLsimplex(t,[[0, 0], [1, 0], [1, 2]],[],[1,1],0))];
    end:
    c := checks(15/7):
    ASSERT(c[1] = c[2],
           "SLsimplex test #2");
    ASSERT(c[1] = 484/49+(1/7)^2-44/7*1/7, # Looks correct
           "SLsimplex test #3"); 
    threechecks:=proc(u);
        [function_SL_simplex_ell([[0, 0,0], [u, 0,0], [u, 2*u,0],[u,u,3*u]],[],[1,1,1],0),
         subs(t=u,SLsimplex(t,[[0, 0,0],[1,0,0], [1, 2,0], [1, 1,3]],[],[1,1,1],0))];
    end:
    c := threechecks(11/2);
    ASSERT(c[1] = c[2],
           "SLsimplex test #4");
    ASSERT(c[1] = 1027/4-(483/4)*(1/2)+19*(1/2)^2-(1/2)^3, # WHOOAH
           "SLsimplex test #5");
    ASSERT((SLsimplex(t,[[0, 0], [1, 0], [1, 1]],[],[x1,x2],0) assuming t::integer)
           =  
           1+1/2*t^2+3/2*t,
           "SLsimplex test #6");
fi:


# Almost at the end
# Nicole  formula to compute rho:
# Datas: k,n,p integers with n<=p. We want to compute all the ordered (n_i<=n_{i+1}.....) subpartition [n_1,..n_k] of [1..p] with n_j>=n and n_1+..n_k<=p 
# Intermediate_step does one step in the following way: start with a list L=[n_1,..n_m] such that L[j]>=n L[1]<=.  ..<=L[m] and L[1]+..L[nops(L)]<=p (k,n,p as above) Construct all the lists L' (beware we want to arrive to something of lenght k)  obtained by adding to L an element more with the property we want. If nops(L)>=k+1 we want to return [].
# The program complete_inter_step(L,k,n,p) does the same but now L is a list of list.
# Finally the program truncated_partitions constructs all the subpartition [n_1,..n_k] of [1..p] with n_j>=n and n_1+..n_k<=p

# Input: k,n,p integers. L list  with L[j]>=n and L[1]+..L[nops(L)]<=p or L=[] or nops(L)+1<=k
# Output: a list of list, each with an element more (same rule) or the empty list
intermediate_step:=proc(L,k,n,p) local B,s,m,A,Q,out;
    out:=[];
    s:=nops(L);
    m:=add(L[j],j=1..nops(L));##print("m",m,s,p-m,k-s,(p-m),(k-s));
    if (k-s)<0 then out:=out; 
        
    elif k=s then out:=L;
    else #print("notzero",k-s);
        A:=floor((p-m)/(k-s));#print("A",A);
        #A:=floor((p-m)/(m-s));
        if L=[] then B:=n; else B:=L[s];fi;
        #print("B",B);
        Q:=[seq(B+i,i=0..A-B)];
        out:=[seq([op(L),Q[j]],j=1..nops(Q))];
    fi;
    out;
end:
if check_examples() then
    ASSERT(intermediate_step([1,2],2,1,3)
           = [1,2],
           "intermediate_step test #1");
    ASSERT(intermediate_step([],2,1,3)
           = [[1]],
           "intermediate_step test #2");
    ASSERT(intermediate_step([1],3,1,5)
           = [[1, 1], [1, 2]],
           "intermediate_step test #3");
    ASSERT(intermediate_step([],3,1,5)
           = [[1]],
           "intermediate_step test #4");
    ASSERT(intermediate_step([1,2,3,4],3,1,5)
           = [],
           "intermediate_step test #5");
    ASSERT(intermediate_step([1,2,2],3,2,7)
           = [1, 2, 2],
           "intermediate_step test #6");
    ASSERT(intermediate_step([1,2,2,2],3,2,7)
           = [],
           "intermediate_step test #7");
fi:

# Input:k,n,p, integers.  L a list of list of integers. meaning as above
# Output: a list of lists each with one element more
# Example: complete_inter_step([[1]],2,1,3)→[[1,1],[1,2]]
# 
complete_inter_step:=proc(L,k,n,p) local out,i,newL;
    out:=[];
    for i from 1 to nops(L) do
        newL:=intermediate_step(L[i],k,n,p);
        out:=[op(out),op(newL)];
    od:
    out;
end:
if check_examples() then
    ASSERT(complete_inter_step([[1]],2,1,3)
           = [[1, 1], [1, 2]],
           "");
    ASSERT(complete_inter_step([[1],[2]],2,1,3)
           = [[1, 1], [1, 2]],
           "");
    ASSERT(complete_inter_step([[2],[2]],2,1,3)
           = [],
           "");
fi:

# Input: k,n,p integers # In the application n=d0+1 so is always greater or equal than 2
# Output: a list of lists
# Math: the partition with repetition of a number <=p via a list of k elements each >=n
truncated_partition:=proc(k,n,p) local j,out,L:
    out:=[];
    L:=[seq([s],s=n..floor(p/k))];#print("startL",L);
    for j from 1 to k-1 do
        #print("L",L,j);
        L:=complete_inter_step(L,k,n,p);
    od;
    L
end:
if check_examples() then
    ASSERT(truncated_partition(3,2,8)
           = [[2, 2, 2], [2, 2, 3], [2, 2, 4], [2, 3, 3]],
           "truncated_partition test #1");
    ASSERT(truncated_partition(2,1,8)
           = [[1, 1], [1, 2], [1, 3], [1, 4], [1, 5], [1, 6], [1, 7], [2, 2], [2, 3], [2, 4], [2, 5], [2, 6], [3, 3], [3, 4], [3, 5], [4, 4]],
           "truncated_partition test #2");
    ASSERT(truncated_partition(4,3,9)
           = [],
           "truncated_partition test #3");
    ASSERT(truncated_partition(3,2,7)
           = [[2, 2, 2], [2, 2, 3]],
           "truncated_partition test #4");
fi:

#we want to sort a list of lists of lenght  from 1.. to p/n corresponding to dim face >=n-1
all_truncated_partition:=proc(p,n) local k,out;
    out:=[];
    #print(floor(p/n));
    #for k from 1  to floor((p)/2)  do 
    for k from 1  to floor(p/n)  do #print(k,truncated_partition(k,n,p));
        out:=[op(out),op(truncated_partition(k,n,p))];
    od;
    out:
end:
if check_examples() then
    ASSERT(all_truncated_partition(5,2)
           = [[2], [3], [4], [5], [2, 2], [2, 3]],
           "");
fi:

# Now we need to recover the I=[I_1..I_m], with |I_1|=n_1..|I_m|=n_m starting from an element of truncated_partition
# midstep: 
# input L={[mixed,[{[A1],[],..[Ar]},s1...st],[set,setofindices]} 
# output: a list of the same form L'={[mixed,{[A1],[],..[Ar+1]},s2...st],[set,newsetofindices]}
midstep_possible_I:=proc(L) local newLset,Lsetspecial,QQ,Lmixed,start,Lset,C,i,j,out1,LLset;
    #print("inputnew_mid",L);
    if L[1][1]=mixed then
        Lmixed:=L[1][2];Lset:=L[2][2];else 
        Lmixed:=L[2][2];Lset:=L[1][2];
    fi;
    Lsetspecial:=[op({op(Lset)}minus {1})];#print("Lsetspecial",Lsetspecial);print("Lmixed,Lstart",Lmixed,Lset);
    out1:={};
    start:=Lmixed[2];#print("start",start);
    #if start=1 then
    #C:=choose(Lsetspecial,start);
    #else
    #print("start",start);
    #print("again",Lmixed,Lset,start);
    C:=choose(Lset,start);#print(Lset,start,C);
    for i from 1 to nops(C) do
        LLset:=[op({op(Lset)} minus {op(C[i])})];
        
        newLset:=[set,LLset];#print("LL",newLset);
        QQ:=[mixed,[{op(Lmixed[1]),C[i]},seq(Lmixed[k],k=3..nops(Lmixed))]];
        
        out1:={op(out1),{QQ,newLset}};#print(out1,"i",i);
    od;
    #fi:
    out1;
end:
if check_examples() then
    ASSERT(midstep_possible_I({[mixed, [{}, 1, 2]],[set, [1, 2, 3, 4]]})
           = {{[mixed, [{[1]}, 2]], [set, [2, 3, 4]]}, {[mixed, [{[2]}, 2]], [set, [1, 3, 4]]}, {[mixed, [{[3]}, 2]], [set, [1, 2, 4]]}, {[mixed, [{[4]}, 2]], [set, [1, 2, 3]]}},
           "midstep_possible_I test #1");
    ASSERT(midstep_possible_I({[mixed, [{}, 2, 2, 2]],[set, [1, 2, 3, 4, 5, 6]]})
           = {{[mixed, [{[1, 2]}, 2, 2]], [set, [3, 4, 5, 6]]}, {[mixed, [{[1, 3]}, 2, 2]], [set, [2, 4, 5, 6]]}, {[mixed, [{[1, 4]}, 2, 2]], [set, [2, 3, 5, 6]]}, {[mixed, [{[1, 5]}, 2, 2]], [set, [2, 3, 4, 6]]}, {[mixed, [{[1, 6]}, 2, 2]], [set, [2, 3, 4, 5]]}, {[mixed, [{[2, 3]}, 2, 2]], [set, [1, 4, 5, 6]]}, {[mixed, [{[2, 4]}, 2, 2]], [set, [1, 3, 5, 6]]}, {[mixed, [{[2, 5]}, 2, 2]], [set, [1, 3, 4, 6]]}, {[mixed, [{[2, 6]}, 2, 2]], [set, [1, 3, 4, 5]]}, {[mixed, [{[3, 4]}, 2, 2]], [set, [1, 2, 5, 6]]}, {[mixed, [{[3, 5]}, 2, 2]], [set, [1, 2, 4, 6]]}, {[mixed, [{[3, 6]}, 2, 2]], [set, [1, 2, 4, 5]]}, {[mixed, [{[4, 5]}, 2, 2]], [set, [1, 2, 3, 6]]}, {[mixed, [{[4, 6]}, 2, 2]], [set, [1, 2, 3, 5]]}, {[mixed, [{[5, 6]}, 2, 2]], [set, [1, 2, 3, 4]]}},
           "midstep_possible_I test #2");
fi:

#given a list=[n_1,..,n_k], as in truncated, we construct all the [I_1,..I_m], I_i intersects I_j empty |I_j|=n_j.
one_possible_I:=proc(TR,p)local out, Lset,i,j,Lmixed,initial:
    out:=[];
    Lset:=[set,[seq(k,k=1..p)]];
    Lmixed:=[mixed,[{},op(TR)]];
    initial:=midstep_possible_I({Lmixed,Lset});
    j:=2; 
    while j <=nops(TR) do
        #for k from 1 to nops(out) do
        initial:={seq(op(midstep_possible_I(initial[i])),i=1..nops(initial))};
        #print("j",j,initial);#out1:=out;
        j:=j+1;
    od:
    for i from 1 to nops(initial) do
        if initial[i][1][1]=mixed then out:=[op(out),convert(op(initial[i][1][2]),list)];else out:=[op(out),convert(op(initial[i][2][2]),list)]
        fi;od:
    out:
end:
if check_examples() then
    ASSERT(one_possible_I([1,2],7)
           = [[[1], [2, 3]], [[1], [2, 4]], [[1], [2, 5]], [[1], [2, 6]], [[1], [2, 7]], [[1], [3, 4]], [[1], [3, 5]], [[1], [3, 6]], [[1], [3, 7]], [[1], [4, 5]], [[1], [4, 6]], [[1], [4, 7]], [[1], [5, 6]], [[1], [5, 7]], [[1], [6, 7]], [[2], [1, 3]], [[2], [1, 4]], [[2], [1, 5]], [[2], [1, 6]], [[2], [1, 7]], [[2], [3, 4]], [[2], [3, 5]], [[2], [3, 6]], [[2], [3, 7]], [[2], [4, 5]], [[2], [4, 6]], [[2], [4, 7]], [[2], [5, 6]], [[2], [5, 7]], [[2], [6, 7]], [[3], [1, 2]], [[3], [1, 4]], [[3], [1, 5]], [[3], [1, 6]], [[3], [1, 7]], [[3], [2, 4]], [[3], [2, 5]], [[3], [2, 6]], [[3], [2, 7]], [[3], [4, 5]], [[3], [4, 6]], [[3], [4, 7]], [[3], [5, 6]], [[3], [5, 7]], [[3], [6, 7]], [[4], [1, 2]], [[4], [1, 3]], [[4], [1, 5]], [[4], [1, 6]], [[4], [1, 7]], [[4], [2, 3]], [[4], [2, 5]], [[4], [2, 6]], [[4], [2, 7]], [[4], [3, 5]], [[4], [3, 6]], [[4], [3, 7]], [[4], [5, 6]], [[4], [5, 7]], [[4], [6, 7]], [[5], [1, 2]], [[5], [1, 3]], [[5], [1, 4]], [[5], [1, 6]], [[5], [1, 7]], [[5], [2, 3]], [[5], [2, 4]], [[5], [2, 6]], [[5], [2, 7]], [[5], [3, 4]], [[5], [3, 6]], [[5], [3, 7]], [[5], [4, 6]], [[5], [4, 7]], [[5], [6, 7]], [[6], [1, 2]], [[6], [1, 3]], [[6], [1, 4]], [[6], [1, 5]], [[6], [1, 7]], [[6], [2, 3]], [[6], [2, 4]], [[6], [2, 5]], [[6], [2, 7]], [[6], [3, 4]], [[6], [3, 5]], [[6], [3, 7]], [[6], [4, 5]], [[6], [4, 7]], [[6], [5, 7]], [[7], [1, 2]], [[7], [1, 3]], [[7], [1, 4]], [[7], [1, 5]], [[7], [1, 6]], [[7], [2, 3]], [[7], [2, 4]], [[7], [2, 5]], [[7], [2, 6]], [[7], [3, 4]], [[7], [3, 5]], [[7], [3, 6]], [[7], [4, 5]], [[7], [4, 6]], [[7], [5, 6]]],
           "one_possible_I test #1");
    ASSERT(one_possible_I([3],6)
           = [[[1, 2, 3]], [[1, 2, 4]], [[1, 2, 5]], [[1, 2, 6]], [[1, 3, 4]], [[1, 3, 5]], [[1, 3, 6]], [[1, 4, 5]], [[1, 4, 6]], [[1, 5, 6]], [[2, 3, 4]], [[2, 3, 5]], [[2, 3, 6]], [[2, 4, 5]], [[2, 4, 6]], [[2, 5, 6]], [[3, 4, 5]], [[3, 4, 6]], [[3, 5, 6]], [[4, 5, 6]]],
           "one_possible_I test #2");
    ASSERT(one_possible_I([3],8)
           = [[[1, 2, 3]], [[1, 2, 4]], [[1, 2, 5]], [[1, 2, 6]], [[1, 2, 7]], [[1, 2, 8]], [[1, 3, 4]], [[1, 3, 5]], [[1, 3, 6]], [[1, 3, 7]], [[1, 3, 8]], [[1, 4, 5]], [[1, 4, 6]], [[1, 4, 7]], [[1, 4, 8]], [[1, 5, 6]], [[1, 5, 7]], [[1, 5, 8]], [[1, 6, 7]], [[1, 6, 8]], [[1, 7, 8]], [[2, 3, 4]], [[2, 3, 5]], [[2, 3, 6]], [[2, 3, 7]], [[2, 3, 8]], [[2, 4, 5]], [[2, 4, 6]], [[2, 4, 7]], [[2, 4, 8]], [[2, 5, 6]], [[2, 5, 7]], [[2, 5, 8]], [[2, 6, 7]], [[2, 6, 8]], [[2, 7, 8]], [[3, 4, 5]], [[3, 4, 6]], [[3, 4, 7]], [[3, 4, 8]], [[3, 5, 6]], [[3, 5, 7]], [[3, 5, 8]], [[3, 6, 7]], [[3, 6, 8]], [[3, 7, 8]], [[4, 5, 6]], [[4, 5, 7]], [[4, 5, 8]], [[4, 6, 7]], [[4, 6, 8]], [[4, 7, 8]], [[5, 6, 7]], [[5, 6, 8]], [[5, 7, 8]], [[6, 7, 8]]],
           "one_possible_I test #3");
    #[[4, 4, 4], [4, 4, 5], [4, 5, 5], [4, 4, 6], [5, 5, 5], [4, 5, 6], [4, 4, 7]];
fi:


# here we compute the Lspace associate to a list of vertices examples [1,2] corresponds to lin(s_1-s_2 )
L_space:=proc(simplex,II) local s,i,out;
    out:=[];
    s:=simplex;
    for i from 1 to nops(II) do
        out:=[op(out),seq(s[II[i][j]]-s[II[i][1]],j=2..nops(II[i]))];
    od;
end:
if check_examples() then
    ASSERT(L_space([[1,0,0],[0,0,0],[0,1,0],[0,0,1]],[[1,3],[2,4]])
           = [[-1, 1, 0], [0, 0, 1]],
           "L_space test #1");
fi:
#complete_approximation(t,Sstandard(2),2,0,0);

# rho
# Input: k integers
# Output: a polynom
# Math: we compute the exp truncated at x^{k-1}

exp_trunc:=proc(x,k) ;
    add(1/n!*x^n,n=0..k-1);
end:

Bjiorner_rho:=proc(n,p)local out;
    if n=1 then out:=0; else
        out:=-n!*coeff(series(log(exp_trunc(x,p)),x=0,n+4),x,n);
    fi:
    out;
end:

# Input: a list L of integers SORTED
# Output: a list of  integers
# Math: we compute the multiplicity of each integer appearing in L
# Exa
new_list_multiplicity:=proc(L) local out,LL,nL,AA,newL,j;LL:=[];out:=[];
    if nops(L)=1 then out:=[1]; 
    else 
        nL:=sort(L);
        newL:=[seq(nL[i+1]-nL[i],i=1..nops(nL)-1)];#print(newL);#nL:=newL;
        for j from 1 to nops(newL) do
            if newL[j]<>0 then LL:=[op(LL),j];else LL:=LL;
            fi;
        od; 
        if newL[nops(newL)]=0 then 
            LL:=[op(LL),nops(newL)+1];
        fi;#print(LL);
        j:=1; 
        while j<=nops(LL) do
            if j=1 then AA:=LL[j]; else
                AA:= LL[j]-LL[j-1]; 
            fi;
            if AA=1 then out:=out; j:=j+1;
            else out:=[op(out),AA];j:=j+1;fi;
        od:
        #print("out",out);
        #B:=add(out[i],i=1..nops(out));
        #out:=[op(out),seq(1,i=1..nops(L)-B)];
    fi:
    if out=[] then out:=[1];fi;
    out;
end:

# Input:=k,n,p,integers
# Output: a number
# Math:= truncated multinomials as in formula....
truncated_multinomial:=proc(k,n,p) local A,multT,T,i,out,ex;out:=0;
    #p:=floor(N/n);
    T:=truncated_partition(k,n,p):#print(T);
    for i from 1 to nops(T) do 
        #multT:=list_multiplicity(T[i]); #print(multT,T[i],i);
        multT:=new_list_multiplicity(T[i]);
        A:=add(multT[j],j=1..nops(multT));
        ex:=p-add(T[i][j],j=1..k);
        #out:=out+1/k*(-1)^(k-1)*(-k)^ex*multinomial(k, seq(multT[j],j=1..nops(multT)),0)*multinomial(p, seq(T[i][j],j=1..k),ex);
        out:=out+1/k*(-1)^(k-1)*(-k)^ex*multinomial(k, seq(multT[j],j=1..nops(multT)),k-A)*multinomial(p, seq(T[i][j],j=1..k),ex);
    od:
    out:
end: 
if check_examples() then
    ASSERT(truncated_multinomial(2,1,4)
           = -7,
           "truncated_multinomial test #1");
fi:

N_rho:=proc(n,p) local f;
    f:=floor(p/n):
    add((-1)^(k-1)*truncated_multinomial(k,n,p),k=1..f):
end:
if check_examples() then
    ASSERT(N_rho(1,4)
           = 0,
           "N_rho test #1");
    ASSERT(N_rho(2,2)
           = 1,
           "N_rho test #2");
    ASSERT(N_rho(2,3)
           = -2,
           "N_rho test #3");
    ASSERT(N_rho(2,4)
           = 6,
           "N_rho test #4");
    #it doesn't compute....N_rho(2,100)
fi:

#check the (-1) in front
patchrho:=proc(n,L) local i,RHO;RHO:=1;
    for i from 1 to nops(L) do
        RHO:=RHO*N_rho(n,L[i]);
        
    od:
    (-1)^(nops(L)-1)*RHO;
end:

# 
# Approximation with lambda patch
#L is the list of cardinality [n_1,..n_m] where m<=[(d+1)/2] and n_i>=d0+1 in the sequel we will have to construct all the list depending from k,n,p, where k=m is the number of elements, n means n_j>=n=d-k0, p=d+1 
list_contribution_approximation:=proc(t,S,L,n,ell,M) local out,FS,Lspace,rho,j,d,LL;
    #print("enter",L);
    d:=nops(S)-1; #k0:=d+1-d0;
    #rho:=patchrho(n+1,L);
    rho:=patchrho(n,L);
    if rho=0 then out:=0; #print(rho=0);
    else
        LL:=one_possible_I(L,d+1);#print("LL",LL,L);
        out:=0;
        for j from 1 to nops(LL) do
            Lspace:=L_space(S,LL[j]);#print("Lspace",Lspace,LL[j]);
            #rho:=patchrho(n+1,L);
            #rho:=
            #FS:=function_SL_simplex(S,Lspace,x);
            FS:=SLsimplex(t,S,Lspace,ell,M);#print(j,FS);
            out:=out+rho*FS;
        od:
    fi;
    out;
end:

fullbarvinok:=proc(t,S,k0,ell,M) local d,d0,L,i,out; #NEW
    d:=nops(S)-1;out:=0;
    if not type(S, list) then
        error "Parameter S must be a list (of vertices of the simplex). Received %1", S;
    fi:
    d0:=d-k0;
    if d0 < 0 then
        error "Parameter k0 = %1 is too big for dimension of S, %2", k0, d;
    fi:
    L:=all_truncated_partition(d+1,d0+1);#print("L",L);
    for i from 1 to nops(L) do 
        out:=out+list_contribution_approximation(t,S,L[i],d0+1,ell,M);
    od;
    out:=out;
end:

if check_examples() then
    #Exemples - copied from RealBarvinok-avril-forMatthias-2014-10-16.mpl
    # corresponds to Example 5.5
    A:=[[1,1],[1,2],[2,2]]:
    ASSERT(fullbarvinok(t,A,2,[9,2],0)
           = 1/2*ourfrac(-t)^2-t*ourfrac(-t)-3/2*ourfrac(-t)+1+3/2*t+1/2*t^2+ourfrac(-t)*ourfrac(2*t)-3/2*ourfrac(2*t)+1/2*ourfrac(2*t)^2-t*ourfrac(2*t),
           "fullbarvinok test #1");
    ASSERT(fullbarvinok(t,A,1,[9,2],0)
           = 1/2*t^2-t*ourfrac(-t)+3/2*t-1/2*ourfrac(t)^2+1/2*ourfrac(t)-t*ourfrac(2*t),
           "fullbarvinok test #2");
    # Example 5.4
    Simplex4 := [[4,6,4,3],[5,7,9,1],[5,7,3,7],[6,8,3,9],[2,1,8,0]];
    ASSERT(collect((fullbarvinok(t, Simplex4, 1, [0, 0, 0, 0], 0) assuming t::integer), t)
           = 3/4*t^4+7/24*t^2+2*t^3,
           "fullbarvinok test #3");
fi:    
