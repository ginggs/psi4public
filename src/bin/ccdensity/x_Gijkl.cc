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

/* Gijkl(): computes non-R0 parts of Gijkl 2pdm */
/* Dijkl = 0.5 Lklef * Rijef + P(ij) Lklef * Rie * tjf */
/* Dijkl = R2L2_OOOO + P(ij) Lklef * Rie * tjf */

void x_Gijkl(void)
{
  dpdfile2 R1, T1;
  dpdbuf4 L2, I2, GIJKL, Gijkl, GIjKl;
  int L_irr, R_irr, G_irr;
  double value;
  L_irr = params.L_irr;
  R_irr = params.R_irr;
  G_irr = params.G_irr;

  if (params.ref == 0 || params.ref == 1) {
    /* Gijkl += R2L2_OOOO */
    dpd_->buf4_init(&GIJKL, PSIF_CC_GAMMA, G_irr, 2, 2, 2, 2, 0, "GIJKL");
    dpd_->buf4_init(&I2, PSIF_EOM_TMP, G_irr, 2, 2, 2, 2, 0, "R2L2_OOOO");
    dpd_->buf4_axpy(&I2, &GIJKL, 1.0);
    dpd_->buf4_close(&I2);
    dpd_->buf4_close(&GIJKL);

    dpd_->buf4_init(&Gijkl, PSIF_CC_GAMMA, G_irr, 2, 2, 2, 2, 0, "Gijkl");
    dpd_->buf4_init(&I2, PSIF_EOM_TMP, G_irr, 2, 2, 2, 2, 0, "R2L2_oooo");
    dpd_->buf4_axpy(&I2, &Gijkl, 1.0);
    dpd_->buf4_close(&I2);
    dpd_->buf4_close(&Gijkl);

    dpd_->buf4_init(&GIjKl, PSIF_CC_GAMMA, G_irr, 0, 0, 0, 0, 0, "GIjKl");
    dpd_->buf4_init(&I2, PSIF_EOM_TMP, G_irr, 0, 0, 0, 0, 0, "R2L2_OoOo");
    dpd_->buf4_axpy(&I2, &GIjKl, 1.0);
    dpd_->buf4_close(&I2);
    dpd_->buf4_close(&GIjKl);
  }
  else {
    /* Gijkl += R2L2_OOOO */
    dpd_->buf4_init(&GIJKL, PSIF_CC_GAMMA, G_irr, 2, 2, 2, 2, 0, "GIJKL");
    dpd_->buf4_init(&I2, PSIF_EOM_TMP, G_irr, 2, 2, 2, 2, 0, "R2L2_OOOO");
    dpd_->buf4_axpy(&I2, &GIJKL, 1.0);
    dpd_->buf4_close(&I2);
    dpd_->buf4_close(&GIJKL);

    dpd_->buf4_init(&Gijkl, PSIF_CC_GAMMA, G_irr, 12, 12, 12, 12, 0, "Gijkl");
    dpd_->buf4_init(&I2, PSIF_EOM_TMP, G_irr, 12, 12, 12, 12, 0, "R2L2_oooo");
    dpd_->buf4_axpy(&I2, &Gijkl, 1.0);
    dpd_->buf4_close(&I2);
    dpd_->buf4_close(&Gijkl);

    dpd_->buf4_init(&GIjKl, PSIF_CC_GAMMA, G_irr, 22, 22, 22, 22, 0, "GIjKl");
    dpd_->buf4_init(&I2, PSIF_EOM_TMP, G_irr, 22, 22, 22, 22, 0, "R2L2_OoOo");
    dpd_->buf4_axpy(&I2, &GIjKl, 1.0);
    dpd_->buf4_close(&I2);
    dpd_->buf4_close(&GIjKl);
  }

  if (params.ref == 0 || params.ref == 1) {
    /* GIJKL += - (Lklfe rie) tjf */
    dpd_->buf4_init(&GIJKL, PSIF_CC_GAMMA, G_irr, 0, 2, 2, 2, 0, "GIJKL");
    dpd_->buf4_init(&I2, PSIF_EOM_TMP, G_irr, 2, 10, 2, 10, 0, "L2R1_OOVO(pqsr)");
    dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
    dpd_->contract424(&I2, &T1, &GIJKL, 3, 1, 1, -1.0, 1.0);
    dpd_->file2_close(&T1);
    dpd_->buf4_close(&I2);

    dpd_->buf4_init(&I2, PSIF_EOM_TMP, G_irr, 2, 11, 2, 11, 0, "L2R1_OOVO");
    dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
    dpd_->contract244(&T1, &I2, &GIJKL, 1, 2, 0, 1.0, 1.0);
    dpd_->file2_close(&T1);
    dpd_->buf4_close(&I2);
    dpd_->buf4_close(&GIJKL);

    dpd_->buf4_init(&Gijkl, PSIF_CC_GAMMA, G_irr, 0, 2, 2, 2, 0, "Gijkl");
    dpd_->buf4_init(&I2, PSIF_EOM_TMP, G_irr, 2, 10, 2, 10, 0, "L2R1_oovo(pqsr)");
    dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
    dpd_->contract424(&I2, &T1, &Gijkl, 3, 1, 1, -1.0, 1.0);
    dpd_->file2_close(&T1);
    dpd_->buf4_close(&I2);

    dpd_->buf4_init(&I2, PSIF_EOM_TMP, G_irr, 2, 11, 2, 11, 0, "L2R1_oovo");
    dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
    dpd_->contract244(&T1, &I2, &Gijkl, 1, 2, 0, 1.0, 1.0);
    dpd_->file2_close(&T1);
    dpd_->buf4_close(&I2);
    dpd_->buf4_close(&Gijkl);

    dpd_->buf4_init(&GIjKl, PSIF_CC_GAMMA, G_irr, 0, 0, 0, 0, 0, "GIjKl");
    dpd_->buf4_init(&I2, PSIF_EOM_TMP, G_irr, 0, 10, 0, 10, 0, "L2R1_OovO(pqsr)");
    dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tia");
    dpd_->contract424(&I2, &T1, &GIjKl, 3, 1, 1, 1.0, 1.0);
    dpd_->file2_close(&T1);
    dpd_->buf4_close(&I2);

    dpd_->buf4_init(&I2, PSIF_EOM_TMP, G_irr, 0, 11, 0, 11, 0, "L2R1_OoVo");
    dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
    dpd_->contract244(&T1, &I2, &GIjKl, 1, 2, 0, 1.0, 1.0);
    dpd_->file2_close(&T1);
    dpd_->buf4_close(&I2);
    dpd_->buf4_close(&GIjKl);
  }
  else {
    /* GIJKL += - (Lklfe rie) tjf */
    dpd_->buf4_init(&GIJKL, PSIF_CC_GAMMA, G_irr, 0, 2, 2, 2, 0, "GIJKL");
    dpd_->buf4_init(&I2, PSIF_EOM_TMP, G_irr, 2, 20, 2, 20, 0, "L2R1_OOVO(pqsr)");
    dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
    dpd_->contract424(&I2, &T1, &GIJKL, 3, 1, 1, -1.0, 1.0);
    dpd_->file2_close(&T1);
    dpd_->buf4_close(&I2);

    dpd_->buf4_init(&I2, PSIF_EOM_TMP, G_irr, 2, 21, 2, 21, 0, "L2R1_OOVO");
    dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
    dpd_->contract244(&T1, &I2, &GIJKL, 1, 2, 0, 1.0, 1.0);
    dpd_->file2_close(&T1);
    dpd_->buf4_close(&I2);
    dpd_->buf4_close(&GIJKL);

    dpd_->buf4_init(&Gijkl, PSIF_CC_GAMMA, G_irr, 10, 12, 12, 12, 0, "Gijkl");
    dpd_->buf4_init(&I2, PSIF_EOM_TMP, G_irr, 12, 30, 12, 30, 0, "L2R1_oovo(pqsr)");
    dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 2, 3, "tia");
    dpd_->contract424(&I2, &T1, &Gijkl, 3, 1, 1, -1.0, 1.0);
    dpd_->file2_close(&T1);
    dpd_->buf4_close(&I2);

    dpd_->buf4_init(&I2, PSIF_EOM_TMP, G_irr, 12, 31, 12, 31, 0, "L2R1_oovo");
    dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 2, 3, "tia");
    dpd_->contract244(&T1, &I2, &Gijkl, 1, 2, 0, 1.0, 1.0);
    dpd_->file2_close(&T1);
    dpd_->buf4_close(&I2);
    dpd_->buf4_close(&Gijkl);

    dpd_->buf4_init(&GIjKl, PSIF_CC_GAMMA, G_irr, 22, 22, 22, 22, 0, "GIjKl");
    dpd_->buf4_init(&I2, PSIF_EOM_TMP, G_irr, 22, 24, 22, 24, 0, "L2R1_OovO(pqsr)");
    dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 2, 3, "tia");
    dpd_->contract424(&I2, &T1, &GIjKl, 3, 1, 1, 1.0, 1.0);
    dpd_->file2_close(&T1);
    dpd_->buf4_close(&I2);

    dpd_->buf4_init(&I2, PSIF_EOM_TMP, G_irr, 22, 26, 22, 26, 0, "L2R1_OoVo");
    dpd_->file2_init(&T1, PSIF_CC_OEI, 0, 0, 1, "tIA");
    dpd_->contract244(&T1, &I2, &GIjKl, 1, 2, 0, 1.0, 1.0);
    dpd_->file2_close(&T1);
    dpd_->buf4_close(&I2);
    dpd_->buf4_close(&GIjKl);
  }

  /* Now bra-ket symmetrize Gijkl */
  if (params.ref == 0 || params.ref == 1) {
    dpd_->buf4_init(&GIJKL, PSIF_CC_GAMMA, G_irr, 2, 2, 2, 2, 0, "GIJKL");
    dpd_->buf4_symm(&GIJKL);
    dpd_->buf4_close(&GIJKL);
    dpd_->buf4_init(&Gijkl, PSIF_CC_GAMMA, G_irr, 2, 2, 2, 2, 0, "Gijkl");
    dpd_->buf4_symm(&Gijkl);
    dpd_->buf4_close(&Gijkl);
    dpd_->buf4_init(&GIjKl, PSIF_CC_GAMMA, G_irr, 0, 0, 0, 0, 0, "GIjKl");
    dpd_->buf4_symm(&GIjKl);
    dpd_->buf4_close(&GIjKl);
  }
  else {
    dpd_->buf4_init(&GIJKL, PSIF_CC_GAMMA, G_irr, 2, 2, 2, 2, 0, "GIJKL");
    dpd_->buf4_symm(&GIJKL);
    dpd_->buf4_close(&GIJKL);
    dpd_->buf4_init(&Gijkl, PSIF_CC_GAMMA, G_irr, 12, 12, 12, 12, 0, "Gijkl");
    dpd_->buf4_symm(&Gijkl);
    dpd_->buf4_close(&Gijkl);
    dpd_->buf4_init(&GIjKl, PSIF_CC_GAMMA, G_irr, 22, 22, 22, 22, 0, "GIjKl");
    dpd_->buf4_symm(&GIjKl);
    dpd_->buf4_close(&GIjKl);
  }
  return;
}
 

}} // namespace psi::ccdensity
