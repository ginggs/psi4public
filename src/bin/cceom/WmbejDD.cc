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
    \ingroup CCEOM
    \brief Enter brief description of file here 
*/
#include <cstdio>
#include <cmath>
#include "MOInfo.h"
#include "Params.h"
#include "Local.h"
#define EXTERN
#include "globals.h"

namespace psi { namespace cceom {

/* This function computes the H-bar doubles-doubles block contribution
   from P(ij)P(ab) Wmbej*Cimae to a Sigma vector stored at Sigma plus 'i' */

void WmbejDD(int i, int C_irr) {
  dpdbuf4 S2, S2temp, D;
  dpdbuf4 C2, C2temp;
  dpdbuf4 CMNEF, Cmnef, CMnEf, CmNeF, TIJAB, Tijab, TIjAb;
  dpdbuf4 SIJAB, Sijab, SIjAb, W, W1, W2, Z;
  char CMNEF_lbl[32], Cmnef_lbl[32], CMnEf_lbl[32];
  char SIJAB_lbl[32], Sijab_lbl[32], SIjAb_lbl[32];

  if(params.eom_ref == 0) { /** RHF **/
    /* RHF code adapted from WmbejT2.c 6-2002 */
    sprintf(SIjAb_lbl, "%s %d", "SIjAb", i);

    /* C2(Ib,mE) * W(mE,jA) --> Z(Ib,jA) */
    dpd_->buf4_init(&SIjAb, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "Z (Ib,jA)");
    dpd_->buf4_init(&C2, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "CMfnE");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 10, 10, 10, 10, 0, "WMbeJ");
    dpd_->contract444(&C2, &W, &SIjAb, 0, 1, 1, 0);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&C2);
    /* C2(Ib,jA) --> C2(IA,jb) (part III) */
    dpd_->buf4_sort(&SIjAb, PSIF_EOM_TMP, psrq, 10, 10, "C2 (IA,jb) 3");
    dpd_->buf4_close(&SIjAb);

