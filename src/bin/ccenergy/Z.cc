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
    \ingroup CCENERGY
    \brief Enter brief description of file here 
*/
#include <cstdio>
#include <cstdlib>
#include <libdpd/dpd.h>
#include <libqt/qt.h>
#include "Params.h"
#define EXTERN
#include "globals.h"

namespace psi { namespace ccenergy {

void Z_build(void)
{
  dpdbuf4 ZIJMA, Zijma, ZIjMa, ZIjmA, ZIjAm, ZMaIj, ZmAIj, Z;
  dpdbuf4 tauIJAB, tauijab, tauIjAb, tauIjbA, F_anti, F, tau;
  int Gmb, Gij, mb, nrows, ncols;

#ifdef TIME_CCENERGY
  timer_on("Z");
#endif

  if(params.ref == 0) { /** RHF **/
    /* ZMbIj = <Mb|Ef> * tau(Ij,Ef) */
    /* OOC code added 3/23/05  -TDC */
    dpd_->buf4_init(&Z, PSIF_CC_MISC, 0, 10, 0, 10, 0, 0, "ZMbIj");
    dpd_->buf4_init(&F, PSIF_CC_FINTS, 0, 10, 5, 10, 5, 0, "F <ia|bc>");
    dpd_->buf4_init(&tau, PSIF_CC_TAMPS, 0, 0, 5, 0, 5, 0, "tauIjAb");
    dpd_->contract444(&F, &tau, &Z, 0, 0, 1, 0);
/*     for(Gmb=0; Gmb < moinfo.nirreps; Gmb++) { */
/*       Gij = Gmb;  /\* tau is totally symmetric *\/ */
/*       dpd_buf4_mat_irrep_init(&tau, Gij); */
/*       dpd_buf4_mat_irrep_rd(&tau, Gij); */

/*       dpd_buf4_mat_irrep_init(&Z, Gmb); */

/*       dpd_buf4_mat_irrep_row_init(&F, Gmb); */

/*       for(mb=0; mb < F.params->rowtot[Gmb]; mb++) { */
/* 	dpd_buf4_mat_irrep_row_rd(&F, Gmb, mb); */

/* 	nrows = tau.params->rowtot[Gij]; */
/* 	ncols = tau.params->coltot[Gij]; */
/* 	if(nrows && ncols) */
/* 	  C_DGEMV('n',nrows,ncols,1.0,tau.matrix[Gij][0],ncols,F.matrix[Gmb][0],1, */
/* 		  0.0,Z.matrix[Gmb][mb],1); */
/*       } */

/*       dpd_buf4_mat_irrep_row_close(&F, Gmb); */
/*       dpd_buf4_mat_irrep_wrt(&Z, Gmb); */
/*       dpd_buf4_mat_irrep_close(&Z, Gmb); */
/*       dpd_buf4_mat_irrep_close(&tau, Gij); */
/*     } */
    dpd_->buf4_close(&tau);
    dpd_->buf4_close(&F);
    dpd_->buf4_close(&Z);  
  }
  else if(params.ref == 1) { /** ROHF **/
    dpd_->buf4_init(&ZIJMA, PSIF_CC_MISC, 0, 2, 10, 2, 10, 0, "ZIJMA");
    dpd_->buf4_init(&Zijma, PSIF_CC_MISC, 0, 2, 10, 2, 10, 0, "Zijma");
    dpd_->buf4_init(&ZIjMa, PSIF_CC_MISC, 0, 0, 10, 0, 10, 0, "ZIjMa");
    dpd_->buf4_init(&ZIjmA, PSIF_CC_MISC, 0, 0, 10, 0, 10, 0, "ZIjmA");

    dpd_->buf4_init(&tauIJAB, PSIF_CC_TAMPS, 0, 2, 7, 2, 7, 0, "tauIJAB");
    dpd_->buf4_init(&tauijab, PSIF_CC_TAMPS, 0, 2, 7, 2, 7, 0, "tauijab");
    dpd_->buf4_init(&tauIjAb, PSIF_CC_TAMPS, 0, 0, 5, 0, 5, 0, "tauIjAb");
    dpd_->buf4_init(&tauIjbA, PSIF_CC_TAMPS, 0, 0, 5, 0, 5, 0, "tauIjbA");

    dpd_->buf4_init(&F_anti, PSIF_CC_FINTS, 0, 10, 7, 10, 5, 1, "F <ia|bc>");
    dpd_->buf4_init(&F, PSIF_CC_FINTS, 0, 10, 5, 10, 5, 0, "F <ia|bc>");

    dpd_->contract444(&tauIJAB, &F_anti, &ZIJMA, 0, 0, 1, 0);
    dpd_->contract444(&tauijab, &F_anti, &Zijma, 0, 0, 1, 0);
    dpd_->contract444(&tauIjAb, &F, &ZIjMa, 0, 0, 1, 0);
    dpd_->contract444(&tauIjbA, &F, &ZIjmA, 0, 0, 1, 0);

    dpd_->buf4_close(&tauIJAB); 
    dpd_->buf4_close(&tauijab); 
    dpd_->buf4_close(&tauIjAb);
    dpd_->buf4_close(&tauIjbA);

    dpd_->buf4_close(&F_anti); 
    dpd_->buf4_close(&F);

    dpd_->buf4_sort(&ZIJMA, PSIF_CC_MISC, pqsr, 2, 11, "ZIJAM");
    dpd_->buf4_sort(&Zijma, PSIF_CC_MISC, pqsr, 2, 11, "Zijam");
    dpd_->buf4_sort(&ZIjmA, PSIF_CC_MISC, pqsr, 0, 11, "ZIjAm");

    dpd_->buf4_close(&ZIJMA);  
    dpd_->buf4_close(&Zijma);
    dpd_->buf4_close(&ZIjMa);  
    dpd_->buf4_close(&ZIjmA);
  }
  else if(params.ref == 2) { /*** UHF ***/

    dpd_->buf4_init(&ZIJMA, PSIF_CC_MISC, 0, 2, 20, 2, 20, 0, "ZIJMA");
    dpd_->buf4_init(&Zijma, PSIF_CC_MISC, 0, 12, 30, 12, 30, 0, "Zijma");
    dpd_->buf4_init(&ZIjMa, PSIF_CC_MISC, 0, 22, 24, 22, 24, 0, "ZIjMa");
    dpd_->buf4_init(&ZIjAm, PSIF_CC_MISC, 0, 22, 26, 22, 26, 0, "ZIjAm");

    dpd_->buf4_init(&tauIJAB, PSIF_CC_TAMPS, 0, 2, 7, 2, 7, 0, "tauIJAB");
    dpd_->buf4_init(&tauijab, PSIF_CC_TAMPS, 0, 12, 17, 12, 17, 0, "tauijab");
    dpd_->buf4_init(&tauIjAb, PSIF_CC_TAMPS, 0, 22, 28, 22, 28, 0, "tauIjAb");

    dpd_->buf4_init(&F, PSIF_CC_FINTS, 0, 20, 7, 20, 5, 1, "F <IA|BC>");
    dpd_->contract444(&tauIJAB, &F, &ZIJMA, 0, 0, 1, 0);
    dpd_->buf4_close(&F);

    dpd_->buf4_init(&F, PSIF_CC_FINTS, 0, 30, 17, 30, 15, 1, "F <ia|bc>");
    dpd_->contract444(&tauijab, &F, &Zijma, 0, 0, 1, 0);
    dpd_->buf4_close(&F);

    dpd_->buf4_init(&F, PSIF_CC_FINTS, 0, 24, 28, 24, 28, 0, "F <Ia|Bc>");
    dpd_->contract444(&tauIjAb, &F, &ZIjMa, 0, 0, 1, 0);
    dpd_->buf4_close(&F);

    dpd_->buf4_init(&F, PSIF_CC_FINTS, 0, 28, 26, 28, 26, 0, "F <Ab|Ci>");
    dpd_->contract444(&tauIjAb, &F, &ZIjAm, 0, 1, 1, 0);
    dpd_->buf4_close(&F);

    dpd_->buf4_close(&tauIJAB); 
    dpd_->buf4_close(&tauijab); 
    dpd_->buf4_close(&tauIjAb);

    dpd_->buf4_sort(&ZIJMA, PSIF_CC_MISC, pqsr, 2, 21, "ZIJAM");
    dpd_->buf4_sort(&Zijma, PSIF_CC_MISC, pqsr, 12, 31, "Zijam");

    dpd_->buf4_close(&ZIJMA);  
    dpd_->buf4_close(&Zijma);
    dpd_->buf4_close(&ZIjMa);  
    dpd_->buf4_close(&ZIjAm);

  }

#ifdef TIME_CCENERGY
  timer_off("Z");
#endif
}

}} // namespace psi::ccenergy
