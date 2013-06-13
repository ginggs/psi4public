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

/*! \file
    \ingroup CCDENSITY
    \brief Enter brief description of file here 
*/
#include <cstdio>
#include <libdpd/dpd.h>
#include "MOInfo.h"
#include "Params.h"
#include "Frozen.h"
#define EXTERN
#include "globals.h"

namespace psi { namespace ccdensity {

/* computes non R0 parts of EOM CCSD Gijab */

extern void x_Gijab_uhf(void);
void x_Gijab_rohf(void);
void x_Gijab_rohf_2(void);
void x_Gijab_rohf_3(void);

void x_Gijab(void) {
  if (params.ref == 0 || params.ref == 1)
    x_Gijab_rohf();
  else
    x_Gijab_uhf();
  return;
}

void x_Gijab_rohf(void)
{
  int h, nirreps;
  int R_irr, L_irr, G_irr;
  double value, tval;
  dpdfile2 T1, L1, I1, T1A, T1B, Z1, R1;
  dpdbuf4 R, I, G, L, T, V, Z, Z2;

  nirreps = moinfo.nirreps;
  R_irr = params.R_irr; L_irr = params.L_irr; G_irr = params.G_irr;

  /* term 1,2: (L*R) * Tau(IJ,AB), see comments in xi_connected */
  if (G_irr == 0) {
    dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 2, 7, 2, 7, 0, "tauIJAB");
    dpd_->buf4_copy(&T, PSIF_EOM_TMP0, "GIJAB");
    dpd_->buf4_close(&T);
    dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 2, 7, 2, 7, 0, "tauijab");
    dpd_->buf4_copy(&T, PSIF_EOM_TMP0, "Gijab");
    dpd_->buf4_close(&T);
    dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 0, 5, 0, 5, 0, "tauIjAb");
    dpd_->buf4_copy(&T, PSIF_EOM_TMP0, "GIjAb");
    dpd_->buf4_close(&T);
  }
  else {
    dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 2, 7, 2, 7, 0, "GIJAB");
    dpd_->buf4_scm(&G,0.0);
    dpd_->buf4_close(&G);
    dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 2, 7, 2, 7, 0, "Gijab");
    dpd_->buf4_scm(&G,0.0);
    dpd_->buf4_close(&G);
    dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 5, 0, 5, 0, "GIjAb");
    dpd_->buf4_scm(&G,0.0);
    dpd_->buf4_close(&G);
  }

  /* -P(ij) LR_OO(M,I) Tau(MJ,AB); terms 4,5,8,9 */
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 7, 0, 7, 0, "Z(IJ,A>B)");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, G_irr, 0, 0, "LR_OO");
  dpd_->file2_init(&Z1, PSIF_EOM_TMP1, G_irr, 0, 0, "Z(N,I)");
  dpd_->file2_axpy(&I1, &Z1, 1.0, 0);
  dpd_->file2_close(&I1);
  /* -P(ij) L2R1_OV(M,F) T(I,F) Tau(MJ,AB); terms 35, 36 */
  if (!params.connect_xi) {
  dpd_->file2_init(&I1, PSIF_EOM_TMP, G_irr, 0, 1, "L2R1_OV");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract222(&I1, &T1, &Z1, 0, 0, 1.0, 1.0);
  dpd_->file2_close(&T1);
  dpd_->file2_close(&I1);
  }
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 0, 7, 2, 7, 0, "tauIJAB");
  dpd_->contract244(&Z1, &T, &Z, 0, 0, 0, 1.0, 0.0);
  dpd_->file2_close(&Z1);
  dpd_->buf4_close(&T);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 7, 2, 7, 0, "GIJAB");
  dpd_->buf4_axpy(&Z, &G, -1.0);
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, qprs, 0, 7, "Z(JI,A>B)");
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 7, 0, 7, 0, "Z(JI,A>B)");
  dpd_->buf4_axpy(&Z, &G, 1.0);
  dpd_->buf4_close(&Z);
  dpd_->buf4_close(&G);

  /* -P(ij) LR_oo(m,i) Tau(mj,ab); terms 4,5,8,9 */
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 7, 0, 7, 0, "Z(ij,a>b)");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, G_irr, 0, 0, "LR_oo");
  dpd_->file2_init(&Z1, PSIF_EOM_TMP1, G_irr, 0, 0, "Z(n,i)");
  dpd_->file2_axpy(&I1, &Z1, 1.0, 0);
  dpd_->file2_close(&I1);
  /* -P(ij) L2R1_ov(m,f) T(i,f) Tau(mj,ab); terms 35, 36 */
  if (!params.connect_xi) {
  dpd_->file2_init(&I1, PSIF_EOM_TMP, G_irr, 0, 1, "L2R1_ov");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract222(&I1, &T1, &Z1, 0, 0, 1.0, 1.0);
  dpd_->file2_close(&T1);
  dpd_->file2_close(&I1);
  }
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 0, 7, 2, 7, 0, "tauijab");
  dpd_->contract244(&Z1, &T, &Z, 0, 0, 0, 1.0, 0.0);
  dpd_->file2_close(&Z1);
  dpd_->buf4_close(&T);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 7, 2, 7, 0, "Gijab");
  dpd_->buf4_axpy(&Z, &G, -1.0);
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, qprs, 0, 7, "Z(ji,a>b)");
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 7, 0, 7, 0, "Z(ji,a>b)");
  dpd_->buf4_axpy(&Z, &G, 1.0);
  dpd_->buf4_close(&Z);
  dpd_->buf4_close(&G);
  psio_close(PSIF_EOM_TMP1, 0);
  psio_open(PSIF_EOM_TMP1,PSIO_OPEN_NEW);

  /* GIjAb += -P(Ij) LR_OO(M,I) Tau(Mj,Ab); terms 4,5,8,9 */
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 5, 0, 5, 0, "GIjAb");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, G_irr, 0, 0, "LR_OO");
  dpd_->file2_init(&Z1, PSIF_EOM_TMP1, G_irr, 0, 0, "Z(N,I)");
  dpd_->file2_axpy(&I1, &Z1, 1.0, 0);
  dpd_->file2_close(&I1);
  /* -P(Ij) L2R1_OV(M,F) T(I,F) Tau(Nm,Ab); terms 35, 36 */
  if (!params.connect_xi) {
  dpd_->file2_init(&I1, PSIF_EOM_TMP, G_irr, 0, 1, "L2R1_OV");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract222(&I1, &T1, &Z1, 0, 0, 1.0, 1.0);
  dpd_->file2_close(&T1);
  dpd_->file2_close(&I1);
  }
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 0, 5, 0, 5, 0, "tauIjAb");
  dpd_->contract244(&Z1, &T, &G, 0, 0, 0, -1.0, 1.0);
  dpd_->file2_close(&Z1);
  dpd_->buf4_close(&T);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 5, 0, 5, 0, "Z(jI,bA)");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, G_irr, 0, 0, "LR_oo");
  dpd_->file2_init(&Z1, PSIF_EOM_TMP1, G_irr, 0, 0, "Z(n,i)");
  dpd_->file2_axpy(&I1, &Z1, 1.0, 0);
  dpd_->file2_close(&I1);
  if (!params.connect_xi) {
  dpd_->file2_init(&I1, PSIF_EOM_TMP, G_irr, 0, 1, "L2R1_ov");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract222(&I1, &T1, &Z1, 0, 0, 1.0, 1.0);
  dpd_->file2_close(&T1);
  dpd_->file2_close(&I1);
  }
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 0, 5, 0, 5, 0, "tauiJaB");
  dpd_->contract244(&Z1, &T, &Z, 0, 0, 0, 1.0, 0.0);
  dpd_->file2_close(&Z1);
  dpd_->buf4_close(&T);
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, qpsr, 0, 5, "Z(Ij,Ab)");
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 5, 0, 5, 0, "Z(Ij,Ab)");
  dpd_->buf4_axpy(&Z, &G, -1.0);
  dpd_->buf4_close(&G);

  psio_close(PSIF_EOM_TMP1,0);
  psio_open(PSIF_EOM_TMP1,PSIO_OPEN_NEW);

  /* -P(ab) LR_VV(F,A) Tau(IJ,FB); terms 6,7,10,11 */
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 2, 5, 2, 5, 0, "Z(I>J,AB)");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 2, 5, 2, 7, 0, "tauIJAB");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, G_irr, 1, 1, "LR_VV");
  dpd_->contract244(&I1, &T, &Z, 0, 2, 1, 1.0, 0.0);
  dpd_->file2_close(&I1);
  dpd_->buf4_close(&T);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 2, 5, 2, 7, 0, "GIJAB");
  dpd_->buf4_axpy(&Z, &G, -1.0);
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, pqsr, 2, 5, "Z(I>J,BA)");
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 2, 5, 2, 5, 0, "Z(I>J,BA)");
  dpd_->buf4_axpy(&Z, &G, 1.0);
  dpd_->buf4_close(&Z);
  dpd_->buf4_close(&G);
  /* -P(ab) LR_vv(f,a) Tau(ij,fb); */
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 2, 5, 2, 5, 0, "Z(i>j,ab)");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 2, 5, 2, 7, 0, "tauijab");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, G_irr, 1, 1, "LR_vv");
  dpd_->contract244(&I1, &T, &Z, 0, 2, 1, 1.0, 0.0);
  dpd_->file2_close(&I1);
  dpd_->buf4_close(&T);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 2, 5, 2, 7, 0, "Gijab");
  dpd_->buf4_axpy(&Z, &G, -1.0);
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, pqsr, 2, 5, "Z(i>j,ba)");
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 2, 5, 2, 5, 0, "Z(i>j,ba)");
  dpd_->buf4_axpy(&Z, &G, 1.0);
  dpd_->buf4_close(&Z);
  dpd_->buf4_close(&G);
  /* GIjAb += - LR_VV(F,A) Tau(Ij,Fb) + LR_VV(f,a) Tau(Ij,fA) */
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 5, 0, 5, 0, "GIjAb");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 0, 5, 0, 5, 0, "tauIjAb");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, G_irr, 1, 1, "LR_VV");
  dpd_->contract244(&I1, &T, &G, 0, 2, 1, -1.0, 1.0);
  dpd_->file2_close(&I1);
  dpd_->buf4_close(&T);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 5, 0, 5, 0, "Z(Ij,bA)");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 0, 5, 0, 5, 0, "tauIjbA");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, G_irr, 1, 1, "LR_vv");
  dpd_->contract244(&I1, &T, &Z, 0, 2, 1, 1.0, 0.0);
  dpd_->file2_close(&I1);
  dpd_->buf4_close(&T);
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, pqsr, 0, 5, "Z(Ij,Ab)");
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 5, 0, 5, 0, "Z(Ij,Ab)");
  dpd_->buf4_axpy(&Z, &G, -1.0);
  dpd_->buf4_close(&G);

  psio_close(PSIF_EOM_TMP1,0);
  psio_open(PSIF_EOM_TMP1,PSIO_OPEN_NEW);

  /* + 1/4 Lmnef Rmnab Tau_ijef, terms 13, 15 */
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 2, 7, 2, 7, 0, "GIJAB");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, L_irr, 2, 2, 2, 2, 0, "Tau2L2_OOOO");
  dpd_->buf4_init(&R, PSIF_CC_GR, R_irr, 2, 7, 2, 7, 0, "RIJAB");
  dpd_->contract444(&I, &R, &G, 0, 1, 1.0, 1.0);
  dpd_->buf4_close(&R);
  dpd_->buf4_close(&I);
  dpd_->buf4_close(&G);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 2, 7, 2, 7, 0, "Gijab");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, L_irr, 2, 2, 2, 2, 0, "Tau2L2_oooo");
  dpd_->buf4_init(&R, PSIF_CC_GR, R_irr, 2, 7, 2, 7, 0, "Rijab");
  dpd_->contract444(&I, &R, &G, 0, 1, 1.0, 1.0);
  dpd_->buf4_close(&R);
  dpd_->buf4_close(&I);
  dpd_->buf4_close(&G);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 5, 0, 5, 0, "GIjAb");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, L_irr, 0, 0, 0, 0, 0, "Tau2L2_OoOo");
  dpd_->buf4_init(&R, PSIF_CC_GR, R_irr, 0, 5, 0, 5, 0, "RIjAb");
  dpd_->contract444(&I, &R, &G, 0, 1, 1.0, 1.0);
  dpd_->buf4_close(&R);
  dpd_->buf4_close(&I);
  dpd_->buf4_close(&G);

  /* - 0.5 P(ij) (Lmnfe Rie) Tjf (taumnab), terms 24, 26 */
  /* + 1/4 Lmnef Rijef Tau_mnab, terms 12, 14 */
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 2, 0, 2, 0, "Z(IJ,M>N)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 2, 10, 2, 10, 0, "L2R1_OOVO(pqsr)");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract424(&I, &T1, &Z, 3, 1, 1, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&I);
  /* add terms 12, 14 */
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 0, 2, 2, 2, 0, "R2L2_OOOO");
  dpd_->buf4_axpy(&I, &Z, -0.5);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 0, 7, 0, 7, 0, "Z(IJ,A>B)");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 2, 7, 2, 7, 0, "tauIJAB");
  dpd_->contract444(&Z, &T, &Z2, 0, 1, 1.0, 0.0);
  dpd_->buf4_close(&T);
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 7, 2, 7, 0, "GIJAB");
  dpd_->buf4_axpy(&Z2, &G, -1.0);
  dpd_->buf4_sort(&Z2, PSIF_EOM_TMP1, qprs, 0, 7, "Z(JI,A>B)");
  dpd_->buf4_close(&Z2);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 0, 7, 0, 7, 0, "Z(JI,A>B)");
  dpd_->buf4_axpy(&Z2, &G, 1.0);
  dpd_->buf4_close(&Z2);
  dpd_->buf4_close(&G);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 2, 0, 2, 0, "Z(ij,m>n)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 2, 10, 2, 10, 0, "L2R1_oovo(pqsr)");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract424(&I, &T1, &Z, 3, 1, 1, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&I);
  /* add terms 12, 14 */
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 0, 2, 2, 2, 0, "R2L2_oooo");
  dpd_->buf4_axpy(&I, &Z, -0.5);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 0, 7, 0, 7, 0, "Z(ij,a>b)");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 2, 7, 2, 7, 0, "tauijab");
  dpd_->contract444(&Z, &T, &Z2, 0, 1, 1.0, 0.0);
  dpd_->buf4_close(&T);
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 7, 2, 7, 0, "Gijab");
  dpd_->buf4_axpy(&Z2, &G, -1.0);
  dpd_->buf4_sort(&Z2, PSIF_EOM_TMP1, qprs, 0, 7, "Z(ji,a>b)");
  dpd_->buf4_close(&Z2);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 0, 7, 0, 7, 0, "Z(ji,a>b)");
  dpd_->buf4_axpy(&Z2, &G, 1.0);
  dpd_->buf4_close(&Z2);
  dpd_->buf4_close(&G);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 0, 0, 0, 0, "Z(Ij,Mn)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 0, 10, 0, 10, 0, "L2R1_OovO(pqsr)");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract424(&I, &T1, &Z, 3, 1, 1, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 0, 11, 0, 11, 0, "L2R1_OoVo");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract244(&T1, &I, &Z, 1, 2, 0, 1.0, 1.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&I);
  /* add terms 12, 14 */
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 0, 0, 0, 0, 0, "R2L2_OoOo");
  dpd_->buf4_axpy(&I, &Z, 1.0);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 5, 0, 5, 0, "GIjAb");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 0, 5, 0, 5, 0, "tauIjAb");
  dpd_->contract444(&Z, &T, &G, 0, 1, 1.0, 1.0);
  dpd_->buf4_close(&T);
  dpd_->buf4_close(&Z);
  dpd_->buf4_close(&G);

  psio_close(PSIF_EOM_TMP1,0);
  psio_open(PSIF_EOM_TMP1,PSIO_OPEN_NEW);

  /* + 0.5 P(AB) (tau_IJEF LMNEF) RMA TNB ; terms 25, 27 */
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 2, 11, 2, 11, 0, "Z(I>J,AN)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, L_irr, 2, 0, 2, 2, 0, "Tau2L2_OOOO");
  dpd_->file2_init(&R1, PSIF_CC_GR, R_irr, 0, 1, "RIA");
  dpd_->contract244(&R1, &I, &Z, 0, 2, 1, 1.0, 0.0);
  dpd_->file2_close(&R1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 2, 5, 2, 5, 0, "Z(I>J,AB)");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract424(&Z, &T1, &Z2, 3, 0, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 2, 5, 2, 7, 0, "GIJAB");
  dpd_->buf4_axpy(&Z2, &G, 1.0);
  dpd_->buf4_sort(&Z2, PSIF_EOM_TMP1, pqsr, 2, 5, "Z(I>J,BA)");
  dpd_->buf4_close(&Z2);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 2, 5, 2, 5, 0, "Z(I>J,BA)");
  dpd_->buf4_axpy(&Z2, &G, -1.0);
  dpd_->buf4_close(&Z2);
  dpd_->buf4_close(&G);
  /* + 0.5 P(ab) (tau_ijef Lmnef) Rma Tnb ; terms 25, 27 */
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 2, 11, 2, 11, 0, "Z(i>j,an)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, L_irr, 2, 0, 2, 2, 0, "Tau2L2_oooo");
  dpd_->file2_init(&R1, PSIF_CC_GR, R_irr, 0, 1, "Ria");
  dpd_->contract244(&R1, &I, &Z, 0, 2, 1, 1.0, 0.0);
  dpd_->file2_close(&R1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 2, 5, 2, 5, 0, "Z(i>j,ab)");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract424(&Z, &T1, &Z2, 3, 0, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 2, 5, 2, 7, 0, "Gijab");
  dpd_->buf4_axpy(&Z2, &G, 1.0);
  dpd_->buf4_sort(&Z2, PSIF_EOM_TMP1, pqsr, 2, 5, "Z(i>j,ba)");
  dpd_->buf4_close(&Z2);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 2, 5, 2, 5, 0, "Z(i>j,ba)");
  dpd_->buf4_axpy(&Z2, &G, -1.0);
  dpd_->buf4_close(&Z2);
  dpd_->buf4_close(&G);
  /* + tau_IjEf LMnEf RMA Tnb ; terms 25, 27 */
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 11, 0, 11, 0, "Z(Ij,An)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, L_irr, 0, 0, 0, 0, 0, "Tau2L2_OoOo");
  dpd_->file2_init(&R1, PSIF_CC_GR, R_irr, 0, 1, "RIA");
  dpd_->contract244(&R1, &I, &Z, 0, 2, 1, 1.0, 0.0);
  dpd_->file2_close(&R1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 5, 0, 5, 0, "GIjAb");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract424(&Z, &T1, &G, 3, 0, 0, 1.0, 1.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&Z);
  /* + tau_IjEf LNmEf Rmb TNA ; terms 25, 27 */
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, L_irr, 0, 11, 0, 11, 0, "Z2(Ij,An)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, L_irr, 0, 0, 0, 0, 0, "Tau2L2_OoOo");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract244(&T1, &I, &Z, 0, 2, 1, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&I);
  dpd_->file2_init(&R1, PSIF_CC_GR, R_irr, 0, 1, "Ria");
  dpd_->contract424(&Z, &R1, &G, 3, 0, 0, 1.0, 1.0);
  dpd_->file2_close(&R1);
  dpd_->buf4_close(&G);

  psio_close(PSIF_EOM_TMP1,0);
  psio_open(PSIF_EOM_TMP1,PSIO_OPEN_NEW);

  /* terms combined to P(ij)P(ab) Z(i,a) T(j,b), 3,22,23,33 */
  x_Gijab_rohf_2();
  /* terms combined to P(ij)P(ab) Z(i,a) R(j,b), 18,32,34,19 */
  x_Gijab_rohf_3();

  /* -P(ij)(Lme Tie + 0.5 Lmnef Tinef) Rmjab, term 16, 30 */
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 7, 0, 7, 0, "Z(IJ,A>B)");
  dpd_->buf4_init(&R, PSIF_CC_GR, R_irr, 0, 7, 2, 7, 0, "RIJAB");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, L_irr, 0, 0, "LT_OO");
  dpd_->contract244(&I1, &R, &Z, 0, 0, 0, 1.0, 0.0);
  dpd_->file2_close(&I1);
  dpd_->buf4_close(&R);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 7, 2, 7, 0, "GIJAB");
  dpd_->buf4_axpy(&Z, &G, -1.0);
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, qprs, 0, 7, "Z(JI,A>B)");
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 7, 0, 7, 0, "Z(JI,A>B)");
  dpd_->buf4_axpy(&Z, &G, 1.0);
  dpd_->buf4_close(&Z);
  dpd_->buf4_close(&G);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 7, 0, 7, 0, "Z(ij,a>b)");
  dpd_->buf4_init(&R, PSIF_CC_GR, R_irr, 0, 7, 2, 7, 0, "Rijab");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, L_irr, 0, 0, "LT_oo");
  dpd_->contract244(&I1, &R, &Z, 0, 0, 0, 1.0, 0.0);
  dpd_->file2_close(&I1);
  dpd_->buf4_close(&R);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 7, 2, 7, 0, "Gijab");
  dpd_->buf4_axpy(&Z, &G, -1.0);
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, qprs, 0, 7, "Z(ji,a>b)");
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 7, 0, 7, 0, "Z(ji,a>b)");
  dpd_->buf4_axpy(&Z, &G, 1.0);
  dpd_->buf4_close(&Z);
  dpd_->buf4_close(&G);

  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 5, 0, 5, 0, "GIjAb");
  dpd_->buf4_init(&R, PSIF_CC_GR, R_irr, 0, 5, 0, 5, 0, "RIjAb");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, L_irr, 0, 0, "LT_OO");
  dpd_->contract244(&I1, &R, &G, 0, 0, 0, -1.0, 1.0);
  dpd_->file2_close(&I1);
  dpd_->buf4_close(&R);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 5, 0, 5, 0, "Z(jI,Ab)");
  dpd_->buf4_init(&R, PSIF_CC_GR, R_irr, 0, 5, 0, 5, 0, "RiJAb");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, L_irr, 0, 0, "LT_oo");
  dpd_->contract244(&I1, &R, &Z, 0, 0, 0, 1.0, 0.0);
  dpd_->file2_close(&I1);
  dpd_->buf4_close(&R);
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, qprs, 0, 5, "Z(Ij,Ab)");
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 5, 0, 5, 0, "Z(Ij,Ab)");
  dpd_->buf4_axpy(&Z, &G, -1.0);
  dpd_->buf4_close(&Z);
  dpd_->buf4_close(&G);

  psio_close(PSIF_EOM_TMP1,0);
  psio_open(PSIF_EOM_TMP1, PSIO_OPEN_NEW);

  /* -P(ab)(Lme Tmb + 0.5 Lmnfe Tmnfb) Rijae, term 17, 31 */
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 2, 5, 2, 5, 0, "Z(I>J,AB)");
  dpd_->buf4_init(&R, PSIF_CC_GR, R_irr, 2, 5, 2, 7, 0, "RIJAB");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, L_irr, 1, 1, "LT_VV");
  dpd_->contract424(&R, &I1, &Z, 3, 0, 0, 1.0, 0.0);
  dpd_->file2_close(&I1);
  dpd_->buf4_close(&R);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 2, 5, 2, 7, 0, "GIJAB");
  dpd_->buf4_axpy(&Z, &G, -1.0);
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, pqsr, 2, 5, "Z(I>J,BA)");
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 2, 5, 2, 5, 0, "Z(I>J,BA)");
  dpd_->buf4_axpy(&Z, &G, 1.0);
  dpd_->buf4_close(&Z);
  dpd_->buf4_close(&G);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 2, 5, 2, 5, 0, "Z(i>j,ab)");
  dpd_->buf4_init(&R, PSIF_CC_GR, R_irr, 2, 5, 2, 7, 0, "Rijab");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, L_irr, 1, 1, "LT_vv");
  dpd_->contract424(&R, &I1, &Z, 3, 0, 0, 1.0, 0.0);
  dpd_->file2_close(&I1);
  dpd_->buf4_close(&R);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 2, 5, 2, 7, 0, "Gijab");
  dpd_->buf4_axpy(&Z, &G, -1.0);
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, pqsr, 2, 5, "Z(i>j,ba)");
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 2, 5, 2, 5, 0, "Z(i>j,ba)");
  dpd_->buf4_axpy(&Z, &G, 1.0);
  dpd_->buf4_close(&Z);
  dpd_->buf4_close(&G);

  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 5, 0, 5, 0, "GIjAb");
  dpd_->buf4_init(&R, PSIF_CC_GR, R_irr, 0, 5, 0, 5, 0, "RIjAb");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, L_irr, 1, 1, "LT_vv");
  dpd_->contract424(&R, &I1, &G, 3, 0, 0, -1.0, 1.0);
  dpd_->file2_close(&I1);
  dpd_->buf4_close(&R);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 5, 0, 5, 0, "Z(Ij,bA)");
  dpd_->buf4_init(&R, PSIF_CC_GR, R_irr, 0, 5, 0, 5, 0, "RIjaB");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, L_irr, 1, 1, "LT_VV");
  dpd_->contract424(&R, &I1, &Z, 3, 0, 0, 1.0, 0.0);
  dpd_->file2_close(&I1);
  dpd_->buf4_close(&R);
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, pqsr, 0, 5, "Z(Ij,Ab)");
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 5, 0, 5, 0, "Z(Ij,Ab)");
  dpd_->buf4_axpy(&Z, &G, -1.0);
  dpd_->buf4_close(&Z);
  dpd_->buf4_close(&G);

  psio_close(PSIF_EOM_TMP1,0);
  psio_open(PSIF_EOM_TMP1, PSIO_OPEN_NEW);

  /* -P(ab) lmnef rme tijfb tna, term 37 */
  if (!params.connect_xi) {
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 2, 11, 2, 11, 0, "Z(I>J,AN)");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 2, 5, 2, 7, 0, "tIJAB");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, G_irr, 0, 1, "L2R1_OV");
  dpd_->contract424(&T, &I1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&I1);
  dpd_->buf4_close(&T);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 2, 5, 2, 5, 0, "Z(I>J,AB)");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract424(&Z, &T1, &Z2, 3, 0, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_close(&Z2);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 2, 5, 2, 7, 0, "GIJAB");
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, 0, 2, 5, 2, 5, 0, "Z(I>J,AB)");
  dpd_->buf4_axpy(&Z, &G, -1.0);
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, pqsr, 2, 5, "Z(I>J,BA)");
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, 0, 2, 5, 2, 5, 0, "Z(I>J,BA)");
  dpd_->buf4_axpy(&Z, &G, 1.0);
  dpd_->buf4_close(&G);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 2, 11, 2, 11, 0, "Z(i>j,an)");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 2, 5, 2, 7, 0, "tijab");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, G_irr, 0, 1, "L2R1_ov");
  dpd_->contract424(&T, &I1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&I1);
  dpd_->buf4_close(&T);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 2, 5, 2, 5, 0, "Z(i>j,ab)");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract424(&Z, &T1, &Z2, 3, 0, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_close(&Z2);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 2, 5, 2, 7, 0, "Gijab");
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 2, 5, 2, 5, 0, "Z(i>j,ab)");
  dpd_->buf4_axpy(&Z, &G, -1.0);
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, pqsr, 2, 5, "Z(i>j,ba)");
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 2, 5, 2, 5, 0, "Z(i>j,ba)");
  dpd_->buf4_axpy(&Z, &G, 1.0);
  dpd_->buf4_close(&G);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 11, 0, 11, 0, "Z(Ij,An)");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 0, 5, 0, 5, 0, "tIjAb");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, G_irr, 0, 1, "L2R1_ov");
  dpd_->contract424(&T, &I1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&I1);
  dpd_->buf4_close(&T);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 5, 0, 5, 0, "GIjAb");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract424(&Z, &T1, &G, 3, 0, 0, -1.0, 1.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_close(&G);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 10, 0, 10, 0, "Z(Ij,Nb)");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 0, 5, 0, 5, 0, "tIjAb");
  dpd_->file2_init(&I1, PSIF_EOM_TMP, G_irr, 0, 1, "L2R1_OV");
  dpd_->contract244(&I1, &T, &Z, 1, 2, 1, 1.0, 0.0);
  dpd_->file2_close(&I1);
  dpd_->buf4_close(&T);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 5, 0, 5, 0, "GIjAb");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract244(&T1, &Z, &G, 0, 2, 1, -1.0, 1.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_close(&G);
  }

  /* compute Z(IA,JB) Z(ia,jb) and Z(IA,jb) for terms
     20, 28, 29, 21 then permute and add in */
  /* + P(ij) P(ab) (Rimae Lnmfe) Tnjfb, term 20 */
  dpd_->buf4_init(&Z, PSIF_EOM_TMP0, G_irr, 10, 10, 10, 10, 0, "Z(IA,JB)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 10, 10, 10, 10, 0, "R2L2_OVOV");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 10, 10, 10, 10, 0, "tIAJB");
  dpd_->contract444(&I, &T, &Z, 0, 1, 1.0, 0.0);
  dpd_->buf4_close(&I);
  dpd_->buf4_close(&T);
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 10, 10, 10, 10, 0, "R2L2_OVov");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 10, 10, 10, 10, 0, "tiaJB");
  dpd_->contract444(&I, &T, &Z, 0, 1, 1.0, 1.0);
  dpd_->buf4_close(&I);
  dpd_->buf4_close(&T);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP0, G_irr, 10, 10, 10, 10, 0, "Z(ia,jb)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 10, 10, 10, 10, 0, "R2L2_ovov");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 10, 10, 10, 10, 0, "tiajb");
  dpd_->contract444(&I, &T, &Z, 0, 1, 1.0, 0.0);
  dpd_->buf4_close(&I);
  dpd_->buf4_close(&T);
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 10, 10, 10, 10, 0, "R2L2_ovOV");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 10, 10, 10, 10, 0, "tIAjb");
  dpd_->contract444(&I, &T, &Z, 0, 1, 1.0, 1.0);
  dpd_->buf4_close(&I);
  dpd_->buf4_close(&T);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP0, G_irr, 10, 10, 10, 10, 0, "Z(IA,jb)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 10, 10, 10, 10, 0, "R2L2_OVOV");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 10, 10, 10, 10, 0, "tIAjb");
  dpd_->contract444(&I, &T, &Z, 0, 1, 1.0, 0.0);
  dpd_->buf4_close(&I);
  dpd_->buf4_close(&T);
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 10, 10, 10, 10, 0, "R2L2_OVov");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 10, 10, 10, 10, 0, "tiajb");
  dpd_->contract444(&I, &T, &Z, 0, 1, 1.0, 1.0);
  dpd_->buf4_close(&I);
  dpd_->buf4_close(&T);
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 10, 10, 10, 10, 0, "R2L2_ovOV");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 10, 10, 10, 10, 0, "tIAJB");
  dpd_->contract444(&T, &I, &Z, 0, 0, 1.0, 1.0);
  dpd_->buf4_close(&I);
  dpd_->buf4_close(&T);
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 10, 10, 10, 10, 0, "R2L2_ovov");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 10, 10, 10, 10, 0, "tIAjb");
  dpd_->contract444(&T, &I, &Z, 0, 0, 1.0, 1.0);
  dpd_->buf4_close(&I);
  dpd_->buf4_close(&T);
  dpd_->buf4_close(&Z);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 10, 10, 10, 10, 0, "Z(Ib,jA)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 10, 10, 10, 10, 0, "R2L2_oVoV");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 10, 10, 10, 10, 0, "tIbjA");
  dpd_->contract444(&T, &I, &Z, 0, 0, 1.0, 0.0);
  dpd_->buf4_close(&I);
  dpd_->buf4_close(&T);
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 10, 10, 10, 10, 0, "R2L2_OvOv");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 10, 10, 10, 10, 0, "tIbjA");
  dpd_->contract444(&I, &T, &Z, 0, 1, 1.0, 1.0);
  dpd_->buf4_close(&I);
  dpd_->buf4_close(&T);
  dpd_->buf4_sort_axpy(&Z, PSIF_EOM_TMP0, psrq, 10, 10, "Z(IA,jb)", 1.0);
  dpd_->buf4_close(&Z);

  psio_close(PSIF_EOM_TMP1,0);
  psio_open(PSIF_EOM_TMP1,PSIO_OPEN_NEW);

  /* - P(ij) P(ab) (Tjmbe Lnmfe) Tif Rna, term 28 */
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, L_irr, 10, 0, 10, 0, 0, "Z(JB,NI)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, L_irr, 10, 10, 10, 10, 0, "VIAJB");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract424(&I, &T1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 11, 10, 11, 10, 0, "Z(AI,JB)");
  dpd_->file2_init(&R1, PSIF_CC_GR, R_irr, 0, 1, "RIA");
  dpd_->contract244(&R1, &Z, &Z2, 0, 2, 0, 1.0, 0.0);
  dpd_->file2_close(&R1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_sort_axpy(&Z2, PSIF_EOM_TMP0, qprs, 10, 10, "Z(IA,JB)", -1.0);
  dpd_->buf4_close(&Z2);
  
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, L_irr, 10, 0, 10, 0, 0, "Z(jb,ni)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, L_irr, 10, 10, 10, 10, 0, "Viajb");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract424(&I, &T1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 11, 10, 11, 10, 0, "Z(ai,jb)");
  dpd_->file2_init(&R1, PSIF_CC_GR, R_irr, 0, 1, "Ria");
  dpd_->contract244(&R1, &Z, &Z2, 0, 2, 0, 1.0, 0.0);
  dpd_->file2_close(&R1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_sort_axpy(&Z2, PSIF_EOM_TMP0, qprs, 10, 10, "Z(ia,jb)", -1.0);
  dpd_->buf4_close(&Z2);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, L_irr, 10, 0, 10, 0, 0, "Z(jb,NI)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, L_irr, 10, 10, 10, 10, 0, "ViaJB");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract424(&I, &T1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 11, 10, 11, 10, 0, "Z(AI,jb)");
  dpd_->file2_init(&R1, PSIF_CC_GR, R_irr, 0, 1, "RIA");
  dpd_->contract244(&R1, &Z, &Z2, 0, 2, 0, 1.0, 0.0);
  dpd_->file2_close(&R1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_sort_axpy(&Z2, PSIF_EOM_TMP0, qprs, 10, 10, "Z(IA,jb)", -1.0);
  dpd_->buf4_close(&Z2);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, L_irr, 10, 0, 10, 0, 0, "Z(Ib,Nj)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, L_irr, 10, 10, 10, 10, 0, "VIaJb");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract424(&I, &T1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 10, 11, 10, 11, 0, "Z(Ib,Aj)");
  dpd_->file2_init(&R1, PSIF_CC_GR, R_irr, 0, 1, "RIA");
  dpd_->contract244(&R1, &Z, &Z2, 0, 2, 1, 1.0, 0.0);
  dpd_->file2_close(&R1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_sort_axpy(&Z2, PSIF_EOM_TMP0, prsq, 10, 10, "Z(IA,jb)", +1.0);
  dpd_->buf4_close(&Z2);

  psio_close(PSIF_EOM_TMP1,0);
  psio_open(PSIF_EOM_TMP1,PSIO_OPEN_NEW);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, L_irr, 10, 0, 10, 0, 0, "Z(jA,nI)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, L_irr, 10, 10, 10, 10, 0, "ViAjB");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract424(&I, &T1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 11, 10, 11, 10, 0, "Z(bI,jA)");
  dpd_->file2_init(&R1, PSIF_CC_GR, R_irr, 0, 1, "Ria");
  dpd_->contract244(&R1, &Z, &Z2, 0, 2, 0, 1.0, 0.0);
  dpd_->file2_close(&R1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_sort(&Z2, PSIF_EOM_TMP1, sqrp, 11, 10, "Z(AI,jb)");
  dpd_->buf4_close(&Z2);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 11, 10, 11, 10, 0, "Z(AI,jb)");
  dpd_->buf4_sort_axpy(&Z2, PSIF_EOM_TMP0, qprs, 10, 10, "Z(IA,jb)", +1.0);
  dpd_->buf4_close(&Z2);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, L_irr, 10, 0, 10, 0, 0, "Z(IA,nj)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, L_irr, 10, 10, 10, 10, 0, "VIAjb");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract424(&I, &T1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 10, 11, 10, 11, 0, "Z(IA,bj)");
  dpd_->file2_init(&R1, PSIF_CC_GR, R_irr, 0, 1, "Ria");
  dpd_->contract244(&R1, &Z, &Z2, 0, 2, 1, 1.0, 0.0);
  dpd_->file2_close(&R1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_sort_axpy(&Z2, PSIF_EOM_TMP0, pqsr, 10, 10, "Z(IA,jb)", -1.0);
  dpd_->buf4_close(&Z2);

  psio_close(PSIF_EOM_TMP1,0);
  psio_open(PSIF_EOM_TMP1,PSIO_OPEN_NEW);

  /* - P(ij) P(ab) (Tjmbe Lnmfe) Rif Tna, term 29 */
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 10, 0, 10, 0, 0, "Z(JB,NI)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, L_irr, 10, 10, 10, 10, 0, "VIAJB");
  dpd_->file2_init(&R1, PSIF_CC_GR, R_irr, 0, 1, "RIA");
  dpd_->contract424(&I, &R1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&R1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 11, 10, 11, 10, 0, "Z(AI,JB)");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract244(&T1, &Z, &Z2, 0, 2, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_sort_axpy(&Z2, PSIF_EOM_TMP0, qprs, 10, 10, "Z(IA,JB)", -1.0);
  dpd_->buf4_close(&Z2);
  
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 10, 0, 10, 0, 0, "Z(jb,ni)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, L_irr, 10, 10, 10, 10, 0, "Viajb");
  dpd_->file2_init(&R1, PSIF_CC_GR, R_irr, 0, 1, "Ria");
  dpd_->contract424(&I, &R1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&R1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 11, 10, 11, 10, 0, "Z(ai,jb)");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract244(&T1, &Z, &Z2, 0, 2, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_sort_axpy(&Z2, PSIF_EOM_TMP0, qprs, 10, 10, "Z(ia,jb)", -1.0);
  dpd_->buf4_close(&Z2);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 10, 0, 10, 0, 0, "Z(jb,NI)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, L_irr, 10, 10, 10, 10, 0, "ViaJB");
  dpd_->file2_init(&R1, PSIF_CC_GR, R_irr, 0, 1, "RIA");
  dpd_->contract424(&I, &R1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&R1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 11, 10, 11, 10, 0, "Z(AI,jb)");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract244(&T1, &Z, &Z2, 0, 2, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_sort_axpy(&Z2, PSIF_EOM_TMP0, qprs, 10, 10, "Z(IA,jb)", -1.0);
  dpd_->buf4_close(&Z2);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 10, 0, 10, 0, 0, "Z(Ib,Nj)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, L_irr, 10, 10, 10, 10, 0, "VIaJb");
  dpd_->file2_init(&R1, PSIF_CC_GR, R_irr, 0, 1, "Ria");
  dpd_->contract424(&I, &R1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&R1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 10, 11, 10, 11, 0, "Z(Ib,Aj)");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract244(&T1, &Z, &Z2, 0, 2, 1, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_sort_axpy(&Z2, PSIF_EOM_TMP0, prsq, 10, 10, "Z(IA,jb)", +1.0);
  dpd_->buf4_close(&Z2);

  psio_close(PSIF_EOM_TMP1,0);
  psio_open(PSIF_EOM_TMP1,PSIO_OPEN_NEW);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 10, 0, 10, 0, 0, "Z(jA,nI)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, L_irr, 10, 10, 10, 10, 0, "ViAjB");
  dpd_->file2_init(&R1, PSIF_CC_GR, R_irr, 0, 1, "RIA");
  dpd_->contract424(&I, &R1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&R1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 11, 10, 11, 10, 0, "Z(bI,jA)");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract244(&T1, &Z, &Z2, 0, 2, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_sort(&Z2, PSIF_EOM_TMP1, sqrp, 11, 10, "Z(AI,jb)");
  dpd_->buf4_close(&Z2);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 11, 10, 11, 10, 0, "Z(AI,jb)");
  dpd_->buf4_sort_axpy(&Z2, PSIF_EOM_TMP0, qprs, 10, 10, "Z(IA,jb)", +1.0);
  dpd_->buf4_close(&Z2);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 10, 0, 10, 0, 0, "Z(IA,nj)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, L_irr, 10, 10, 10, 10, 0, "VIAjb");
  dpd_->file2_init(&R1, PSIF_CC_GR, R_irr, 0, 1, "Ria");
  dpd_->contract424(&I, &R1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&R1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 10, 11, 10, 11, 0, "Z(IA,bj)");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract244(&T1, &Z, &Z2, 0, 2, 1, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_sort_axpy(&Z2, PSIF_EOM_TMP0, pqsr, 10, 10, "Z(IA,jb)", -1.0);
  dpd_->buf4_close(&Z2);

  psio_close(PSIF_EOM_TMP1,0);
  psio_open(PSIF_EOM_TMP1,PSIO_OPEN_NEW);

  /* - P(ij) P(ab) (Rjmbe Lnmfe) Tif Tna, term 21 */
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 10, 0, 10, 0, 0, "Z(JB,NI)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 10, 10, 10, 10, 0, "R2L2_OVOV");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract424(&I, &T1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 11, 10, 11, 10, 0, "Z(AI,JB)");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract244(&T1, &Z, &Z2, 0, 2, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_sort_axpy(&Z2, PSIF_EOM_TMP0, qprs, 10, 10, "Z(IA,JB)", -1.0);
  dpd_->buf4_close(&Z2);
  
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 10, 0, 10, 0, 0, "Z(jb,ni)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 10, 10, 10, 10, 0, "R2L2_ovov");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract424(&I, &T1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 11, 10, 11, 10, 0, "Z(ai,jb)");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract244(&T1, &Z, &Z2, 0, 2, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_sort_axpy(&Z2, PSIF_EOM_TMP0, qprs, 10, 10, "Z(ia,jb)", -1.0);
  dpd_->buf4_close(&Z2);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 10, 0, 10, 0, 0, "Z(jb,NI)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 10, 10, 10, 10, 0, "R2L2_ovOV");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract424(&I, &T1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 11, 10, 11, 10, 0, "Z(AI,jb)");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract244(&T1, &Z, &Z2, 0, 2, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_sort_axpy(&Z2, PSIF_EOM_TMP0, qprs, 10, 10, "Z(IA,jb)", -1.0);
  dpd_->buf4_close(&Z2);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 10, 0, 10, 0, 0, "Z(Ib,Nj)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 10, 10, 10, 10, 0, "R2L2_OvOv");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract424(&I, &T1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 10, 11, 10, 11, 0, "Z(Ib,Aj)");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract244(&T1, &Z, &Z2, 0, 2, 1, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_sort_axpy(&Z2, PSIF_EOM_TMP0, prsq, 10, 10, "Z(IA,jb)", +1.0);
  dpd_->buf4_close(&Z2);

  psio_close(PSIF_EOM_TMP1,0);
  psio_open(PSIF_EOM_TMP1,PSIO_OPEN_NEW);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 10, 0, 10, 0, 0, "Z(jA,nI)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 10, 10, 10, 10, 0, "R2L2_oVoV");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract424(&I, &T1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 11, 10, 11, 10, 0, "Z(bI,jA)");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract244(&T1, &Z, &Z2, 0, 2, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_sort(&Z2, PSIF_EOM_TMP1, sqrp, 11, 10, "Z(AI,jb)");
  dpd_->buf4_close(&Z2);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 11, 10, 11, 10, 0, "Z(AI,jb)");
  dpd_->buf4_sort_axpy(&Z2, PSIF_EOM_TMP0, qprs, 10, 10, "Z(IA,jb)", +1.0);
  dpd_->buf4_close(&Z2);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 10, 0, 10, 0, 0, "Z(IA,nj)");
  dpd_->buf4_init(&I, PSIF_EOM_TMP, G_irr, 10, 10, 10, 10, 0, "R2L2_OVov");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract424(&I, &T1, &Z, 3, 1, 0, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&I);
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP1, G_irr, 10, 11, 10, 11, 0, "Z(IA,bj)");
  dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract244(&T1, &Z, &Z2, 0, 2, 1, 1.0, 0.0);
  dpd_->file2_close(&T1);
  dpd_->buf4_close(&Z);
  dpd_->buf4_sort_axpy(&Z2, PSIF_EOM_TMP0, pqsr, 10, 10, "Z(IA,jb)", -1.0);
  dpd_->buf4_close(&Z2);

  psio_close(PSIF_EOM_TMP1,0);
  psio_open(PSIF_EOM_TMP1,PSIO_OPEN_NEW);


  /* Now permute Z(IA,JB) and Z(ia,jb) and add them in along with Z(IA,jb) */
  dpd_->buf4_init(&Z2, PSIF_EOM_TMP0, G_irr, 10, 10, 10, 10, 0, "Z(IA,JB)");
  dpd_->buf4_sort(&Z2, PSIF_EOM_TMP1, prqs, 0, 5, "Z(IJ,AB)");
  dpd_->buf4_close(&Z2);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 5, 2, 7, 0, "GIJAB");
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 5, 0, 5, 0, "Z(IJ,AB)");
  dpd_->buf4_axpy(&Z, &G, 1.0);
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, qprs, 0, 5, "Z(JI,AB)");
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, pqsr, 0, 5, "Z(IJ,BA)");
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 5, 0, 5, 0, "Z(JI,AB)");
  dpd_->buf4_axpy(&Z, &G, -1.0);
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 5, 0, 5, 0, "Z(IJ,BA)");
  dpd_->buf4_axpy(&Z, &G, -1.0);
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, qprs, 0, 5, "Z(JI,BA)");
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 5, 0, 5, 0, "Z(JI,BA)");
  dpd_->buf4_axpy(&Z, &G, 1.0);
  dpd_->buf4_close(&Z);
  dpd_->buf4_close(&G);

  dpd_->buf4_init(&Z2, PSIF_EOM_TMP0, G_irr, 10, 10, 10, 10, 0, "Z(ia,jb)");
  dpd_->buf4_sort(&Z2, PSIF_EOM_TMP1, prqs, 0, 5, "Z(ij,ab)");
  dpd_->buf4_close(&Z2);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 5, 2, 7, 0, "Gijab");
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 5, 0, 5, 0, "Z(ij,ab)");
  dpd_->buf4_axpy(&Z, &G, 1.0);
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, qprs, 0, 5, "Z(ji,ab)");
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, pqsr, 0, 5, "Z(ij,ba)");
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 5, 0, 5, 0, "Z(ji,ab)");
  dpd_->buf4_axpy(&Z, &G, -1.0);
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 5, 0, 5, 0, "Z(ij,ba)");
  dpd_->buf4_axpy(&Z, &G, -1.0);
  dpd_->buf4_sort(&Z, PSIF_EOM_TMP1, qprs, 0, 5, "Z(ji,ba)");
  dpd_->buf4_close(&Z);
  dpd_->buf4_init(&Z, PSIF_EOM_TMP1, G_irr, 0, 5, 0, 5, 0, "Z(ji,ba)");
  dpd_->buf4_axpy(&Z, &G, 1.0);
  dpd_->buf4_close(&Z);
  dpd_->buf4_close(&G);

  dpd_->buf4_init(&Z, PSIF_EOM_TMP0, G_irr, 10, 10, 10, 10, 0, "Z(IA,jb)");
  dpd_->buf4_sort_axpy(&Z, PSIF_EOM_TMP0, prqs, 0, 5, "GIjAb", 1.0);
  dpd_->buf4_close(&Z);

  /* add to ground state parts */
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 2, 7, 2, 7, 0, "GIJAB");
  dpd_->buf4_init(&V, PSIF_CC_GAMMA, G_irr, 2, 7, 2, 7, 0, "GIJAB");
  dpd_->buf4_axpy(&G, &V, 0.5);
  dpd_->buf4_close(&V);
  dpd_->buf4_close(&G);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 2, 7, 2, 7, 0, "Gijab");
  dpd_->buf4_init(&V, PSIF_CC_GAMMA, G_irr, 2, 7, 2, 7, 0, "Gijab");
  dpd_->buf4_axpy(&G, &V, 0.5);
  dpd_->buf4_close(&V);
  dpd_->buf4_close(&G);
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 5, 0, 5, 0, "GIjAb");
  dpd_->buf4_init(&V, PSIF_CC_GAMMA, G_irr, 0, 5, 0, 5, 0, "GIjAb");
  dpd_->buf4_axpy(&G, &V, 0.5);
  dpd_->buf4_close(&V);
  dpd_->buf4_close(&G);

  /*
  dpd_buf4_init(&V, CC_GAMMA, G_irr, 2, 7, 2, 7, 0, "GIJAB");
  tval = dpd_buf4_dot_self(&V);
  dpd_buf4_close(&V);
  dpd_buf4_init(&V, CC_GAMMA, G_irr, 2, 7, 2, 7, 0, "Gijab");
  tval += dpd_buf4_dot_self(&V);
  dpd_buf4_close(&V);
  dpd_buf4_init(&V, CC_GAMMA, G_irr, 0, 5, 0, 5, 0, "GIjAb");
  tval += dpd_buf4_dot_self(&V);
  dpd_buf4_close(&V);
  fprintf(outfile,"<Gijab|Gijab> = %15.10lf\n", tval);
  */


  return;
}



