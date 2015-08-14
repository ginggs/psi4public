/*
 *@BEGIN LICENSE
 *
 * PSI4: an ab initio quantum chemistry software package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *@END LICENSE
 */

#include <libqt/qt.h>
#include "defines.h"
#include "dfocc.h"

using namespace psi;
using namespace std;


namespace psi{ namespace dfoccwave{
  
void DFOCC::ccsd_canonic_triples()
{

    // defs
    SharedTensor2d K, L, M, I, J, T, U, Tau, W, X, Y, Z;
    SharedTensor2d V, J1, J2, J3, Jt;
    SharedTensor1d Eijk;
    int Nijk;

    // Find number of unique ijk combinations (i>=j>=k)
    /*
    Nijk = 0;
    for(int i = 0 ; i < naoccA; ++i){
        for(int j = 0 ; j <= i; ++j){
            for(int k = 0 ; k <= j; ++k){
		Nijk++;
	    }
	}
    }
    */
    Nijk = naoccA * (naoccA + 1) * (naoccA + 2) / 6;
    outfile->Printf("\tNumber of ijk combinations: %i \n",Nijk);

    // Malloc Eijk
    //Eijk = SharedTensor1d(new Tensor1d("Eijk", Nijk));

    // Memory: 2*O^2V^2 + 5*V^3 + O^3V + V^2N + V^3/2 

    // Read t2 amps
    t2 = SharedTensor2d(new Tensor2d("T2 (IA|JB)", naoccA, navirA, naoccA, navirA));
    t2->read_symm(psio_, PSIF_DFOCC_AMPS);
    T = SharedTensor2d(new Tensor2d("T2 <IJ|AB>", naoccA, naoccA, navirA, navirA));
    T->sort(1324, t2, 1.0, 0.0);
    t2.reset();

    // Form (ij|ka)
    M = SharedTensor2d(new Tensor2d("DF_BASIS_CC B (Q|IA)", nQ, naoccA, navirA));
    M->read(psio_, PSIF_DFOCC_INTS);
    K = SharedTensor2d(new Tensor2d("DF_BASIS_CC B (Q|IJ)", nQ, naoccA, naoccA));
    K->read(psio_, PSIF_DFOCC_INTS);
    J = SharedTensor2d(new Tensor2d("DF_BASIS_CC MO Ints (IJ|KA)", naoccA, naoccA, naoccA, navirA));
    J->gemm(true, false, K, M, 1.0, 0.0);
    K.reset();
    I = SharedTensor2d(new Tensor2d("DF_BASIS_CC MO Ints <IJ|KA>", naoccA, naoccA, naoccA, navirA));
    I->sort(1324, J, 1.0, 0.0);
    J.reset();

    // Form (ia|jb)
    J = SharedTensor2d(new Tensor2d("DF_BASIS_CC MO Ints (IA|JB)", naoccA, navirA, naoccA, navirA));
    J->gemm(true, false, M, M, 1.0, 0.0);

    // B(iaQ)
    L = SharedTensor2d(new Tensor2d("DF_BASIS_CC B (IA|Q)", naoccA * navirA, nQ));
    L = M->transpose();
    M.reset();

    // malloc W[ijk](abc)
    W = SharedTensor2d(new Tensor2d("W[IJK] <AB|C>", navirA * navirA, navirA));
    V = SharedTensor2d(new Tensor2d("V[IJK] <BA|C>", navirA * navirA, navirA));
    J1 = SharedTensor2d(new Tensor2d("J[I] <AB|E>", navirA * navirA, navirA));
    J2 = SharedTensor2d(new Tensor2d("J[J] <AB|E>", navirA * navirA, navirA));
    J3 = SharedTensor2d(new Tensor2d("J[K] <AB|E>", navirA * navirA, navirA));

    // B(Q,ab)
    K = SharedTensor2d(new Tensor2d("DF_BASIS_CC B (Q|AB)", nQ, ntri_abAA));
    K->read(psio_, PSIF_DFOCC_INTS);
    Jt = SharedTensor2d(new Tensor2d("J[I] <A|B>=C", navirA, ntri_abAA));

    // main loop
    E_t = 0.0;
    for(int i = 0 ; i < naoccA; ++i){
        double Di = FockA->get(i + nfrzc, i + nfrzc);

	// Compute J[i](a,bc) = (ia|bc) = \sum(Q) B[i](aQ) * B(Q,bc)
        Jt->contract(false, false, navirA, ntri_abAA, nQ, L, K, i*navirA*nQ, 0, 1.0, 0.0);
	J1->expand23(navirA, navirA, navirA, Jt);

        for(int j = 0 ; j <= i; ++j){
	    double Dij = Di + FockA->get(j + nfrzc, j + nfrzc);

	    // Compute J[j](a,bc) = (ja|bc) = \sum(Q) B[j](aQ) * B(Q,bc)
            Jt->contract(false, false, navirA, ntri_abAA, nQ, L, K, j*navirA*nQ, 0, 1.0, 0.0);
	    J2->expand23(navirA, navirA, navirA, Jt);

            for(int k = 0 ; k <= j; ++k){
	        // Compute J[k](a,bc) = (ka|bc) = \sum(Q) B[k](aQ) * B(Q,bc)
                Jt->contract(false, false, navirA, ntri_abAA, nQ, L, K, k*navirA*nQ, 0, 1.0, 0.0);
	        J3->expand23(navirA, navirA, navirA, Jt);

                // W[ijk](ab,c) = \sum(e) t_jk^ec (ia|be) (1+)
                // W[ijk](ab,c) = \sum(e) J[i](ab,e) T[jk](ec)
                W->contract(false, false, navirA*navirA, navirA, navirA, J1, T, 0, (j*naoccA*navirA*navirA) + (k*navirA*navirA), 1.0, 0.0);

                // W[ijk](ab,c) -= \sum(m) t_im^ab <jk|mc> (1-)
                // W[ijk](ab,c) -= \sum(m) T[i](m,ab) I[jk](mc)
                W->contract(true, false, navirA*navirA, navirA, naoccA, T, I, i*naoccA*navirA*navirA, (j*naoccA*naoccA*navirA) + (k*naoccA*navirA), -1.0, 1.0);

                // W[ijk](ac,b) = \sum(e) t_kj^eb (ia|ce) (2+)
                // W[ijk](ac,b) = \sum(e) J[i](ac,e) T[kj](eb)
                V->contract(false, false, navirA*navirA, navirA, navirA, J1, T, 0, (k*naoccA*navirA*navirA) + (j*navirA*navirA), 1.0, 0.0);

                // W[ijk](ac,b) -= \sum(m) t_im^ac <kj|mb> (2-)
                // W[ijk](ac,b) -= \sum(m) T[i](m,ac) I[kj](mb)
                V->contract(true, false, navirA*navirA, navirA, naoccA, T, I, i*naoccA*navirA*navirA, (k*naoccA*naoccA*navirA) + (j*naoccA*navirA), -1.0, 1.0);
                #pragma omp parallel for
                for(int a = 0 ; a < navirA; ++a){
                    for(int b = 0 ; b < navirA; ++b){
		        W->axpy((ULI)navirA, a*navirA*navirA + b, navirA, V, a*navirA*navirA + b*navirA, 1, 1.0);
		    }
		}

                // W[ijk](ba,c) = \sum(e) t_ik^ec (jb|ae) (3+)
                // W[ijk](ba,c) = \sum(e) J[j](ba,e) T[ik](ec)
                V->contract(false, false, navirA*navirA, navirA, navirA, J2, T, 0, (i*naoccA*navirA*navirA) + (k*navirA*navirA), 1.0, 0.0);

                // W[ijk](ba,c) -= \sum(m) t_jm^ba <ik|mc> (3-)
                // W[ijk](ba,c) -= \sum(m) T[j](m,ba) I[ik](mc)
                V->contract(true, false, navirA*navirA, navirA, naoccA, T, I, j*naoccA*navirA*navirA, (i*naoccA*naoccA*navirA) + (k*naoccA*navirA), -1.0, 1.0);
		#pragma omp parallel for
                for(int a = 0 ; a < navirA; ++a){
                    for(int b = 0 ; b < navirA; ++b){
		        W->axpy((ULI)navirA, b*navirA*navirA + a*navirA, 1, V, a*navirA*navirA + b*navirA, 1, 1.0);
		    }
		}

                // W[ijk](bc,a) = \sum(e) t_ki^ea (jb|ce) (4+)
                // W[ijk](bc,a) = \sum(e) J[j](bc,e) T[ki](ea)
                V->contract(false, false, navirA*navirA, navirA, navirA, J2, T, 0, (k*naoccA*navirA*navirA) + (i*navirA*navirA), 1.0, 0.0);

                // W[ijk](bc,a) -= \sum(m) t_jm^bc <ki|ma> (4-)
                // W[ijk](bc,a) -= \sum(m) T[j](m,bc) I[ki](ma)
                V->contract(true, false, navirA*navirA, navirA, naoccA, T, I, j*naoccA*navirA*navirA, (k*naoccA*naoccA*navirA) + (i*naoccA*navirA), -1.0, 1.0);
		#pragma omp parallel for
                for(int a = 0 ; a < navirA; ++a){
                    for(int b = 0 ; b < navirA; ++b){
		        W->axpy((ULI)navirA, b*navirA*navirA + a, navirA, V, a*navirA*navirA + b*navirA, 1, 1.0);
		    }
		}

                // W[ijk](ca,b) = \sum(e) t_ij^eb (kc|ae) (5+)
                // W[ijk](ca,b) = \sum(e) J[k](ca,e) T[ij](eb)
                V->contract(false, false, navirA*navirA, navirA, navirA, J3, T, 0, (i*naoccA*navirA*navirA) + (j*navirA*navirA), 1.0, 0.0);

                // W[ijk](ca,b) -= \sum(m) t_km^ca <ij|mb> (5-)
                // W[ijk](ca,b) -= \sum(m) T[k](m,ca) I[ij](mb)
                V->contract(true, false, navirA*navirA, navirA, naoccA, T, I, k*naoccA*navirA*navirA, (i*naoccA*naoccA*navirA) + (j*naoccA*navirA), -1.0, 1.0);
		#pragma omp parallel for
                for(int a = 0 ; a < navirA; ++a){
                    for(int b = 0 ; b < navirA; ++b){
		        W->axpy((ULI)navirA, a*navirA + b, navirA*navirA, V, a*navirA*navirA + b*navirA, 1, 1.0);
		    }
		}

                // W[ijk](cb,a) = \sum(e) t_ji^ea (kc|be) (6+)
                // W[ijk](cb,a) = \sum(e) J[k](cb,e) T[ji](ea)
                V->contract(false, false, navirA*navirA, navirA, navirA, J3, T, 0, (j*naoccA*navirA*navirA) + (i*navirA*navirA), 1.0, 0.0);

                // W[ijk](cb,a) -= \sum(m) t_km^cb <ji|ma> (6-)
                // W[ijk](cb,a) -= \sum(m) T[k](m,cb) I[ji](ma)
                V->contract(true, false, navirA*navirA, navirA, naoccA, T, I, k*naoccA*navirA*navirA, (j*naoccA*naoccA*navirA) + (i*naoccA*navirA), -1.0, 1.0);
		#pragma omp parallel for
                for(int a = 0 ; a < navirA; ++a){
                    for(int b = 0 ; b < navirA; ++b){
		        W->axpy((ULI)navirA, b*navirA + a, navirA*navirA, V, a*navirA*navirA + b*navirA, 1, 1.0);
		    }
		}

		// V[ijk](ab,c) = W[ijk](ab,c) 
		V->copy(W);

		// V[ijk](ab,c) += t_i^a (jb|kc) + t_j^b (ia|kc) + t_k^c (ia|jb)
		// Vt[ijk](ab,c) = V[ijk](ab,c) / (1 + \delta(abc))
                #pragma omp parallel for
                for(int a = 0 ; a < navirA; ++a){
	            int ia = ia_idxAA->get(i,a);
                    for(int b = 0 ; b < navirA; ++b){
			int jb = ia_idxAA->get(j,b);
			int ab = ab_idxAA->get(a,b);
                        for(int c = 0 ; c < navirA; ++c){
			    int kc = ia_idxAA->get(k,c);
			    double value = V->get(ab,c) + ( t1A->get(i,a)*J->get(jb,kc) ) + ( t1A->get(j,b)*J->get(ia,kc) ) + ( t1A->get(k,c)*J->get(ia,jb) );
			    double denom =  1 + ( (a==b) + (b==c) + (a==c) );
			    V->set(ab, c, value/denom);
			}
		    }
		}

		// Denom
		double Dijk = Dij + FockA->get(k + nfrzc, k + nfrzc);
	        double factor =  2 - ( (i==j) + (j==k) + (i==k) );

		// Compute energy
                #pragma omp parallel for
                for(int a = 0 ; a < navirA; ++a){
	            double Dijka = Dijk - FockA->get(a + noccA, a + noccA);
                    for(int b = 0 ; b <=a; ++b){
			double Dijkab = Dijka - FockA->get(b + noccA, b + noccA);
			int ab = ab_idxAA->get(a,b);
			int ba = ab_idxAA->get(b,a);
                        for(int c = 0 ; c <= b; ++c){
			    int ac = ab_idxAA->get(a,c);
			    int bc = ab_idxAA->get(b,c);
			    int ca = ab_idxAA->get(c,a);
			    int cb = ab_idxAA->get(c,b);

			    // X_ijk^abc
			    double Xvalue = ( W->get(ab,c)*V->get(ab,c) ) + ( W->get(ac,b)*V->get(ac,b) ) 
				         + ( W->get(ba,c)*V->get(ba,c) ) + ( W->get(bc,a)*V->get(bc,a) )
					 + ( W->get(ca,b)*V->get(ca,b) ) + ( W->get(cb,a)*V->get(cb,a) );

			    // Y_ijk^abc
			    double Yvalue = V->get(ab,c) + V->get(bc,a) + V->get(ca,b);

			    // Z_ijk^abc
			    double Zvalue = V->get(ac,b) + V->get(ba,c) + V->get(cb,a);

			    // contributions to energy
			    double value = (Yvalue - (2.0*Zvalue)) * ( W->get(ab,c) + W->get(bc,a) + W->get(ca,b) ) ;
			    value += (Zvalue - (2.0*Yvalue)) * ( W->get(ac,b) + W->get(ba,c) + W->get(cb,a) ) ;
			    value += 3.0 * Xvalue;
			    double Dijkabc = Dijkab - FockA->get(c + noccA, c + noccA);
			    E_t += (value * factor) / Dijkabc;
			}
		    }
		}

	    }//k
	}//j
    }//i
    T.reset();
    J.reset();
    W.reset();
    V.reset();
    J1.reset();
    J2.reset();
    J3.reset();
    K.reset();
    Jt.reset();
    L.reset();
    I.reset();
 
    // set energy
    Eccsd_t = Eccsd + E_t;

}// end ccsd_canonic_triples

//======================================================================
//       (T): high-mem
//======================================================================             
void DFOCC::ccsd_canonic_triples_hm()
{

    // defs
    SharedTensor2d K, L, M, I, J, T, U, Tau, W, X, Y, Z;
    SharedTensor2d V, J1, J2, J3, Jt;
    SharedTensor1d Eijk;
    int Nijk;

    // Find number of unique ijk combinations (i>=j>=k)
    Nijk = naoccA * (naoccA + 1) * (naoccA + 2) / 6;
    outfile->Printf("\tNumber of ijk combinations: %i \n",Nijk);

    // Memory: OV^3 + 2*O^2V^2 + 2*V^3 + O^3V + V^2N

    // Read t2 amps
    t2 = SharedTensor2d(new Tensor2d("T2 (IA|JB)", naoccA, navirA, naoccA, navirA));
    t2->read_symm(psio_, PSIF_DFOCC_AMPS);
    T = SharedTensor2d(new Tensor2d("T2 <IJ|AB>", naoccA, naoccA, navirA, navirA));
    T->sort(1324, t2, 1.0, 0.0);
    t2.reset();

    // Form (ij|ka)
    M = SharedTensor2d(new Tensor2d("DF_BASIS_CC B (Q|IA)", nQ, naoccA, navirA));
    M->read(psio_, PSIF_DFOCC_INTS);
    K = SharedTensor2d(new Tensor2d("DF_BASIS_CC B (Q|IJ)", nQ, naoccA, naoccA));
    K->read(psio_, PSIF_DFOCC_INTS);
    J = SharedTensor2d(new Tensor2d("DF_BASIS_CC MO Ints (IJ|KA)", naoccA, naoccA, naoccA, navirA));
    J->gemm(true, false, K, M, 1.0, 0.0);
    K.reset();
    I = SharedTensor2d(new Tensor2d("DF_BASIS_CC MO Ints <IJ|KA>", naoccA, naoccA, naoccA, navirA));
    I->sort(1324, J, 1.0, 0.0);
    J.reset();

    // Form (ia|jb)
    J = SharedTensor2d(new Tensor2d("DF_BASIS_CC MO Ints (IA|JB)", naoccA, navirA, naoccA, navirA));
    J->gemm(true, false, M, M, 1.0, 0.0);

    // B(iaQ)
    L = SharedTensor2d(new Tensor2d("DF_BASIS_CC B (IA|Q)", naoccA * navirA, nQ));
    L = M->transpose();
    M.reset();

    // malloc W[ijk](abc)
    W = SharedTensor2d(new Tensor2d("W[IJK] <AB|C>", navirA * navirA, navirA));
    V = SharedTensor2d(new Tensor2d("V[IJK] <BA|C>", navirA * navirA, navirA));
    J1 = SharedTensor2d(new Tensor2d("DF_BASIS_CC MO Ints (IA|BC)", naoccA, navirA, navirA, navirA));

    // B(Q,ab)
    K = SharedTensor2d(new Tensor2d("DF_BASIS_CC B (Q|AB)", nQ, navirA, navirA));
    K->read(psio_, PSIF_DFOCC_INTS, true, true);

    // Form (ia|bc)
    J1->gemm(false, false, L, K, 1.0, 0.0);
    K.reset();

    // B(Q,a>=b)
    K = SharedTensor2d(new Tensor2d("DF_BASIS_CC B (Q|AB)", nQ, ntri_abAA));
    K->read(psio_, PSIF_DFOCC_INTS);
    Jt = SharedTensor2d(new Tensor2d("J[I] <A|B>=C", navirA, ntri_abAA));

    // main loop
    E_t = 0.0;
    for(int i = 0 ; i < naoccA; ++i){
        double Di = FockA->get(i + nfrzc, i + nfrzc);
        for(int j = 0 ; j <= i; ++j){
	    double Dij = Di + FockA->get(j + nfrzc, j + nfrzc);
            for(int k = 0 ; k <= j; ++k){

                // W[ijk](ab,c) = \sum(e) t_jk^ec (ia|be) (1+)
                // W[ijk](ab,c) = \sum(e) J[i](ab,e) T[jk](ec)
                W->contract(false, false, navirA*navirA, navirA, navirA, J1, T, i*navirA*navirA*navirA, (j*naoccA*navirA*navirA) + (k*navirA*navirA), 1.0, 0.0);

                // W[ijk](ab,c) -= \sum(m) t_im^ab <jk|mc> (1-)
                // W[ijk](ab,c) -= \sum(m) T[i](m,ab) I[jk](mc)
                W->contract(true, false, navirA*navirA, navirA, naoccA, T, I, i*naoccA*navirA*navirA, (j*naoccA*naoccA*navirA) + (k*naoccA*navirA), -1.0, 1.0);

                // W[ijk](ac,b) = \sum(e) t_kj^eb (ia|ce) (2+)
                // W[ijk](ac,b) = \sum(e) J[i](ac,e) T[kj](eb)
                V->contract(false, false, navirA*navirA, navirA, navirA, J1, T, i*navirA*navirA*navirA, (k*naoccA*navirA*navirA) + (j*navirA*navirA), 1.0, 0.0);

                // W[ijk](ac,b) -= \sum(m) t_im^ac <kj|mb> (2-)
                // W[ijk](ac,b) -= \sum(m) T[i](m,ac) I[kj](mb)
                V->contract(true, false, navirA*navirA, navirA, naoccA, T, I, i*naoccA*navirA*navirA, (k*naoccA*naoccA*navirA) + (j*naoccA*navirA), -1.0, 1.0);
                #pragma omp parallel for
                for(int a = 0 ; a < navirA; ++a){
                    for(int b = 0 ; b < navirA; ++b){
		        W->axpy((ULI)navirA, a*navirA*navirA + b, navirA, V, a*navirA*navirA + b*navirA, 1, 1.0);
		    }
		}

                // W[ijk](ba,c) = \sum(e) t_ik^ec (jb|ae) (3+)
                // W[ijk](ba,c) = \sum(e) J[j](ba,e) T[ik](ec)
                V->contract(false, false, navirA*navirA, navirA, navirA, J1, T, j*navirA*navirA*navirA, (i*naoccA*navirA*navirA) + (k*navirA*navirA), 1.0, 0.0);

                // W[ijk](ba,c) -= \sum(m) t_jm^ba <ik|mc> (3-)
                // W[ijk](ba,c) -= \sum(m) T[j](m,ba) I[ik](mc)
                V->contract(true, false, navirA*navirA, navirA, naoccA, T, I, j*naoccA*navirA*navirA, (i*naoccA*naoccA*navirA) + (k*naoccA*navirA), -1.0, 1.0);
		#pragma omp parallel for
                for(int a = 0 ; a < navirA; ++a){
                    for(int b = 0 ; b < navirA; ++b){
		        W->axpy((ULI)navirA, b*navirA*navirA + a*navirA, 1, V, a*navirA*navirA + b*navirA, 1, 1.0);
		    }
		}

                // W[ijk](bc,a) = \sum(e) t_ki^ea (jb|ce) (4+)
                // W[ijk](bc,a) = \sum(e) J[j](bc,e) T[ki](ea)
                V->contract(false, false, navirA*navirA, navirA, navirA, J1, T, j*navirA*navirA*navirA, (k*naoccA*navirA*navirA) + (i*navirA*navirA), 1.0, 0.0);

                // W[ijk](bc,a) -= \sum(m) t_jm^bc <ki|ma> (4-)
                // W[ijk](bc,a) -= \sum(m) T[j](m,bc) I[ki](ma)
                V->contract(true, false, navirA*navirA, navirA, naoccA, T, I, j*naoccA*navirA*navirA, (k*naoccA*naoccA*navirA) + (i*naoccA*navirA), -1.0, 1.0);
		#pragma omp parallel for
                for(int a = 0 ; a < navirA; ++a){
                    for(int b = 0 ; b < navirA; ++b){
		        W->axpy((ULI)navirA, b*navirA*navirA + a, navirA, V, a*navirA*navirA + b*navirA, 1, 1.0);
		    }
		}

                // W[ijk](ca,b) = \sum(e) t_ij^eb (kc|ae) (5+)
                // W[ijk](ca,b) = \sum(e) J[k](ca,e) T[ij](eb)
                V->contract(false, false, navirA*navirA, navirA, navirA, J1, T, k*navirA*navirA*navirA, (i*naoccA*navirA*navirA) + (j*navirA*navirA), 1.0, 0.0);

                // W[ijk](ca,b) -= \sum(m) t_km^ca <ij|mb> (5-)
                // W[ijk](ca,b) -= \sum(m) T[k](m,ca) I[ij](mb)
                V->contract(true, false, navirA*navirA, navirA, naoccA, T, I, k*naoccA*navirA*navirA, (i*naoccA*naoccA*navirA) + (j*naoccA*navirA), -1.0, 1.0);
		#pragma omp parallel for
                for(int a = 0 ; a < navirA; ++a){
                    for(int b = 0 ; b < navirA; ++b){
		        W->axpy((ULI)navirA, a*navirA + b, navirA*navirA, V, a*navirA*navirA + b*navirA, 1, 1.0);
		    }
		}

                // W[ijk](cb,a) = \sum(e) t_ji^ea (kc|be) (6+)
                // W[ijk](cb,a) = \sum(e) J[k](cb,e) T[ji](ea)
                V->contract(false, false, navirA*navirA, navirA, navirA, J1, T, k*navirA*navirA*navirA, (j*naoccA*navirA*navirA) + (i*navirA*navirA), 1.0, 0.0);

                // W[ijk](cb,a) -= \sum(m) t_km^cb <ji|ma> (6-)
                // W[ijk](cb,a) -= \sum(m) T[k](m,cb) I[ji](ma)
                V->contract(true, false, navirA*navirA, navirA, naoccA, T, I, k*naoccA*navirA*navirA, (j*naoccA*naoccA*navirA) + (i*naoccA*navirA), -1.0, 1.0);
		#pragma omp parallel for
                for(int a = 0 ; a < navirA; ++a){
                    for(int b = 0 ; b < navirA; ++b){
		        W->axpy((ULI)navirA, b*navirA + a, navirA*navirA, V, a*navirA*navirA + b*navirA, 1, 1.0);
		    }
		}

		// V[ijk](ab,c) = W[ijk](ab,c) 
		V->copy(W);

		// V[ijk](ab,c) += t_i^a (jb|kc) + t_j^b (ia|kc) + t_k^c (ia|jb)
		// Vt[ijk](ab,c) = V[ijk](ab,c) / (1 + \delta(abc))
                #pragma omp parallel for
                for(int a = 0 ; a < navirA; ++a){
	            int ia = ia_idxAA->get(i,a);
                    for(int b = 0 ; b < navirA; ++b){
			int jb = ia_idxAA->get(j,b);
			int ab = ab_idxAA->get(a,b);
                        for(int c = 0 ; c < navirA; ++c){
			    int kc = ia_idxAA->get(k,c);
			    double value = V->get(ab,c) + ( t1A->get(i,a)*J->get(jb,kc) ) + ( t1A->get(j,b)*J->get(ia,kc) ) + ( t1A->get(k,c)*J->get(ia,jb) );
			    double denom =  1 + ( (a==b) + (b==c) + (a==c) );
			    V->set(ab, c, value/denom);
			}
		    }
		}

		// Denom
		double Dijk = Dij + FockA->get(k + nfrzc, k + nfrzc);
	        double factor =  2 - ( (i==j) + (j==k) + (i==k) );

		// Compute energy
                #pragma omp parallel for
                for(int a = 0 ; a < navirA; ++a){
	            double Dijka = Dijk - FockA->get(a + noccA, a + noccA);
                    for(int b = 0 ; b <=a; ++b){
			double Dijkab = Dijka - FockA->get(b + noccA, b + noccA);
			int ab = ab_idxAA->get(a,b);
			int ba = ab_idxAA->get(b,a);
                        for(int c = 0 ; c <= b; ++c){
			    int ac = ab_idxAA->get(a,c);
			    int bc = ab_idxAA->get(b,c);
			    int ca = ab_idxAA->get(c,a);
			    int cb = ab_idxAA->get(c,b);

			    // X_ijk^abc
			    double Xvalue = ( W->get(ab,c)*V->get(ab,c) ) + ( W->get(ac,b)*V->get(ac,b) ) 
				         + ( W->get(ba,c)*V->get(ba,c) ) + ( W->get(bc,a)*V->get(bc,a) )
					 + ( W->get(ca,b)*V->get(ca,b) ) + ( W->get(cb,a)*V->get(cb,a) );

			    // Y_ijk^abc
			    double Yvalue = V->get(ab,c) + V->get(bc,a) + V->get(ca,b);

			    // Z_ijk^abc
			    double Zvalue = V->get(ac,b) + V->get(ba,c) + V->get(cb,a);

			    // contributions to energy
			    double value = (Yvalue - (2.0*Zvalue)) * ( W->get(ab,c) + W->get(bc,a) + W->get(ca,b) ) ;
			    value += (Zvalue - (2.0*Yvalue)) * ( W->get(ac,b) + W->get(ba,c) + W->get(cb,a) ) ;
			    value += 3.0 * Xvalue;
			    double Dijkabc = Dijkab - FockA->get(c + noccA, c + noccA);
			    E_t += (value * factor) / Dijkabc;
			}
		    }
		}

	    }//k
	}//j
    }//i
    J1.reset();
    T.reset();
    J.reset();
    W.reset();
    V.reset();
    K.reset();
    L.reset();
    I.reset();
 
    // set energy
    Eccsd_t = Eccsd + E_t;

}// end ccsd_canonic_triples_hm



}} // End Namespaces


