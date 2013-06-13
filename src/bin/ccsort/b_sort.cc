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
    \ingroup CCSORT
    \brief Enter brief description of file here 
*/
#include <cmath>
#include <libpsio/psio.h>
#include <libdpd/dpd.h>
#include "MOInfo.h"
#include "Params.h"
#define EXTERN
#include "globals.h"

namespace psi { namespace ccsort {

void b_sort(void)
{
  dpdbuf4 B, B_s, B_a;
  int h, m, ab, cd, a, b, c, d, AB, CD, DC, Gc, C, cc;
  int rows_per_bucket, rows_left, nbuckets, row_start, nvirt;
  double **B_diag, abcd, abdc;
  psio_address next;

  /* B(+) = <ab|cd> + <ab|dc> */
  /* B(-) = <ab|cd> - <ab|dc> */
  if(params.ref == 0) { /* RHF references only */
    dpd_->buf4_init(&B, PSIF_CC_BINTS, 0, 5, 5, 5, 5, 0, "B <ab|cd>");
    dpd_->buf4_init(&B_s, PSIF_CC_BINTS, 0, 8, 8, 8, 8, 0, "B(+) <ab|cd> + <ab|dc>");
    dpd_->buf4_init(&B_a, PSIF_CC_BINTS, 0, 9, 9, 9, 9, 0, "B(-) <ab|cd> - <ab|dc>");
    dpd_->buf4_scm(&B_s, 0);
    dpd_->buf4_scm(&B_a, 0);
    for(h=0; h < moinfo.nirreps; h++) {
      dpd_->buf4_mat_irrep_row_init(&B, h);
      rows_per_bucket = 0;
      if(B_s.params->coltot[h]) 
    rows_per_bucket = dpd_->dpd_memfree()/(2 * B_s.params->coltot[h]);
      if(rows_per_bucket > B_s.params->rowtot[h]) rows_per_bucket = B_s.params->rowtot[h];
      nbuckets = (int) ceil((double) B_s.params->rowtot[h]/(double) rows_per_bucket);
      rows_left = 0;
      if(rows_per_bucket)
       	rows_left = B_s.params->rowtot[h] % rows_per_bucket;

      dpd_->buf4_mat_irrep_init_block(&B_s, h, rows_per_bucket);
      dpd_->buf4_mat_irrep_init_block(&B_a, h, rows_per_bucket);

      for(m=0; m < (rows_left ? nbuckets-1:nbuckets); m++) {
	row_start = m * rows_per_bucket;

	for(ab=0; ab < rows_per_bucket; ab++) {
	  a = B_s.params->roworb[h][ab+row_start][0];
	  b = B_s.params->roworb[h][ab+row_start][1];
	  AB = B.params->rowidx[a][b];
	  dpd_->buf4_mat_irrep_row_rd(&B, h, AB);
	  for(cd=0; cd < B_s.params->coltot[h]; cd++) {
	    c = B_s.params->colorb[h][cd][0];
	    d = B_s.params->colorb[h][cd][1];
	    CD = B.params->colidx[c][d];
	    DC = B.params->colidx[d][c];
	    abcd = B.matrix[h][0][CD];
	    abdc = B.matrix[h][0][DC];

	    B_s.matrix[h][ab][cd] = abcd + abdc;
	    B_a.matrix[h][ab][cd] = abcd - abdc;
	  }
	}

	dpd_->buf4_mat_irrep_wrt_block(&B_s, h, row_start, rows_per_bucket);
	dpd_->buf4_mat_irrep_wrt_block(&B_a, h, row_start, rows_per_bucket);
      }
      if(rows_left) {
	row_start = m * rows_per_bucket;

	for(ab=0; ab < rows_left; ab++) {
	  a = B_s.params->roworb[h][ab+row_start][0];
	  b = B_s.params->roworb[h][ab+row_start][1];
	  AB = B.params->rowidx[a][b];
	  dpd_->buf4_mat_irrep_row_rd(&B, h, AB);
	  for(cd=0; cd < B_s.params->coltot[h]; cd++) {
	    c = B_s.params->colorb[h][cd][0];
	    d = B_s.params->colorb[h][cd][1];
	    CD = B.params->colidx[c][d];
	    DC = B.params->colidx[d][c];
	    abcd = B.matrix[h][0][CD];
	    abdc = B.matrix[h][0][DC];

	    B_s.matrix[h][ab][cd] = abcd + abdc;
	    B_a.matrix[h][ab][cd] = abcd - abdc;
	  }
	}

	dpd_->buf4_mat_irrep_wrt_block(&B_s, h, row_start, rows_left);
	dpd_->buf4_mat_irrep_wrt_block(&B_a, h, row_start, rows_left);
      }

      dpd_->buf4_mat_irrep_close_block(&B_s, h, rows_per_bucket);
      dpd_->buf4_mat_irrep_close_block(&B_a, h, rows_per_bucket);
      dpd_->buf4_mat_irrep_row_close(&B, h);
    }
    dpd_->buf4_close(&B_a);
    dpd_->buf4_close(&B_s);
    dpd_->buf4_close(&B);

    /* Generate <ab|cc> components of B(+) */
    for(h=0,nvirt=0; h < moinfo.nirreps; h++) nvirt += moinfo.virtpi[h];
    dpd_->buf4_init(&B_s, PSIF_CC_BINTS, 0, 8, 8, 8, 8, 0, "B(+) <ab|cd> + <ab|dc>");

    rows_per_bucket = dpd_->dpd_memfree()/(B_s.params->coltot[0] + nvirt);
    if(rows_per_bucket > B_s.params->rowtot[0]) rows_per_bucket = B_s.params->rowtot[0];
    nbuckets = (int) ceil((double) B_s.params->rowtot[0]/(double) rows_per_bucket);
    rows_left = B_s.params->rowtot[0] % rows_per_bucket;

    dpd_->buf4_mat_irrep_init_block(&B_s, 0, rows_per_bucket);
    B_diag = dpd_->dpd_block_matrix(rows_per_bucket, nvirt);

    next = PSIO_ZERO;
    for(m=0; m < (rows_left ? nbuckets-1:nbuckets); m++) {
      row_start = m * rows_per_bucket;
      dpd_->buf4_mat_irrep_rd_block(&B_s, 0, row_start, rows_per_bucket);
      for(ab=0; ab < rows_per_bucket; ab++)
	for(Gc=0; Gc < moinfo.nirreps; Gc++)
	  for(C=0; C < moinfo.virtpi[Gc]; C++) {
	    c = moinfo.vir_off[Gc] + C;
	    cc = B_s.params->colidx[c][c];
	    B_diag[ab][c] = B_s.matrix[0][ab][cc];
	  }
      psio_write(PSIF_CC_BINTS, "B(+) <ab|cc>", (char *) B_diag[0], rows_per_bucket*nvirt*sizeof(double), next, &next);
    }
    if(rows_left) {
      row_start = m * rows_per_bucket;
      dpd_->buf4_mat_irrep_rd_block(&B_s, 0, row_start, rows_left);
      for(ab=0; ab < rows_left; ab++)
	for(Gc=0; Gc < moinfo.nirreps; Gc++)
	  for(C=0; C < moinfo.virtpi[Gc]; C++) {
	    c = moinfo.vir_off[Gc] + C;
	    cc = B_s.params->colidx[c][c];
	    B_diag[ab][c] = B_s.matrix[0][ab][cc];
	  }
      psio_write(PSIF_CC_BINTS, "B(+) <ab|cc>", (char *) B_diag[0], rows_left*nvirt*sizeof(double), next, &next);
    }
    dpd_->free_dpd_block(B_diag, rows_per_bucket, nvirt);
    dpd_->buf4_mat_irrep_close_block(&B_s, 0, rows_per_bucket);
    dpd_->buf4_close(&B_s);
  }
}

}} // namespace psi::ccsort
