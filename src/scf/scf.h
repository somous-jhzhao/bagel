//
// BAGEL - Parallel electron correlation program.
// Filename: scf.h
// Copyright (C) 2012 Toru Shiozaki
//
// Author: Toru Shiozaki <shiozaki@northwestern.edu>
// Maintainer: Shiozaki group
//
// This file is part of the BAGEL package.
//
// The BAGEL package is free software; you can redistribute it and\/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// The BAGEL package is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with the BAGEL package; see COPYING.  If not, write to
// the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//


#ifndef __BAGEL_SRC_SCF_SCF_H
#define __BAGEL_SRC_SCF_SCF_H

#include <src/scf/scf_base.h>
#include <src/scf/levelshift.h>

namespace bagel {

class SCF : public SCF_base {
  protected:
    std::shared_ptr<LevelShift<DistMatrix>> levelshift_;
    const bool dodf_;

  public:
    SCF(const std::shared_ptr<const PTree> idata_, const std::shared_ptr<const Geometry> geom,
        const std::shared_ptr<const Reference> re = std::shared_ptr<const Reference>());

    void compute() override;

    std::shared_ptr<Reference> conv_to_ref() const;

    bool dodf() const { return dodf_; }

};

}

#endif
