/**
 * @file
 *
 * @author Lawrence Murray <lawrence.murray@csiro.au>
 * $Rev: 1321 $
 * $Date: 2011-03-08 12:34:14 +0800 (Tue, 08 Mar 2011) $
 */
#ifndef BI_METHOD_KERNELRESAMPLER_HPP
#define BI_METHOD_KERNELRESAMPLER_HPP

#include "Resampler.hpp"

namespace bi {
/**
 * Kernel density resampler for particle filter.
 *
 * @ingroup method
 *
 * @tparam R Resampler type.
 *
 * Kernel density resampler with optional shrinkage, based on the scheme of
 * @ref Liu2001 "Liu \& West (2001)". Kernel centres are sampled using a base
 * resampler of type @p R.
 */
template<class B, class R>
class KernelResampler : public Resampler {
public:
  /**
   * Constructor.
   *
   * @param rng Random number generator.
   * @param base Base resampler.
   * @param h Bandwidth.
   * @param shrink True to apply shrinkage, false otherwise.
   */
  KernelResampler(B& m, Random& rng, R* base, const real h, const bool shrink = true);

  /**
   * @name High-level interface
   */
  //@{
  /**
   * @copydoc concept::Resampler::resample(V1&, V2&)
   */
  template<class V1, class V2, Location L>
  void resample(V1& lws, V2& as, Static<L>& theta, State<L>& s);

  /**
   * @copydoc concept::Resampler::resample(const V1&, V1&, V2&)
   */
  template<class V1, class V2, class V3, Location L>
  void resample(const V1& qlws, V2& lws, V3& as, Static<L>& theta, State<L>& s);

  /**
   * @copydoc concept::Resampler::resample(const typename V2::value_type, V1&, V2&)
   */
  template<class V1, class V2, Location L>
  void resample(const int a, V1& lws, V2& as, Static<L>& theta, State<L>& s);

  /**
   * @copydoc concept::Resampler::resample(const typename V2::value_type, const V1&, V1&, V2&)
   */
  template<class V1, class V2, class V3, Location L>
  void resample(const int a, const V1& qlws, V2& lws, V3& as, Static<L>& theta, State<L>& s);
  //@}

  /**
   * @name Low-level interface
   */
  //@{
  //@}

private:
  /**
   * Model.
   */
  B& m;

  /**
   * Random number generator.
   */
  Random& rng;

  /**
   * Base resampler.
   */
  R* base;

  /**
   * Bandwidth.
   */
  real h;

  /**
   * Shrinkage mixture proportion.
   */
  real a;

  /**
   * Shrink?
   */
  bool shrink;
};
}

template<class B, class R>
bi::KernelResampler<B,R>::KernelResampler(B& m, Random& rng, R* base,
    const real h, const bool shrink) : m(m), rng(rng), base(base), h(h),
    a(std::sqrt(1.0 - std::pow(h,2))), shrink(shrink) {
  //
}

