#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <sstream>

#include <psifiles.h>
#include <libciomr/libciomr.h>
#include <libpsio/psio.h>
#include <libchkpt/chkpt.h>
#include <libpsio/psio.hpp>
#include <libchkpt/chkpt.hpp>
#include <libiwl/iwl.h>
#include <libqt/qt.h>
#include <libmints/mints.h>
#include <psi4-dec.h>

#include "dftsapt.h"
#include "infsapt.h"

using namespace boost;

namespace psi { namespace dftsapt {

PsiReturnType dftsapt(boost::shared_ptr<Wavefunction> dimer, 
                      boost::shared_ptr<Wavefunction> mA, 
                      boost::shared_ptr<Wavefunction> mB)
{
    tstart();

    boost::shared_ptr<DFTSAPT> sapt = DFTSAPT::build(dimer,mA,mB);
    sapt->compute_energy();

    tstop();

    return Success;
}

PsiReturnType infsapt(boost::shared_ptr<Wavefunction> dimer, 
                      boost::shared_ptr<Wavefunction> mA, 
                      boost::shared_ptr<Wavefunction> mB)
{
    tstart();

    std::vector<boost::shared_ptr<Wavefunction> > m;
    m.push_back(mA);
    m.push_back(mB);

    boost::shared_ptr<INFSAPT> sapt = INFSAPT::build(dimer,m);
    sapt->compute_energy();

    tstop();

    return Success;
}

}}