/* This function computes the following EOM Gijab terms
   Z(i,a) += Rimae Lme ; term 3
   Z(i,a) -= 0.5 Lmnef Tmnea Rif ; term 22
   Z(i,a) -= 0.5 Lmnef Timef Rna; term 23
   Z(i,a) += lmnef rme tinaf; term 33 
   P(ij) P(ab) [ Z(i,a) * T(j,b) ]
 */

void x_Gijab_rohf_2(void) { 
  int h, nirreps, row, col;
  int i,j,a,b;
  int I1, I2, I3, I4, J1, J2, J3, J4, A1, A2, A3, A4, B1, B2, B3, B4;
  int I1sym, I2sym, I3sym, I4sym, J1sym, J2sym, J3sym, J4sym;
  int A1sym, A2sym, A3sym, A4sym, B1sym, B2sym, B3sym, B4sym;
  int L_irr, R_irr, G_irr;
  dpdfile2 L1R2A, L1R2B, T1A, T1B, Z1A, Z1B, I1A, I1B, R1A, R1B;
  dpdbuf4 G, I, Z, Z2, T, L;
 
  L_irr = params.L_irr; R_irr = params.R_irr; G_irr = params.G_irr;
  nirreps = moinfo.nirreps;

  /* Z(I,A) += L1R2_OV, term 3 */
  dpd_->file2_init(&L1R2A, PSIF_EOM_TMP, G_irr, 0, 1, "L1R2_OV");
  dpd_->file2_init(&L1R2B, PSIF_EOM_TMP, G_irr, 0, 1, "L1R2_ov");
  dpd_->file2_copy(&L1R2A, PSIF_EOM_TMP1, "ZIA");
  dpd_->file2_copy(&L1R2B, PSIF_EOM_TMP1, "Zia");
  dpd_->file2_close(&L1R2A);
  dpd_->file2_close(&L1R2B);

  dpd_->file2_init(&Z1A, PSIF_EOM_TMP1, G_irr, 0, 1, "ZIA");
  dpd_->file2_init(&Z1B, PSIF_EOM_TMP1, G_irr, 0, 1, "Zia");

  /* Z(I,A) += 0.5 (lmnef tmnea) rif, term 22 */
  dpd_->file2_init(&I1A, PSIF_EOM_TMP, L_irr, 1, 1, "LT2_VV");
  dpd_->file2_init(&R1A, PSIF_CC_GR, R_irr, 0, 1, "RIA");
  dpd_->contract222(&R1A, &I1A, &Z1A, 0, 1, -1.0, 1.0);
  dpd_->file2_close(&R1A);
  dpd_->file2_close(&I1A);
  dpd_->file2_init(&I1B, PSIF_EOM_TMP, L_irr, 1, 1, "LT2_vv");
  dpd_->file2_init(&R1B, PSIF_CC_GR, R_irr, 0, 1, "Ria");
  dpd_->contract222(&R1B, &I1B, &Z1B, 0, 1, -1.0, 1.0);
  dpd_->file2_close(&R1B);
  dpd_->file2_close(&I1B);

  /* Z(i,a) -= 0.5 (timef lnmef) rna; term 23 */
  dpd_->file2_init(&I1A, PSIF_EOM_TMP, L_irr, 0, 0, "LT2_OO");
  dpd_->file2_init(&R1A, PSIF_CC_GR, R_irr, 0, 1, "RIA");
  dpd_->contract222(&I1A, &R1A, &Z1A, 1, 1, -1.0, 1.0);
  dpd_->file2_close(&R1A);
  dpd_->file2_close(&I1A);
  dpd_->file2_init(&I1B, PSIF_EOM_TMP, L_irr, 0, 0, "LT2_oo");
  dpd_->file2_init(&R1B, PSIF_CC_GR, R_irr, 0, 1, "Ria");
  dpd_->contract222(&I1B, &R1B, &Z1B, 1, 1, -1.0, 1.0);
  dpd_->file2_close(&R1B);
  dpd_->file2_close(&I1B);

  /* Z(i,a) += lmnef rme tinaf; term 33  */
  if (!params.connect_xi) {
  dpd_->file2_init(&I1A, PSIF_EOM_TMP, G_irr, 0, 1, "L2R1_OV");
  dpd_->file2_init(&I1B, PSIF_EOM_TMP, G_irr, 0, 1, "L2R1_ov");
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 0, 5, 2, 7, 0, "tIJAB");
  dpd_->dot24(&I1A, &T, &Z1A, 0, 0, 1.0, 1.0);
  dpd_->buf4_close(&T);
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 0, 5, 0, 5, 0, "tIjAb");
  dpd_->dot24(&I1B, &T, &Z1A, 0, 0, 1.0, 1.0);
  dpd_->buf4_close(&T);
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 0, 5, 2, 7, 0, "tijab");
  dpd_->dot24(&I1B, &T, &Z1B, 0, 0, 1.0, 1.0);
  dpd_->buf4_close(&T);
  dpd_->buf4_init(&T, PSIF_CC_TAMPS, 0, 0, 5, 0, 5, 0, "tiJaB");
  dpd_->dot24(&I1A, &T, &Z1B, 0, 0, 1.0, 1.0);
  dpd_->buf4_close(&T);
  dpd_->file2_close(&I1A);
  dpd_->file2_close(&I1B);
  }


  /* open one-electron files for the nasty permutations */
  dpd_->file2_mat_init(&Z1A);   dpd_->file2_mat_init(&Z1B);
  dpd_->file2_mat_rd(&Z1A);     dpd_->file2_mat_rd(&Z1B);

  dpd_->file2_init(&T1A, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->file2_init(&T1B, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->file2_mat_init(&T1A);   dpd_->file2_mat_init(&T1B);
  dpd_->file2_mat_rd(&T1A);     dpd_->file2_mat_rd(&T1B);

  /* + Z(I,A) T(J,B) */
  /* - Z(I,B) T(J,A) */
  /* + T(I,A) Z(J,B) */
  /* - T(I,B) Z(J,A) */
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 2, 7, 2, 7, 0, "GIJAB");
  for(h=0; h < nirreps; h++) {
    dpd_->buf4_mat_irrep_init(&G, h);
    dpd_->buf4_mat_irrep_rd(&G, h);
    for(row=0; row < G.params->rowtot[h]; row++) {
      i = G.params->roworb[h][row][0];
      j = G.params->roworb[h][row][1];
      I1 = Z1A.params->rowidx[i]; I1sym = Z1A.params->psym[i];
      I2 = I1; I2sym = I1sym;
      I3 = T1A.params->rowidx[i]; I3sym = T1A.params->psym[i];
      I4 = I3; I4sym = I3sym;
      J1 = T1A.params->rowidx[j]; J1sym = T1A.params->psym[j];
      J2 = J1; J2sym=J1sym;
      J3 = Z1A.params->rowidx[j]; J3sym = Z1A.params->psym[j];
      J4 = J3; J4sym=J3sym;
      for(col=0; col < G.params->coltot[h]; col++) {
        a = G.params->colorb[h][col][0];
        b = G.params->colorb[h][col][1];
        A1 = Z1A.params->colidx[a]; A1sym = Z1A.params->qsym[a];
        A4 = A1; A4sym = A1sym;
        A2 = T1A.params->colidx[a]; A2sym = T1A.params->qsym[a];
        A3 = A2; A3sym = A2sym;
        B1 = T1A.params->colidx[b]; B1sym = T1A.params->qsym[b];
        B4 = B1; B4sym = B1sym;
        B2 = Z1A.params->colidx[b]; B2sym = Z1A.params->qsym[b];
        B3 = B2; B3sym = B2sym;
        /* + Z(I,A) T(J,B) */
        if ( ((I1sym^A1sym)==G_irr) && (J1sym==B1sym) )
          G.matrix[h][row][col] +=
            Z1A.matrix[I1sym][I1][A1] * T1A.matrix[J1sym][J1][B1];
        /* - Z(I,B) T(J,A) */
        if ( ((I2sym^B2sym)==G_irr) && (J2sym==A2sym) )
          G.matrix[h][row][col] -=
            Z1A.matrix[I2sym][I2][B2] * T1A.matrix[J2sym][J2][A2];
        /* + T(I,A) Z(J,B) */
        if ( ((J3sym^B3sym)==G_irr) && (I3sym==A3sym) )
          G.matrix[h][row][col] +=
            Z1A.matrix[J3sym][J3][B3] * T1A.matrix[I3sym][I3][A3];
        /* - T(I,B) Z(J,A) */
        if ( ((J4sym^A4sym)==G_irr) && (I4sym==B4sym) )
          G.matrix[h][row][col] -=
            Z1A.matrix[J4sym][J4][A4] * T1A.matrix[I4sym][I4][B4];
      }
    }
    dpd_->buf4_mat_irrep_wrt(&G, h);
    dpd_->buf4_mat_irrep_close(&G, h);
  }
  dpd_->buf4_close(&G);

  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 2, 7, 2, 7, 0, "Gijab");
  for(h=0; h < nirreps; h++) {
    dpd_->buf4_mat_irrep_init(&G, h);
    dpd_->buf4_mat_irrep_rd(&G, h);
    for(row=0; row < G.params->rowtot[h]; row++) {
      i = G.params->roworb[h][row][0];
      j = G.params->roworb[h][row][1];
      I1 = Z1B.params->rowidx[i]; I1sym = Z1B.params->psym[i];
      I2 = I1; I2sym = I1sym;
      I3 = T1B.params->rowidx[i]; I3sym = T1B.params->psym[i];
      I4 = I3; I4sym = I3sym;
      J1 = T1B.params->rowidx[j]; J1sym = T1B.params->psym[j];
      J2 = J1; J2sym=J1sym;
      J3 = Z1B.params->rowidx[j]; J3sym = Z1B.params->psym[j];
      J4 = J3; J4sym=J3sym;
      for(col=0; col < G.params->coltot[h]; col++) {
        a = G.params->colorb[h][col][0];
        b = G.params->colorb[h][col][1];
        A1 = Z1B.params->colidx[a]; A1sym = Z1B.params->qsym[a];
        A4 = A1; A4sym = A1sym;
        A2 = T1B.params->colidx[a]; A2sym = T1B.params->qsym[a];
        A3 = A2; A3sym = A2sym;
        B1 = T1B.params->colidx[b]; B1sym = T1B.params->qsym[b];
        B4 = B1; B4sym = B1sym;
        B2 = Z1B.params->colidx[b]; B2sym = Z1B.params->qsym[b];
        B3 = B2; B3sym = B2sym;
        /* + Z(i,a) T(j,b) */
        if ( ((I1sym^A1sym)==G_irr) && (J1sym==B1sym) )
          G.matrix[h][row][col] +=
            Z1B.matrix[I1sym][I1][A1] * T1B.matrix[J1sym][J1][B1];
        /* - Z(i,b) T(j,a) */
        if ( ((I2sym^B2sym)==G_irr) && (J2sym==A2sym) )
          G.matrix[h][row][col] -=
            Z1B.matrix[I2sym][I2][B2] * T1B.matrix[J2sym][J2][A2];
        /* + T(i,a) Z(j,b) */
        if ( ((J3sym^B3sym)==G_irr) && (I3sym==A3sym) )
          G.matrix[h][row][col] +=
            Z1B.matrix[J3sym][J3][B3] * T1B.matrix[I3sym][I3][A3];
        /* - T(i,b) Z(j,a) */
        if ( ((J4sym^A4sym)==G_irr) && (I4sym==B4sym) )
          G.matrix[h][row][col] -=
            Z1B.matrix[J4sym][J4][A4] * T1B.matrix[I4sym][I4][B4];
      }
    }
    dpd_->buf4_mat_irrep_wrt(&G, h);
    dpd_->buf4_mat_irrep_close(&G, h);
  }
  dpd_->buf4_close(&G);

  /* + Z(I,A) T(j,b) */
  /* + T(I,A) Z(j,b) */
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 5, 0, 5, 0, "GIjAb");
  for(h=0; h < nirreps; h++) {
    dpd_->buf4_mat_irrep_init(&G, h);
    dpd_->buf4_mat_irrep_rd(&G, h);
    for(row=0; row < G.params->rowtot[h]; row++) {
      i = G.params->roworb[h][row][0];
      j = G.params->roworb[h][row][1];
      I1 = Z1A.params->rowidx[i]; I1sym = Z1A.params->psym[i];
      I2 = T1A.params->rowidx[i]; I2sym = T1A.params->psym[i];
      J1 = T1B.params->rowidx[j]; J1sym = T1B.params->psym[j];
      J2 = Z1B.params->rowidx[j]; J2sym = Z1B.params->psym[j];
      for(col=0; col < G.params->coltot[h]; col++) {
        a = G.params->colorb[h][col][0];
        b = G.params->colorb[h][col][1];
        A1 = Z1A.params->colidx[a]; A1sym = Z1A.params->qsym[a];
        A2 = T1A.params->colidx[a]; A2sym = T1A.params->qsym[a];
        B1 = T1B.params->colidx[b]; B1sym = T1B.params->qsym[b];
        B2 = Z1B.params->colidx[b]; B2sym = Z1B.params->qsym[b];
        /* + Z(I,A) T(j,b) */
        if ( ((I1sym^A1sym)==G_irr) && (J1sym==B1sym) )
          G.matrix[h][row][col] +=
            Z1A.matrix[I1sym][I1][A1] * T1B.matrix[J1sym][J1][B1];
        /* + T(I,A) Z(j,b) */
        if ( ((J2sym^B2sym)==G_irr) && (I2sym==A2sym) )
          G.matrix[h][row][col] +=
            T1A.matrix[I2sym][I2][A2] * Z1B.matrix[J2sym][J2][B2];
      }
    }
    dpd_->buf4_mat_irrep_wrt(&G, h);
    dpd_->buf4_mat_irrep_close(&G, h);
  }
  dpd_->buf4_close(&G);

  dpd_->file2_mat_close(&T1A); dpd_->file2_mat_close(&T1B);
  dpd_->file2_close(&T1A); dpd_->file2_close(&T1B);

  dpd_->file2_mat_close(&Z1A); dpd_->file2_mat_close(&Z1B);
  dpd_->file2_close(&Z1A); dpd_->file2_close(&Z1B);

  psio_close(PSIF_EOM_TMP1,0);
  psio_open(PSIF_EOM_TMP1,PSIO_OPEN_NEW);
  return;
}



