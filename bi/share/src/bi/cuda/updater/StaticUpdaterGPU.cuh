/**
 * @file
 *
 * @author Lawrence Murray <lawrence.murray@csiro.au>
 * $Rev$
 * $Date$
 */
#ifndef BI_CUDA_UPDATER_STATICUPDATERGPU_CUH
#define BI_CUDA_UPDATER_STATICUPDATERGPU_CUH

#include "../../state/State.hpp"
#include "../../method/misc.hpp"

namespace bi {
/**
 * Static updater, on device.
 *
 * @ingroup method_updater
 *
 * @tparam B Model type.
 * @tparam S Action type list.
 */
template<class B, class S>
class StaticUpdaterGPU {
public:
  static void update(State<B,ON_DEVICE>& s);
};
}

#include "StaticUpdaterKernel.cuh"
#include "../bind.cuh"
#include "../device.hpp"

template<class B, class S>
void bi::StaticUpdaterGPU<B,S>::update(State<B,ON_DEVICE>& s) {
  static const int N = block_size<S>::value;
  const int P = s.size();
  dim3 Db, Dg;

  Db.x = std::min(deviceIdealThreadsPerBlock(), P);
  Dg.x = (P + Db.x - 1)/Db.x;
  Db.y = 1;
  Dg.y = N;

  if (N > 0) {
    bind(s);
    kernelStaticUpdater<B,S><<<Dg,Db>>>();
    CUDA_CHECK;
    unbind(s);
  }
}

#endif