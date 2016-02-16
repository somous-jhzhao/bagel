//
// BAGEL - Parallel electron correlation program.
// Filename: tensor.cc
// Copyright (C) 2014 Toru Shiozaki
//
// Author: Toru Shiozaki <shiozaki@northwestern.edu>
// Maintainer: Shiozaki group
//
// This file is part of the BAGEL package.
//
// The BAGEL package is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
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

#include <bagel_config.h>
#ifdef COMPILE_SMITH

#include <src/smith/tensor.h>
#include <src/smith/storagekramers.h>

using namespace std;
using namespace bagel;
using namespace bagel::SMITH;

template <typename DataType>
Tensor_<DataType>::Tensor_(vector<IndexRange> in, const bool kramers, const unordered_set<size_t> sparse, const bool alloc)
  : range_(in), rank_(in.size()), sparse_(sparse), initialized_(false), allocated_(alloc) {

  // make block list
  if (!in.empty()) {
    LoopGenerator lg(in);
    vector<vector<Index>> index = lg.block_loop();

    // first compute hashtags and length
    map<size_t, size_t> hashmap;
    for (auto& i : index) {
      size_t size = 1lu;
      vector<size_t> h;
      for (auto& j : i) {
        size *= j.size();
        h.push_back(j.key());
      }
      auto key = generate_hash_key(h);
      if (sparse.empty() || sparse.count(key))
        hashmap.emplace(key, size);
    }

    if (!kramers)
      data_ = make_shared<Storage<DataType>>(hashmap, alloc);
    else
      data_ = make_shared<StorageKramers<DataType>>(hashmap, alloc);
  } else {
    rank_ = 0;
    map<size_t, size_t> hashmap {{generate_hash_key(), 1lu}};
    data_ = make_shared<Storage<DataType>>(hashmap, alloc);
  }
}


template <typename DataType>
void Tensor_<DataType>::allocate() {
  assert(!allocated_);
  data_->initialize();
  allocated_ = true;
}


template <typename DataType>
size_t Tensor_<DataType>::size_alloc() const {
  return data_->size_alloc();
}


template <typename DataType>
vector<DataType> Tensor_<DataType>::diag() const {
  if (rank_ != 2 || range_.at(0) != range_.at(1))
    throw logic_error("Tensor_<DataType>::diag can be called only with a square tensor of rank 2");
  const size_t size = range_.at(0).back().offset() + range_.at(0).back().size();
  vector<DataType> buf(size);
  for (auto& i : range_.at(0)) {
    unique_ptr<DataType[]> data0 = get_block(i, i);
    for (int j = 0; j != i.size(); ++j) {
      buf[j+i.offset()] = data0[j+j*i.size()];
    }
  }
  return buf;
}


template <typename DataType>
shared_ptr<typename std::conditional<std::is_same<DataType,double>::value, Matrix, ZMatrix>::type> Tensor_<DataType>::matrix() const {
  vector<IndexRange> o = indexrange();
  assert(o.size() == 2);

  const int off0 = o[0].front().offset();
  const int off1 = o[1].front().offset();
  using MatType = typename std::conditional<std::is_same<DataType,double>::value, Matrix, ZMatrix>::type;
  auto out = make_shared<MatType>(o[0].size(), o[1].size());
  for (auto& i1 : o[1].range())
    for (auto& i0 : o[0].range())
      out->copy_block(i0.offset()-off0, i1.offset()-off1, i0.size(), i1.size(), get_block(i0, i1).get());

  return out;
}


// TODO parallelization
template <typename DataType>
shared_ptr<typename std::conditional<std::is_same<DataType,double>::value, Matrix, ZMatrix>::type> Tensor_<DataType>::matrix2() const {
  const vector<IndexRange> o = indexrange();
  assert(o.size() == 4);

  const int dim0 = o[0].size();
  const int dim1 = o[1].size();
  const int dim2 = o[2].size();
  const int dim3 = o[3].size();
  const int off0 = o[0].front().offset();
  const int off1 = o[1].front().offset();
  const int off2 = o[2].front().offset();
  const int off3 = o[3].front().offset();

  auto out = make_shared<typename std::conditional<std::is_same<DataType,double>::value, Matrix, ZMatrix>::type>(dim0*dim1,dim2*dim3);
  for (auto& i3 : o[3].range()) {
    for (auto& i2 : o[2].range()) {
      for (auto& i1 : o[1].range()) {
        for (auto& i0 : o[0].range()) {
          if (exists(i0, i1, i2, i3)) {
            unique_ptr<DataType[]> target = get_block(i0, i1, i2, i3);
            const DataType* ptr = target.get();
            for (int j3 = i3.offset(); j3 != i3.offset()+i3.size(); ++j3)
              for (int j2 = i2.offset(); j2 != i2.offset()+i2.size(); ++j2)
                for (int j1 = i1.offset(); j1 != i1.offset()+i1.size(); ++j1, ptr += i0.size())
                  copy_n(ptr, i0.size(), out->element_ptr(i0.offset()-off0+dim0*(j1-off1), j2-off2+dim2*(j3-off3)));
          }
        }
      }
    }
  }
  return out;
}


template <typename DataType>
shared_ptr<Civector<DataType>> Tensor_<DataType>::civec(shared_ptr<const Determinants> det) const {
  vector<IndexRange> o = indexrange();
  assert(o.size() == 1 && o[0].size() == det->size());

  auto out = make_shared<Civector<DataType>>(det);
  for (auto& i0 : o[0].range())
    copy_n(get_block(i0).get(), i0.size(), out->data()+i0.offset());

  return out;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// explict instantiation at the end of the file
template class Tensor_<double>;
template class Tensor_<complex<double>>;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
