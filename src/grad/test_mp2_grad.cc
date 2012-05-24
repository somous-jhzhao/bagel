//
// Newint - Parallel electron correlation program.
// Filename: test_mp2_grad.cc
// Copyright (C) 2012 Toru Shiozaki
//
// Author: Toru Shiozaki <shiozaki@northwestern.edu>
// Maintainer: Shiozaki group
//
// This file is part of the Newint package (to be renamed).
//
// The Newint package is free software; you can redistribute it and\/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// The Newint package is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with the Newint package; see COPYING.  If not, write to
// the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//


// testing MP2 gradients

#include <src/wfn/reference.h>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <src/prop/dipole.h>
using namespace std;

// plain implementation of MP2 gradient to be replaced

static size_t nocc;
static size_t nvirt;

size_t m(size_t i, size_t j, size_t k, size_t l) { return i+nocc*(j+nvirt*(k+nocc*l)); };
// last index is nvirt
size_t mo(size_t i, size_t j, size_t k, size_t l) { return i+nocc*(j+nocc*(k+nocc*l)); };
// first index is nocc
size_t mv(size_t i, size_t j, size_t k, size_t l) { return i+nocc*(j+nvirt*(k+nvirt*l)); };

void test_mp2_grad(shared_ptr<Reference> ref) {


  cout << "   testing MP2 grad implementation" << endl;

  nocc = ref->nocc();
  nvirt = ref->nvirt();
  const size_t nbasis = nocc+nvirt;
  shared_ptr<const Geometry> geom = ref->geom();
  shared_ptr<const DensityFit> df = geom->df();

  assert(ref->nact() == 0 && nbasis == geom->nbasis());

  const double* c = ref->coeff()->data();
  const double* cv = c + nocc*nbasis;
  shared_ptr<DF_Half> half = df->compute_half_transform(c, nocc)->apply_J();
  shared_ptr<DF_Full> full = half->compute_second_transform(cv, nvirt);
  shared_ptr<DF_Full> fullo = half->compute_second_transform(c, nocc);

  shared_ptr<DF_Half> halfv = df->compute_half_transform(cv, nvirt)->apply_J();
  shared_ptr<DF_Full> fullv = halfv->compute_second_transform(cv, nvirt);

  unique_ptr<double[]> kijab = full->form_4index();

  unique_ptr<double[]> tijab(new double[nocc*nocc*nvirt*nvirt]);
  unique_ptr<double[]> cijab(new double[nocc*nocc*nvirt*nvirt]);

  unique_ptr<double[]> kijkb = fullo->form_4index(full);
  unique_ptr<double[]> kicab = full->form_4index(fullv);
  unique_ptr<double[]> jijab = fullo->form_4index(fullv);

  // TODO ncore == 0 assumed
  vector<double> eig = ref->eig();

  for (int i = 0; i != nvirt; ++i) {
    for (int j = 0; j != nocc; ++j) {
      for (int k = 0; k != nvirt; ++k) {
        for (int l = 0; l != nocc; ++l) {
          tijab[m(l,k,j,i)] = (2*kijab[m(l,k,j,i)] - kijab[m(l,i,j,k)])/ (-eig[i+nocc]+eig[j]-eig[k+nocc]+eig[l]);
          cijab[m(l,k,j,i)] = kijab[m(l,k,j,i)] / (-eig[i+nocc]+eig[j]-eig[k+nocc]+eig[l]);
        }
      }
    }
  }
  // ddot should give a correlation energy
  cout << "     en : " << setprecision(10) << ddot_(nocc*nocc*nvirt*nvirt, tijab, 1, kijab, 1) << endl;

  // density matrices
  shared_ptr<Matrix1e> dmp2(new Matrix1e(geom));
  for (int i = 0; i != nocc; ++i) {
    for (int j = 0; j != nocc; ++j) {
      double tmp = 0.0; 
      for (int k = 0; k != nocc; ++k) {
        for (int a = 0; a != nvirt; ++a) {
          for (int b = 0; b != nvirt; ++b) {
            tmp += -2.0*tijab[m(i,a,k,b)]*cijab[m(j,a,k,b)];
          }
        }
      }
      dmp2->element(j,i) = tmp;
    }
  }
  
  for (int a = 0; a != nvirt; ++a) {
    for (int b = 0; b != nvirt; ++b) {
      double tmp = 0.0;
      for (int i = 0; i != nocc; ++i) {
        for (int j = 0; j != nocc; ++j) {
          for (int c = 0; c != nvirt; ++c) {
            tmp += 2.0*tijab[m(i,a,j,c)]*cijab[m(i,b,j,c)];
          }
        }
      }
      dmp2->element(b+nocc, a+nocc) = tmp;
    }
  }

  // right hand side
  unique_ptr<double[]> lai(new double[nvirt*nocc]);
  fill(lai.get(), lai.get()+nvirt*nocc, 0.0);
  for (int i = 0; i != nocc; ++i) {
    for (int a = 0; a != nvirt; ++a) {
      for (int j = 0; j != nocc; ++j) {
        for (int k = 0; k != nocc; ++k) {
          lai[i+nocc*a] += dmp2->element(j,k)*(4.0*kijkb[mo(j,k,i,a)]-2.0*kijkb[mo(i,k,j,a)]);
        }
      }
      for (int b = 0; b != nvirt; ++b) {
        for (int c = 0; c != nvirt; ++c) {
          lai[i+nocc*a] += dmp2->element(b+nocc,c+nocc)*(4.0*kicab[mv(i,a,b,c)]-2.0*kicab[mv(i,b,a,c)]);
        }
      }
      for (int j = 0; j != nocc; ++j) {
        for (int k = 0; k != nocc; ++k) {
          for (int b = 0; b != nvirt; ++b) {
            lai[i+nocc*a] += (-4.0)*tijab[m(j,a,k,b)]*kijkb[mo(i,j,k,b)];
          }
        }
      }
      for (int j = 0; j != nocc; ++j) {
        for (int b = 0; b != nvirt; ++b) {
          for (int c = 0; c != nvirt; ++c) {
            lai[i+nocc*a] += 4.0*tijab[m(i,b,j,c)]*kicab[mv(j,c,a,b)];
          }
        }
      }
    }
  }

  // printout right hand side
#if 0
  cout << "  -- printing out the right hand side --" << endl;
  for (int a = 0; a != nvirt; ++a) {
    for (int i = 0; i != nocc; ++i) {
      cout << setw(15) << setprecision(10) << lai[i+nocc*a];
    }
    cout << endl;
  }
  cout << "  --------------------------------------" << endl;
#endif

  // solve directly
  unique_ptr<double[]> left(new double[nocc*nocc*nvirt*nvirt]);
  double* t = left.get();
  for (int a = 0; a != nvirt; ++a) {
    for (int i = 0; i != nocc; ++i) {
      for (int b = 0; b != nvirt; ++b) {
        for (int j = 0; j != nocc; ++j, ++t) {
          *t = 4.0*kijab[m(i,a,j,b)] - kijab[m(j,a,i,b)] - jijab[i+nocc*(j+nocc*(b+nvirt*a))] + ((i == j && a == b) ? eig[b+nocc]-eig[j] : 0.0);
        }
      }
    }
  }
  dscal_(nocc*nvirt, -1.0, lai, 1);

  unique_ptr<int[]> ipiv(new int[nocc*nvirt]);
  int info;
  dgesv_(nocc*nvirt, 1, left.get(), nocc*nvirt, ipiv.get(), lai.get(), nocc*nvirt, info); 
  if (info) throw logic_error("strange");

#if 0
  // printout right hand side
  cout << "  -- printing out the right hand side --" << endl;
  for (int a = 0; a != nvirt; ++a) {
    for (int i = 0; i != nocc; ++i) {
      cout << setw(15) << setprecision(10) << lai[i+nocc*a];
    }
    cout << endl;
  }
  cout << "  --------------------------------------" << endl;
#endif


  for (int a = 0; a != nvirt; ++a) {
    for (int i = 0; i != nocc; ++i) {
      dmp2->element(a+nocc, i)  = dmp2->element(i, a+nocc) = lai[i+nocc*a]*0.5;
    }
  } 

  shared_ptr<Matrix1e> dtot(new Matrix1e(*dmp2));
  for (int i = 0; i != nocc; ++i) dtot->element(i,i) += 2.0;

  // computes dipole mements
  shared_ptr<Matrix1e> cinv(new Matrix1e(*ref->coeff()));
  shared_ptr<Matrix1e> dmp2ao(new Matrix1e(*cinv * *dtot ^ *cinv));
  Dipole dipole(geom, dmp2ao);
  dipole.compute();

#if 0
  // Wij(I)
  unique_ptr<double[]> wijI(new double[nocc*nocc]);
  for (int i = 0; i != nocc; ++i) {
    for (int j = 0; j != nocc; ++j) {
      double tmp = 0.0;
      for (int k = 0; k != nocc; ++k) {
        for (int a = 0; a != nvirt; ++a) {
          for (int b = 0; b != nvirt; ++b) {
            tmp += -2*tijab[m(i,a,k,b)]*kijab[m(j,a,k,b)];
          }
        }
      }
      wijI[j+nocc*i] = tmp;
    }
  }
  unique_ptr<double[]> wabI(new double[nvirt*nvirt]);
  for (int a = 0; a != nvirt; ++a) {
    for (int b = 0; b != nvirt; ++b) {
      double tmp = 0.0;
      for (int i = 0; i != nocc; ++i) {
        for (int j = 0; j != nocc; ++j) {
          for (int c = 0; c != nvirt; ++c) {
            tmp += -2*tijab[m(i,a,j,c)]*kijab[m(i,b,j,c)];
          }
        }
      }
      wabI[b+nvirt*a] = tmp;
    }
  }
#endif

};
