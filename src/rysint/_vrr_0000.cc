//
// Newint - Parallel electron correlation program.
// Filename: _vrr_0000.cc
// Copyright (C) 2009 Toru Shiozaki
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


#include <src/rysint/vrrlist.h>

// returns double array of length 1
void VRRList::_vrr_0000(double* data_, const double* C00, const double* D00, const double* B00, const double* B01, const double* B10) {
#ifdef __GNUC__
  double C00_[1]__attribute__((aligned(16))) = {C00[0]};
  double D00_[1]__attribute__((aligned(16))) = {D00[0]};
  double B00_[1]__attribute__((aligned(16))) = {B00[0]};
  double B01_[1]__attribute__((aligned(16))) = {B01[0]};
  double B10_[1]__attribute__((aligned(16))) = {B10[0]};
#else
  double* C00_ = C00;
  double* D00_ = D00;
  double* B00_ = B00;
  double* B01_ = B01;
  double* B10_ = B10;
#endif

  data_[0] = 1.0;
}