template<class B, class R>
template<class V1, class V2, bi::Location L>
void bi::KernelResampler<B,R>::resample(V1& lws, V2& as, Static<L>& theta, State<L>& s) {
  /* pre-condition */
  assert (lws.size() == s.size());

  typedef typename State<L>::value_type T3;
  typedef typename locatable_temp_matrix<L,T3>::type M3;
  typedef typename locatable_temp_vector<L,T3>::type V3;

  const int P = s.size();
  const int ND = m.getNetSize(D_NODE);
  const int NC = m.getNetSize(C_NODE);
  const int NR = m.getNetSize(R_NODE);
  const int NP = m.getNetSize(P_NODE);
  const bool haveParameters = theta.get(P_NODE).size1() > 1;
  const int N = ND + NC + NR + (haveParameters ? NP : 0);

  M3 X(P,N), Sigma(N,N), U(N,N);
  V3 mu(N), ws(P), x(N), z(N);

  /* log relevant variables */
  log_columns(s.get(D_NODE), m.getLogs(D_NODE));
  log_columns(s.get(C_NODE), m.getLogs(C_NODE));
  log_columns(s.get(R_NODE), m.getLogs(R_NODE));
  if (haveParameters) {
    log_columns(theta.get(P_NODE), m.getLogs(P_NODE));
  }

  /* copy to one matrix */
  columns(X, 0, ND) = s.get(D_NODE);
  columns(X, ND, NC) = s.get(C_NODE);
  columns(X, ND + NC, NR) = s.get(R_NODE);
  if (haveParameters) {
    columns(X, ND + NC + NR, NP) = theta.get(P_NODE);
  }

  /* compute statistics */
  ws = lws;
  if (!ws.on_device && lws.on_device) {
    synchronize();
  }
  element_exp_unnormalised(ws.begin(), ws.end(), ws.begin());
  mean(X, ws, mu);
  cov(X, ws, mu, Sigma);
  potrf(Sigma, U, 'U');

  if (shrink) {
    /* shrink kernel centres back toward mean to preserve covariance */
    matrix_scal(a, X);
    scal(1.0 - a, mu);
    add_rows(X, mu);
  }

  /* copy back from one matrix */
  s.get(D_NODE) = columns(X, 0, ND);
  s.get(C_NODE) = columns(X, ND, NC);
  s.get(R_NODE) = columns(X, ND + NC, NR);
  if (haveParameters) {
    theta.get(P_NODE) = columns(X, ND + NC + NR, NP);
  }

  /* sample kernel centres */
  base->resample(lws, as, theta, s);

  /* add kernel noise */
//  if (haveParameters && !shrink) {
//    /* do one by one, ensuring parameter samples do not fall outside prior */
//    int p;
//    for (p = 0; p < s.size(); ++p) {
//      do {
//        rng.gaussians(z);
//        subrange(x, 0, ND) = row(s.get(D_NODE), p);
//        subrange(x, ND, NC) = row(s.get(C_NODE), p);
//        subrange(x, ND + NC, NR) = row(s.get(R_NODE), p);
//        subrange(x, ND + NC + NR, NP) = row(theta.get(P_NODE), p);
//
//        trmv(U, z, 'U');
//        axpy(h, z, x);
//
//        exp_vector(subrange(x, ND + NC + NR, NP), m.getLogs(P_NODE));
//      } while (m.template getPrior<P_NODE>().density(subrange(x, ND + NC + NR, NP)) <= 0.0);
//
//      log_vector(subrange(x, ND + NC + NR, NP), m.getLogs(P_NODE));
//
//      row(s.get(D_NODE), p) = subrange(x, 0, ND);
//      row(s.get(C_NODE), p) = subrange(x, ND, NC);
//      row(s.get(R_NODE), p) = subrange(x, ND + NC, NR);
//      row(theta.get(P_NODE), p) = subrange(x, ND + NC + NR, NP);
//    }
//  } else {
    rng.gaussians(matrix_as_vector(X));
    trmm(h, U, X, 'R', 'U');
    matrix_axpy(1.0, columns(X, 0, ND), s.get(D_NODE));
    matrix_axpy(1.0, columns(X, ND, NC), s.get(C_NODE));
    matrix_axpy(1.0, columns(X, ND + NC, NR), s.get(R_NODE));
    if (haveParameters) {
      matrix_axpy(1.0, columns(X, ND + NC + NR, NP), theta.get(P_NODE));
    }
//  }

  /* exp relevant variables */
  exp_columns(s.get(D_NODE), m.getLogs(D_NODE));
  exp_columns(s.get(C_NODE), m.getLogs(C_NODE));
  exp_columns(s.get(R_NODE), m.getLogs(R_NODE));
  if (haveParameters) {
    exp_columns(theta.get(P_NODE), m.getLogs(P_NODE));
  }

  synchronize();
}

template<class B, class R>
template<class V1, class V2, bi::Location L>
void bi::KernelResampler<B,R>::resample(const int a, V1& lws, V2& as, Static<L>& theta, State<L>& s) {
  assert(false);
}

template<class B, class R>
template<class V1, class V2, class V3, bi::Location L>
void bi::KernelResampler<B,R>::resample(const V1& qlws, V2& lws, V3& as, Static<L>& theta, State<L>& s) {
  assert(false);
}

template<class B, class R>
template<class V1, class V2, class V3, bi::Location L>
void bi::KernelResampler<B,R>::resample(const int a, const V1& qlws,
    V2& lws, V3& as, Static<L>& theta, State<L>& s) {
  assert(false);
}
#endif