    /* 1/2 [ (2 C2(IA,me) - C2(IE,ma)) * (2 W(ME,jb) + W(Me,Jb)] --> C2(IA,jb) */
    dpd_->buf4_init(&SIjAb, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "C2 (IA,jb) 1");
    dpd_->buf4_init(&C2, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "2CMEnf-CMfnE");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 10, 10, 10, 10, 0, "2 W(ME,jb) + W(Me,Jb)");
    dpd_->contract444(&C2, &W, &SIjAb, 0, 1, 0.5, 0);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&C2);

    /* 1/2 Z(Ib,jA) + C2(IA,jb) --> C2(IA,jb) (Part I) */
    dpd_->buf4_init(&Z, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "Z (Ib,jA)");
    dpd_->buf4_axpy(&Z, &SIjAb, 0.5);
    dpd_->buf4_close(&Z);
    dpd_->buf4_close(&SIjAb);

    /* C2(IA,jb) (I) + C2(IA,jb) (III) --> C2(IA,jb) */
    dpd_->buf4_init(&SIjAb, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "C2 (IA,jb) 1");
    dpd_->buf4_init(&C2, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "C2 (IA,jb) 3");
    dpd_->buf4_axpy(&C2, &SIjAb, 1);
    dpd_->buf4_close(&C2);
    dpd_->buf4_sort(&SIjAb, PSIF_EOM_TMP, prqs, 0, 5, "C2 (Ij,Ab) (1+3)");
    dpd_->buf4_close(&SIjAb);
    dpd_->buf4_init(&SIjAb, PSIF_EOM_TMP, C_irr, 0, 5, 0, 5, 0, "C2 (Ij,Ab) (1+3)");
    dpd_->buf4_sort(&SIjAb, PSIF_EOM_TMP, qpsr, 0, 5, "C2 (Ij,Ab) (2+4)");
    dpd_->buf4_close(&SIjAb);

    /* C2(Ij,Ab) <--- I + II + III + IV */
    dpd_->buf4_init(&SIjAb, PSIF_EOM_SIjAb, C_irr, 0, 5, 0, 5, 0, SIjAb_lbl);
    dpd_->buf4_init(&C2, PSIF_EOM_TMP, C_irr, 0, 5, 0, 5, 0, "C2 (Ij,Ab) (1+3)");
    dpd_->buf4_axpy(&C2, &SIjAb, 1);
    dpd_->buf4_close(&C2);
    dpd_->buf4_init(&C2, PSIF_EOM_TMP, C_irr, 0, 5, 0, 5, 0, "C2 (Ij,Ab) (2+4)");
    dpd_->buf4_axpy(&C2, &SIjAb, 1);
    dpd_->buf4_close(&C2);
    dpd_->buf4_close(&SIjAb);
  }

  else if (params.eom_ref == 1) { /* ROHF */
    sprintf(CMNEF_lbl, "%s %d", "CMNEF", i);
    sprintf(Cmnef_lbl, "%s %d", "Cmnef", i);
    sprintf(CMnEf_lbl, "%s %d", "CMnEf", i);
    sprintf(SIJAB_lbl, "%s %d", "SIJAB", i);
    sprintf(Sijab_lbl, "%s %d", "Sijab", i);
    sprintf(SIjAb_lbl, "%s %d", "SIjAb", i);

    /* C2(IA,ME) * W(ME,JB) --> S2(IA,JB) */
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "SIAJB");
    dpd_->buf4_init(&CMNEF, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "CMENF");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 10, 10, 10, 10, 0, "WMBEJ");
    dpd_->contract444(&CMNEF, &W, &S2, 0, 1, 1, 0);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&CMNEF);
    /* C2(IA,me) * W(me,JB) --> S2(IA,JB) */
    dpd_->buf4_init(&CMnEf, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "CMEnf");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 10, 10, 10, 10, 0, "WmBeJ");
    dpd_->contract444(&CMnEf, &W, &S2, 0, 1, 1, 1);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&CMnEf);
    /* P(IJ) S2(IA,JB) */
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, rqps, 10, 10, "SJAIB");
    /* P(AB) S2(IA,JB) */
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, psrq, 10, 10, "SIBJA");
    dpd_->buf4_close(&S2);
    /* P(IJ) P(AB) S2(IA,JB) */
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "SJAIB");
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, psrq, 10, 10, "SJBIA");
    dpd_->buf4_close(&S2);
    /* S2(IA,JB) - S2(JA,IB) - S2(IB,JA) + S2(JB,IA) --> S2(IA,JB) */
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "SIAJB");
    dpd_->buf4_init(&S2temp, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "SJAIB");
    dpd_->buf4_axpy(&S2temp, &S2, -1);
    dpd_->buf4_close(&S2temp);
    dpd_->buf4_init(&S2temp, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "SIBJA");
    dpd_->buf4_axpy(&S2temp, &S2, -1);
    dpd_->buf4_close(&S2temp);
    dpd_->buf4_init(&S2temp, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "SJBIA");
    dpd_->buf4_axpy(&S2temp, &S2, 1);
    dpd_->buf4_close(&S2temp);
    /* S2(IA,JB) --> S2(IJ,AB) */
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, prqs, 0, 5, "SIJAB");
    dpd_->buf4_close(&S2);
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 0, 5, 0, 5, 0, "SIJAB");
    dpd_->buf4_init(&SIJAB, PSIF_EOM_SIJAB, C_irr, 0, 5, 2, 7, 0, SIJAB_lbl);
    dpd_->buf4_axpy(&S2, &SIJAB, 1.0);
    dpd_->buf4_close(&SIJAB);
    dpd_->buf4_close(&S2);


    /* C2(ia,me) * W(me,jb) --> S2(ia,jb) */
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "Siajb");
    dpd_->buf4_init(&Cmnef, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "Cmenf");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 10, 10, 10, 10, 0, "Wmbej");
    dpd_->contract444(&Cmnef, &W, &S2, 0, 1, 1, 0);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&Cmnef);
    /* C2(ia,ME) * W(ME,jb) --> S2(ia,jb) */
    dpd_->buf4_init(&CMnEf, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "CmeNF");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 10, 10, 10, 10, 0, "WMbEj");
    dpd_->contract444(&CMnEf, &W, &S2, 0, 1, 1, 1);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&CMnEf);
    /* P(ij) S2(ia,jb) */
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, rqps, 10, 10, "Sjaib");
    /* P(ab) S2(ia,jb) */
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, psrq, 10, 10, "Sibja");
    dpd_->buf4_close(&S2);
    /* P(ij) P(ab) S2(ia,jb) */
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "Sjaib");
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, psrq, 10, 10, "Sjbia");
    dpd_->buf4_close(&S2);
    /* S2(ia,jb) - S2(ja,ib) - S2(ib,ja) + S2(jb,ia) --> S2(ia,jb) */
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "Siajb");
    dpd_->buf4_init(&S2temp, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "Sjaib");
    dpd_->buf4_axpy(&S2temp, &S2, -1);
    dpd_->buf4_close(&S2temp);
    dpd_->buf4_init(&S2temp, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "Sibja");
    dpd_->buf4_axpy(&S2temp, &S2, -1);
    dpd_->buf4_close(&S2temp);
    dpd_->buf4_init(&S2temp, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "Sjbia");
    dpd_->buf4_axpy(&S2temp, &S2, 1);
    dpd_->buf4_close(&S2temp);
    /* S2(ia,jb) --> S2(ij,ab) */
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, prqs, 0, 5, "Sijab");
    dpd_->buf4_close(&S2);
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 0, 5, 0, 5, 0, "Sijab");
    dpd_->buf4_init(&Sijab, PSIF_EOM_Sijab, C_irr, 0, 5, 2, 7, 0, Sijab_lbl);
    dpd_->buf4_axpy(&S2, &Sijab, 1.0);
    dpd_->buf4_close(&Sijab);
    dpd_->buf4_close(&S2);

    /* C2(IA,ME) * W(ME,jb) --> S2(IA,jb) */
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "SIAjb");
    dpd_->buf4_init(&CMnEf, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "CMENF");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 10, 10, 10, 10, 0, "WMbEj");
    dpd_->contract444(&CMnEf, &W, &S2, 0, 1, 1, 0);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&CMnEf);
    /* C2(IA,me) * W(me,jb) --> S2(IA,jb) */
    dpd_->buf4_init(&CMnEf, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "CMEnf");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 10, 10, 10, 10, 0, "Wmbej");
    dpd_->contract444(&CMnEf, &W, &S2, 0, 1, 1, 1);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&CMnEf);
    /* W(ME,IA) * C2(jb,ME) --> S2(IA,jb) */
    dpd_->buf4_init(&CMnEf, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "CmeNF");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 10, 10, 10, 10, 0, "WMBEJ");
    dpd_->contract444(&W, &CMnEf, &S2, 1, 0, 1, 1);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&CMnEf);
    /* W(me,IA) * C2(jb,me) --> S2(IA,jb) */
    dpd_->buf4_init(&Cmnef, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "Cmenf");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 10, 10, 10, 10, 0, "WmBeJ");
    dpd_->contract444(&W, &Cmnef, &S2, 1, 0, 1, 1);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&Cmnef);
    /* C2(IA,jb) --> S2(Ij,Ab) */
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, prqs, 0, 5, "SIjAb");
    dpd_->buf4_close(&S2);
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 0, 5, 0, 5, 0, "SIjAb");
    dpd_->buf4_init(&SIjAb, PSIF_EOM_SIjAb, C_irr, 0, 5, 0, 5, 0, SIjAb_lbl);
    dpd_->buf4_axpy(&S2, &SIjAb, 1);
    dpd_->buf4_close(&SIjAb);
    dpd_->buf4_close(&S2);
    /* C2(Ib,mE) * W(mE,jA) --> S2(Ib,jA) */
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "SIbjA");
    /* dpd_buf4_init(&CMnEf, EOM_TMP, C_irr, 10, 10, 10, 10, 0, "CMenF"); */
    dpd_->buf4_init(&CMnEf, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "CMfnE");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 10, 10, 10, 10, 0, "WmBEj");
    dpd_->contract444(&CMnEf, &W, &S2, 0, 1, 1, 0);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&CMnEf);
    /* W(Me,Ib) * C2(jA,Me) --> S2(Ib,jA) */
    dpd_->buf4_init(&CMnEf, PSIF_EOM_TMP, C_irr, 10, 10, 10, 10, 0, "CnEMf");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 10, 10, 10, 10, 0, "WMbeJ");
    dpd_->contract444(&W, &CMnEf, &S2, 1, 0, 1, 1);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&CMnEf);
    /* C2(Ib,jA) --> S2(Ij,bA) --> S2(Ij,Ab) */
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, prqs, 0, 5, "SIjbA");
    dpd_->buf4_close(&S2);
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 0, 5, 0, 5, 0, "SIjbA");
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, pqsr, 0, 5, "SIjAb");
    dpd_->buf4_close(&S2);
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 0, 5, 0, 5, 0, "SIjAb");
    dpd_->buf4_init(&SIjAb, PSIF_EOM_SIjAb, C_irr, 0, 5, 0, 5, 0, SIjAb_lbl);
    dpd_->buf4_axpy(&S2, &SIjAb, 1);
    dpd_->buf4_close(&SIjAb);
    dpd_->buf4_close(&S2);

  }

  else if (params.eom_ref == 2) { /* UHF */
    sprintf(CMNEF_lbl, "%s %d", "CMNEF", i);
    sprintf(Cmnef_lbl, "%s %d", "Cmnef", i);
    sprintf(CMnEf_lbl, "%s %d", "CMnEf", i);
    sprintf(SIJAB_lbl, "%s %d", "SIJAB", i);
    sprintf(Sijab_lbl, "%s %d", "Sijab", i);
    sprintf(SIjAb_lbl, "%s %d", "SIjAb", i);

    /* C2(IA,ME) * W(ME,JB) --> S2(IA,JB) */
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 20, 20, 20, 20, 0, "SIAJB");
    dpd_->buf4_init(&CMNEF, PSIF_EOM_TMP, C_irr, 20, 20, 20, 20, 0, "CMENF");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 20, 20, 20, 20, 0, "WMBEJ");
    dpd_->contract444(&CMNEF, &W, &S2, 0, 1, 1, 0);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&CMNEF);
    /* C2(IA,me) * W(me,JB) --> S2(IA,JB) */
    dpd_->buf4_init(&CMnEf, PSIF_EOM_TMP, C_irr, 20, 30, 20, 30, 0, "CMEnf");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 30, 20, 30, 20, 0, "WmBeJ");
    dpd_->contract444(&CMnEf, &W, &S2, 0, 1, 1, 1);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&CMnEf);
    /* P(IJ) S2(IA,JB) */
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, rqps, 20, 20, "SJAIB");
    /* P(AB) S2(IA,JB) */
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, psrq, 20, 20, "SIBJA");
    dpd_->buf4_close(&S2);
    /* P(IJ) P(AB) S2(IA,JB) */
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 20, 20, 20, 20, 0, "SJAIB");
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, psrq, 20, 20, "SJBIA");
    dpd_->buf4_close(&S2);
    /* S2(IA,JB) - S2(JA,IB) - S2(IB,JA) + S2(JB,IA) --> S2(IA,JB) */
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 20, 20, 20, 20, 0, "SIAJB");
    dpd_->buf4_init(&S2temp, PSIF_EOM_TMP, C_irr, 20, 20, 20, 20, 0, "SJAIB");
    dpd_->buf4_axpy(&S2temp, &S2, -1);
    dpd_->buf4_close(&S2temp);
    dpd_->buf4_init(&S2temp, PSIF_EOM_TMP, C_irr, 20, 20, 20, 20, 0, "SIBJA");
    dpd_->buf4_axpy(&S2temp, &S2, -1);
    dpd_->buf4_close(&S2temp);
    dpd_->buf4_init(&S2temp, PSIF_EOM_TMP, C_irr, 20, 20, 20, 20, 0, "SJBIA");
    dpd_->buf4_axpy(&S2temp, &S2, 1);
    dpd_->buf4_close(&S2temp);
    /* S2(IA,JB) --> S2(IJ,AB) */
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, prqs, 0, 5, "SIJAB");
    dpd_->buf4_close(&S2);
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 0, 5, 0, 5, 0, "SIJAB");
    dpd_->buf4_init(&SIJAB, PSIF_EOM_SIJAB, C_irr, 0, 5, 2, 7, 0, SIJAB_lbl);
    dpd_->buf4_axpy(&S2, &SIJAB, 1.0);
    dpd_->buf4_close(&SIJAB);
    dpd_->buf4_close(&S2);


    /* C2(ia,me) * W(me,jb) --> S2(ia,jb) */
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 30, 30, 30, 30, 0, "Siajb");
    dpd_->buf4_init(&Cmnef, PSIF_EOM_TMP, C_irr, 30, 30, 30, 30, 0, "Cmenf");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 30, 30, 30, 30, 0, "Wmbej");
    dpd_->contract444(&Cmnef, &W, &S2, 0, 1, 1, 0);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&Cmnef);
    /* C2(ia,ME) * W(ME,jb) --> S2(ia,jb) */
    dpd_->buf4_init(&CMnEf, PSIF_EOM_TMP, C_irr, 30, 20, 30, 20, 0, "CmeNF");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 20, 30, 20, 30, 0, "WMbEj");
    dpd_->contract444(&CMnEf, &W, &S2, 0, 1, 1, 1);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&CMnEf);
    /* P(ij) S2(ia,jb) */
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, rqps, 30, 30, "Sjaib");
    /* P(ab) S2(ia,jb) */
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, psrq, 30, 30, "Sibja");
    dpd_->buf4_close(&S2);
    /* P(ij) P(ab) S2(ia,jb) */
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 30, 30, 30, 30, 0, "Sjaib");
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, psrq, 30, 30, "Sjbia");
    dpd_->buf4_close(&S2);
    /* S2(ia,jb) - S2(ja,ib) - S2(ib,ja) + S2(jb,ia) --> S2(ia,jb) */
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 30, 30, 30, 30, 0, "Siajb");
    dpd_->buf4_init(&S2temp, PSIF_EOM_TMP, C_irr, 30, 30, 30, 30, 0, "Sjaib");
    dpd_->buf4_axpy(&S2temp, &S2, -1);
    dpd_->buf4_close(&S2temp);
    dpd_->buf4_init(&S2temp, PSIF_EOM_TMP, C_irr, 30, 30, 30, 30, 0, "Sibja");
    dpd_->buf4_axpy(&S2temp, &S2, -1);
    dpd_->buf4_close(&S2temp);
    dpd_->buf4_init(&S2temp, PSIF_EOM_TMP, C_irr, 30, 30, 30, 30, 0, "Sjbia");
    dpd_->buf4_axpy(&S2temp, &S2, 1);
    dpd_->buf4_close(&S2temp);
    /* S2(ia,jb) --> S2(ij,ab) */
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, prqs, 10, 15, "Sijab");
    dpd_->buf4_close(&S2);
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 10, 15, 10, 15, 0, "Sijab");
    dpd_->buf4_init(&Sijab, PSIF_EOM_Sijab, C_irr, 10, 15, 12, 17, 0, Sijab_lbl);
    dpd_->buf4_axpy(&S2, &Sijab, 1.0);
    dpd_->buf4_close(&Sijab);
    dpd_->buf4_close(&S2);


    /* C2(IA,ME) * W(ME,jb) --> S2(IA,jb) */
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 20, 30, 20, 30, 0, "SIAjb");
    dpd_->buf4_init(&CMnEf, PSIF_EOM_TMP, C_irr, 20, 20, 20, 20, 0, "CMENF");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 20, 30, 20, 30, 0, "WMbEj");
    dpd_->contract444(&CMnEf, &W, &S2, 0, 1, 1, 0);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&CMnEf);
    /* C2(IA,me) * W(me,jb) --> S2(IA,jb) */
    dpd_->buf4_init(&CMnEf, PSIF_EOM_TMP, C_irr, 20, 30, 20, 30, 0, "CMEnf");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 30, 30, 30, 30, 0, "Wmbej");
    dpd_->contract444(&CMnEf, &W, &S2, 0, 1, 1, 1);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&CMnEf);
    /* W(ME,IA) * C2(jb,ME) --> S2(IA,jb) */
    dpd_->buf4_init(&CMnEf, PSIF_EOM_TMP, C_irr, 30, 20, 30, 20, 0, "CmeNF");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 20, 20, 20, 20, 0, "WMBEJ");
    dpd_->contract444(&W, &CMnEf, &S2, 1, 0, 1, 1);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&CMnEf);
    /* W(me,IA) * C2(jb,me) --> S2(IA,jb) */
    dpd_->buf4_init(&Cmnef, PSIF_EOM_TMP, C_irr, 30, 30, 30, 30, 0, "Cmenf");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 30, 20, 30, 20, 0, "WmBeJ");
    dpd_->contract444(&W, &Cmnef, &S2, 1, 0, 1, 1);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&Cmnef);
    /* C2(IA,jb) --> S2(Ij,Ab) */
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, prqs, 22, 28, "SIjAb");
    dpd_->buf4_close(&S2);
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 22, 28, 22, 28, 0, "SIjAb");
    dpd_->buf4_init(&SIjAb, PSIF_EOM_SIjAb, C_irr, 22, 28, 22, 28, 0, SIjAb_lbl);
    dpd_->buf4_axpy(&S2, &SIjAb, 1);
    dpd_->buf4_close(&SIjAb);
    dpd_->buf4_close(&S2);
    /* C2(Ib,mE) * W(mE,jA) --> S2(Ib,jA) */
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 24, 27, 24, 27, 0, "SIbjA");
    /* dpd_buf4_init(&CMnEf, EOM_TMP, C_irr, 10, 10, 10, 10, 0, "CMenF"); */
    dpd_->buf4_init(&CMnEf, PSIF_EOM_TMP, C_irr, 24, 27, 24, 27, 0, "CMfnE");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 27, 27, 27, 27, 0, "WmBEj");
    dpd_->contract444(&CMnEf, &W, &S2, 0, 1, 1, 0);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&CMnEf);
    /* W(Me,Ib) * C2(jA,Me) --> S2(Ib,jA) */
    dpd_->buf4_init(&CMnEf, PSIF_EOM_TMP, C_irr, 27, 24, 27, 24, 0, "CnEMf");
    dpd_->buf4_init(&W, PSIF_CC_HBAR, H_IRR, 24, 24, 24, 24, 0, "WMbeJ");
    dpd_->contract444(&W, &CMnEf, &S2, 1, 0, 1, 1);
    dpd_->buf4_close(&W);
    dpd_->buf4_close(&CMnEf);
    /* C2(Ib,jA) --> S2(Ij,bA) --> S2(Ij,Ab) */
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, prqs, 22, 29, "SIjbA");
    dpd_->buf4_close(&S2);
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 22, 29, 22, 29, 0, "SIjbA");
    dpd_->buf4_sort(&S2, PSIF_EOM_TMP, pqsr, 22, 28, "SIjAb");
    dpd_->buf4_close(&S2);
    dpd_->buf4_init(&S2, PSIF_EOM_TMP, C_irr, 22, 28, 22, 28, 0, "SIjAb");
    dpd_->buf4_init(&SIjAb, PSIF_EOM_SIjAb, C_irr, 22, 28, 22, 28, 0, SIjAb_lbl);
    dpd_->buf4_axpy(&S2, &SIjAb, 1);
    dpd_->buf4_close(&SIjAb);
    dpd_->buf4_close(&S2);
  }

#ifdef EOM_DEBUG
  check_sum("WmbejDD",i,C_irr);
#endif
  return;
}

}} // namespace psi::cceom