/* This function computes the following EOM Gijab terms
   P(ij) P(ab) [ Z(i,a) * R(j,b) ]
   Z(i,a) += Timae Lme, term 18
   Z(i,a) -= 0.5 (Lmnef Tmnea) Tif, term 32
   Z(i,a) -= 0.5 (Lmnef Tmief) Tna; term 34
   Z(i,a) += Lme Tma Tie ; term 19
 */

void x_Gijab_rohf_3(void) { 
  int h, nirreps, row, col;
  int i,j,a,b;
  int I1, I2, I3, I4, J1, J2, J3, J4, A1, A2, A3, A4, B1, B2, B3, B4;
  int I1sym, I2sym, I3sym, I4sym, J1sym, J2sym, J3sym, J4sym;
  int A1sym, A2sym, A3sym, A4sym, B1sym, B2sym, B3sym, B4sym;
  int L_irr, R_irr, G_irr;
  dpdfile2 L1T2A, L1T2B, T1A, T1B, Z1A, Z1B, I1A, I1B, R1A, R1B;
  dpdbuf4 G, I, Z, Z2, T, L;
 
  L_irr = params.L_irr; R_irr = params.R_irr; G_irr = params.G_irr;
  nirreps = moinfo.nirreps;

  /* Z(I,A) += L1T2_OV, term 18 */
  dpd_->file2_init(&L1T2A, PSIF_EOM_TMP, L_irr, 0, 1, "L1T2_OV");
  dpd_->file2_init(&L1T2B, PSIF_EOM_TMP, L_irr, 0, 1, "L1T2_ov");
  dpd_->file2_copy(&L1T2A, PSIF_EOM_TMP1, "ZIA");
  dpd_->file2_copy(&L1T2B, PSIF_EOM_TMP1, "Zia");
  dpd_->file2_close(&L1T2A);
  dpd_->file2_close(&L1T2B);

  dpd_->file2_init(&Z1A, PSIF_EOM_TMP1, L_irr, 0, 1, "ZIA");
  dpd_->file2_init(&Z1B, PSIF_EOM_TMP1, L_irr, 0, 1, "Zia");

  /* Z(I,A) -= 0.5 Lmnef Tmnea Tif, term 32 */
  dpd_->file2_init(&I1A, PSIF_EOM_TMP, L_irr, 1, 1, "LT2_VV");
  dpd_->file2_init(&T1A, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract222(&T1A, &I1A, &Z1A, 0, 1, -1.0, 1.0);
  dpd_->file2_close(&T1A);
  dpd_->file2_close(&I1A);
  dpd_->file2_init(&I1B, PSIF_EOM_TMP, L_irr, 1, 1, "LT2_vv");
  dpd_->file2_init(&T1B, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract222(&T1B, &I1B, &Z1B, 0, 1, -1.0, 1.0);
  dpd_->file2_close(&T1B);
  dpd_->file2_close(&I1B);

  /* Z(i,a) -= 0.5 (Lmnef Tmief ) Tna; term 34 */
  dpd_->file2_init(&I1A, PSIF_EOM_TMP, L_irr, 0, 0, "LT2_OO");
  dpd_->file2_init(&T1A, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract222(&I1A, &T1A, &Z1A, 1, 1, -1.0, 1.0);
  dpd_->file2_close(&T1A);
  dpd_->file2_close(&I1A);
  dpd_->file2_init(&I1B, PSIF_EOM_TMP, L_irr, 0, 0, "LT2_oo");
  dpd_->file2_init(&T1B, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract222(&I1B, &T1B, &Z1B, 1, 1, -1.0, 1.0);
  dpd_->file2_close(&T1B);
  dpd_->file2_close(&I1B);

  /* Z(i,a) += Lme Tma Tie ; term 19  */
  dpd_->file2_init(&I1A, PSIF_EOM_TMP, L_irr, 1, 1, "LT1_VV");
  dpd_->file2_init(&T1A, PSIF_CC_OEI, 0, 0, 1, "tIA");
  dpd_->contract222(&T1A, &I1A, &Z1A, 0, 1, -1.0, 1.0);
  dpd_->file2_close(&T1A);
  dpd_->file2_close(&I1A);
  dpd_->file2_init(&I1B, PSIF_EOM_TMP, L_irr, 1, 1, "LT1_vv");
  dpd_->file2_init(&T1B, PSIF_CC_OEI, 0, 0, 1, "tia");
  dpd_->contract222(&T1B, &I1B, &Z1B, 0, 1, -1.0, 1.0);
  dpd_->file2_close(&T1B);
  dpd_->file2_close(&I1B);

  /* open one-electron files for the nasty terms */
  dpd_->file2_mat_init(&Z1A);   dpd_->file2_mat_init(&Z1B);
  dpd_->file2_mat_rd(&Z1A);     dpd_->file2_mat_rd(&Z1B);

  dpd_->file2_init(&R1A, PSIF_CC_GR, R_irr, 0, 1, "RIA");
  dpd_->file2_init(&R1B, PSIF_CC_GR, R_irr, 0, 1, "Ria");
  dpd_->file2_mat_init(&R1A);   dpd_->file2_mat_init(&R1B);
  dpd_->file2_mat_rd(&R1A);     dpd_->file2_mat_rd(&R1B);

  /* + Z(I,A) R(J,B) */
  /* - Z(I,B) R(J,A) */
  /* + R(I,A) Z(J,B) */
  /* - R(I,B) Z(J,A) */
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 2, 7, 2, 7, 0, "GIJAB");
  for(h=0; h < nirreps; h++) {
    dpd_->buf4_mat_irrep_init(&G, h);
    dpd_->buf4_mat_irrep_rd(&G, h);
    for(row=0; row < G.params->rowtot[h]; row++) {
      i = G.params->roworb[h][row][0];
      j = G.params->roworb[h][row][1];
      I1 = Z1A.params->rowidx[i]; I1sym = Z1A.params->psym[i];
      I2 = I1; I2sym = I1sym;
      I3 = R1A.params->rowidx[i]; I3sym = R1A.params->psym[i];
      I4 = I3; I4sym = I3sym;
      J1 = R1A.params->rowidx[j]; J1sym = R1A.params->psym[j];
      J2 = J1; J2sym=J1sym;
      J3 = Z1A.params->rowidx[j]; J3sym = Z1A.params->psym[j];
      J4 = J3; J4sym=J3sym;
      for(col=0; col < G.params->coltot[h]; col++) {
        a = G.params->colorb[h][col][0];
        b = G.params->colorb[h][col][1];
        A1 = Z1A.params->colidx[a]; A1sym = Z1A.params->qsym[a];
        A4 = A1; A4sym = A1sym;
        A2 = R1A.params->colidx[a]; A2sym = R1A.params->qsym[a];
        A3 = A2; A3sym = A2sym;
        B1 = R1A.params->colidx[b]; B1sym = R1A.params->qsym[b];
        B4 = B1; B4sym = B1sym;
        B2 = Z1A.params->colidx[b]; B2sym = Z1A.params->qsym[b];
        B3 = B2; B3sym = B2sym;
        /* + Z(I,A) R(J,B) */
        if ( ((I1sym^A1sym)==L_irr) && ((J1sym^B1sym)==R_irr) )
          G.matrix[h][row][col] +=
            Z1A.matrix[I1sym][I1][A1] * R1A.matrix[J1sym][J1][B1];
        /* - Z(I,B) R(J,A) */
        if ( ((I2sym^B2sym)==L_irr) && ((J2sym^A2sym)==R_irr) )
          G.matrix[h][row][col] -=
            Z1A.matrix[I2sym][I2][B2] * R1A.matrix[J2sym][J2][A2];
        /* + R(I,A) Z(J,B) */
        if ( ((J3sym^B3sym)==L_irr) && ((I3sym^A3sym)==R_irr) )
          G.matrix[h][row][col] +=
            Z1A.matrix[J3sym][J3][B3] * R1A.matrix[I3sym][I3][A3];
        /* - R(I,B) Z(J,A) */
        if ( ((J4sym^A4sym)==L_irr) && ((I4sym^B4sym)==R_irr) )
          G.matrix[h][row][col] -=
            Z1A.matrix[J4sym][J4][A4] * R1A.matrix[I4sym][I4][B4];
      }
    }
    dpd_->buf4_mat_irrep_wrt(&G, h);
    dpd_->buf4_mat_irrep_close(&G, h);
  }
  dpd_->buf4_close(&G);

  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 2, 7, 2, 7, 0, "Gijab");
  for(h=0; h < nirreps; h++) {
    dpd_->buf4_mat_irrep_init(&G, h);
    dpd_->buf4_mat_irrep_rd(&G, h);
    for(row=0; row < G.params->rowtot[h]; row++) {
      i = G.params->roworb[h][row][0];
      j = G.params->roworb[h][row][1];
      I1 = Z1B.params->rowidx[i]; I1sym = Z1B.params->psym[i];
      I2 = I1; I2sym = I1sym;
      I3 = R1B.params->rowidx[i]; I3sym = R1B.params->psym[i];
      I4 = I3; I4sym = I3sym;
      J1 = R1B.params->rowidx[j]; J1sym = R1B.params->psym[j];
      J2 = J1; J2sym=J1sym;
      J3 = Z1B.params->rowidx[j]; J3sym = Z1B.params->psym[j];
      J4 = J3; J4sym=J3sym;
      for(col=0; col < G.params->coltot[h]; col++) {
        a = G.params->colorb[h][col][0];
        b = G.params->colorb[h][col][1];
        A1 = Z1B.params->colidx[a]; A1sym = Z1B.params->qsym[a];
        A4 = A1; A4sym = A1sym;
        A2 = R1B.params->colidx[a]; A2sym = R1B.params->qsym[a];
        A3 = A2; A3sym = A2sym;
        B1 = R1B.params->colidx[b]; B1sym = R1B.params->qsym[b];
        B4 = B1; B4sym = B1sym;
        B2 = Z1B.params->colidx[b]; B2sym = Z1B.params->qsym[b];
        B3 = B2; B3sym = B2sym;
        /* + Z(i,a) R(j,b) */
        if ( ((I1sym^A1sym)==L_irr) && ((J1sym^B1sym)==R_irr ) )
          G.matrix[h][row][col] +=
            Z1B.matrix[I1sym][I1][A1] * R1B.matrix[J1sym][J1][B1];
        /* - Z(i,b) R(j,a) */
        if ( ((I2sym^B2sym)==L_irr) && ((J2sym^A2sym)==R_irr ) )
          G.matrix[h][row][col] -=
            Z1B.matrix[I2sym][I2][B2] * R1B.matrix[J2sym][J2][A2];
        /* + R(i,a) Z(j,b) */
        if ( ((J3sym^B3sym)==L_irr) && ((I3sym^A3sym)==R_irr ) )
          G.matrix[h][row][col] +=
            Z1B.matrix[J3sym][J3][B3] * R1B.matrix[I3sym][I3][A3];
        /* - R(i,b) Z(j,a) */
        if ( ((J4sym^A4sym)==L_irr) && ((I4sym^B4sym)==R_irr ) )
          G.matrix[h][row][col] -=
            Z1B.matrix[J4sym][J4][A4] * R1B.matrix[I4sym][I4][B4];
      }
    }
    dpd_->buf4_mat_irrep_wrt(&G, h);
    dpd_->buf4_mat_irrep_close(&G, h);
  }
  dpd_->buf4_close(&G);

  /* + Z(I,A) R(j,b) */
  /* + R(I,A) Z(j,b) */
  dpd_->buf4_init(&G, PSIF_EOM_TMP0, G_irr, 0, 5, 0, 5, 0, "GIjAb");
  for(h=0; h < nirreps; h++) {
    dpd_->buf4_mat_irrep_init(&G, h);
    dpd_->buf4_mat_irrep_rd(&G, h);
    for(row=0; row < G.params->rowtot[h]; row++) {
      i = G.params->roworb[h][row][0];
      j = G.params->roworb[h][row][1];
      I1 = Z1A.params->rowidx[i]; I1sym = Z1A.params->psym[i];
      I2 = R1A.params->rowidx[i]; I2sym = R1A.params->psym[i];
      J1 = R1B.params->rowidx[j]; J1sym = R1B.params->psym[j];
      J2 = Z1B.params->rowidx[j]; J2sym = Z1B.params->psym[j];
      for(col=0; col < G.params->coltot[h]; col++) {
        a = G.params->colorb[h][col][0];
        b = G.params->colorb[h][col][1];
        A1 = Z1A.params->colidx[a]; A1sym = Z1A.params->qsym[a];
        A2 = R1A.params->colidx[a]; A2sym = R1A.params->qsym[a];
        B1 = R1B.params->colidx[b]; B1sym = R1B.params->qsym[b];
        B2 = Z1B.params->colidx[b]; B2sym = Z1B.params->qsym[b];
        /* + Z(I,A) R(j,b) */
        if ( ((I1sym^A1sym)==L_irr) && ((J1sym^B1sym)==R_irr ) )
          G.matrix[h][row][col] +=
            Z1A.matrix[I1sym][I1][A1] * R1B.matrix[J1sym][J1][B1];
        /* + R(I,A) Z(j,b) */
        if ( ((J2sym^B2sym)==L_irr) && ((I2sym^A2sym)==R_irr ) )
          G.matrix[h][row][col] +=
            R1A.matrix[I2sym][I2][A2] * Z1B.matrix[J2sym][J2][B2];
      }
    }
    dpd_->buf4_mat_irrep_wrt(&G, h);
    dpd_->buf4_mat_irrep_close(&G, h);
  }
  dpd_->buf4_close(&G);

  dpd_->file2_mat_close(&R1A); dpd_->file2_mat_close(&R1B);
  dpd_->file2_close(&R1A); dpd_->file2_close(&R1B);

  dpd_->file2_mat_close(&Z1A); dpd_->file2_mat_close(&Z1B);
  dpd_->file2_close(&Z1A); dpd_->file2_close(&Z1B);

  psio_close(PSIF_EOM_TMP1,0);
  psio_open(PSIF_EOM_TMP1,PSIO_OPEN_NEW);
  return;
}

}} // namespace psi::ccdensity
