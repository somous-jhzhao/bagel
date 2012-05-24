//
// Newint - Parallel electron correlation program.
// Filename: cphf.h
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


#ifndef __SRC_GRAD_CPHF_H
#define __SRC_GRAD_CPHF_H

#include <src/scf/matrix1e.h>
#include <memory>
#include <src/util/linear.h>
#include <src/wfn/reference.h>

class CPHF {
  protected:
    const std::shared_ptr<Linear<Matrix1e> > solver_;
    const std::shared_ptr<const Matrix1e> grad_;
    const std::vector<double> eig_;
    const std::shared_ptr<DF_Half> halfjj_;
    const std::shared_ptr<const Reference> ref_;
    const std::shared_ptr<const Geometry> geom_;

  public:
    CPHF(const std::shared_ptr<const Matrix1e> grad, const std::vector<double>& eig,
         const std::shared_ptr<DF_Half> half, const std::shared_ptr<const Reference> g);
    ~CPHF() {};

    std::shared_ptr<Matrix1e> solve() const;

};

#endif

