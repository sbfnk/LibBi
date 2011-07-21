/**
 * @file
 *
 * @author Lawrence Murray <lawrence.murray@csiro.au>
 * $Rev: 1757 $
 * $Date: 2011-07-14 13:08:12 +0800 (Thu, 14 Jul 2011) $
 */
#ifndef BI_METHOD_AUXILIARYCONDITIONALUNSCENTEDPARTICLEFILTER_HPP
#define BI_METHOD_AUXILIARYCONDITIONALUNSCENTEDPARTICLEFILTER_HPP

#include "ConditionalUnscentedParticleFilter.hpp"

namespace bi {
/**
 * Particle filter with unscented Kalman filter proposals conditioned on each
 * particle, using likelihoods under UKF as stage-1 weights.
 *
 * @ingroup method
 *
 * @tparam B Model type.
 * @tparam IO1 #concept::SparseInputBuffer type.
 * @tparam IO2 #concept::SparseInputBuffer type.
 * @tparam IO3 #concept::ParticleFilterBuffer type.
 * @tparam CL Cache location.
 *
 * @section Concepts
 *
 * #concept::Filter, #concept::Markable
 */
template<class B, class IO1, class IO2, class IO3, Location CL = ON_HOST,
    StaticHandling SH = STATIC_SHARED>
class AuxiliaryConditionalUnscentedParticleFilter :
    public ConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH> {
public:
  /**
   * Particle filter type.
   */
  typedef typename ConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH>::particle_filter_type particle_filter_type;

  /**
   * Kalman filter type.
   */
  typedef typename ConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH>::kalman_filter_type kalman_filter_type;

  /**
   * Constructor.
   *
   * @param m Model.
   * @param rng Random number generator.
   * @param delta Time step for d- and r-nodes.
   * @param in Forcings.
   * @param obs Observations.
   * @param out Output.
   */
  AuxiliaryConditionalUnscentedParticleFilter(B& m, Random& rng,
      const real delta = 1.0, IO1* in = NULL, IO2* obs = NULL,
      IO3* out = NULL);

  /**
   * @name High-level interface.
   *
   * An easier interface for common usage.
   */
  //@{
  /**
   * @copydoc ConditionalUnscentedParticleFilter::filter()
   */
  template<Location L, class R>
  void filter(const real T, Static<L>& theta, State<L>& s, R* resam = NULL,
      const real relEss = 1.0);

  /**
   * @copydoc ConditionalUnscentedParticleFilter::filter()
   */
  template<Location L, class R, class V1>
  void filter(const real T, const V1 x0, Static<L>& theta, State<L>& s,
      R* resam = NULL, const real relEss = 1.0);

  /**
   * @copydoc ConditionalUnscentedParticleFilter::filter()
   */
  template<Location L, class M1, class R>
  void filter(const real T, Static<L>& theta, State<L>& s, M1& xd, M1& xc,
      M1& xr, R* resam = NULL, const real relEss = 1.0);

  /**
   * @copydoc summarise_apf()
   */
  template<class T1, class V1, class V2>
  void summarise(T1* ll, V1* lls, V2* ess);
//@}

  /**
   * @name Low-level interface.
   *
   * Largely used by other features of the library or for finer control over
   * performance and behaviour.
   */
  //@{
  /**
   * @copydoc AuxiliaryParticleFilter::init
   */
  template<Location L, class V1, class V2>
  void init(Static<L>& theta, V1& lw1s, V1& lw2s, V2& as);

  /**
   * @copydoc AuxiliaryParticleFilter::resample
   */
  template<Location L, class V1, class V2, class R>
  bool resample(Static<L>& theta, State<L>& s, V1& lw1s, V1& lw2s, V2& as,
      R* resam = NULL, const real relEss = 1.0);

  /**
   * @copydoc AuxiliaryParticleFilter::resample
   */
  template<Location L, class V1, class V2, class R>
  bool resample(Static<L>& theta, State<L>& s, const int a, V1& lw1s,
      V1& lw2s, V2& as, R* resam = NULL, const real relEss = 1.0);

  /**
   * @copydoc AuxiliaryParticleFilter::output
   */
  template<Location L, class V1, class V2>
  void output(const int k, const Static<L>& theta, const State<L>& s,
      const int r, const V1& lw1s, const V1& lw2s, const V2& as);

  /**
   * @copydoc AuxiliaryParticleFilter::flush
   */
  void flush();
  //@}

protected:
  /**
   * Perform lookahead.
   *
   * @tparam V1 Vector type.
   *
   * @param[in,out] lw1s On input, current log-weights of particles, on
   * output, stage 1 log-weights.
   */
  template<class V1>
  void lookahead(V1& lw1s);

  /**
   * Cache for stage 1 log-weights.
   */
  Cache2D<real> stage1LogWeightsCache;
};

/**
 * Factory for creating AuxiliaryConditionalUnscentedParticleFilter objects.
 *
 * @ingroup method
 *
 * @tparam CL Cache location.
 *
 * @see AuxiliaryConditionalUnscentedParticleFilter
 */
template<Location CL = ON_HOST, StaticHandling SH = STATIC_SHARED>
struct AuxiliaryConditionalUnscentedParticleFilterFactory {
  /**
   * Create auxiliary conditional unscented particle filter.
   *
   * @return AuxiliaryConditionalUnscentedParticleFilter object. Caller has ownership.
   *
   * @see AuxiliaryConditionalUnscentedParticleFilter::AuxiliaryConditionalUnscentedParticleFilter()
   */
  template<class B, class IO1, class IO2, class IO3>
  static AuxiliaryConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH>* create(
      B& m, Random& rng, const real delta = 1.0, IO1* in = NULL,
      IO2* obs = NULL, IO3* out = NULL) {
    return new AuxiliaryConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH>(
        m, rng, delta, in, obs, out);
  }
};

}

#include "../math/primitive.hpp"
#include "../math/functor.hpp"

template<class B, class IO1, class IO2, class IO3, bi::Location CL,
    bi::StaticHandling SH>
bi::AuxiliaryConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH>::AuxiliaryConditionalUnscentedParticleFilter(
    B& m, Random& rng, const real delta, IO1* in, IO2* obs, IO3* out) :
    ConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH>(m, rng, delta,
        in, obs, out) {
  //
}

template<class B, class IO1, class IO2, class IO3, bi::Location CL,
    bi::StaticHandling SH>
template<bi::Location L, class R>
void bi::AuxiliaryConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH>::filter(
    const real T,Static<L>& theta, State<L>& s, R* resam,
    const real relEss) {
  /* pre-conditions */
  assert (T > this->getTime());
  assert (relEss >= 0.0 && relEss <= 1.0);

  typedef typename locatable_vector<L,real>::type V3;
  typedef typename locatable_vector<L,int>::type V4;

  Static<L> theta1(kalman_filter_type::m, theta.size());
  State<L> s1(kalman_filter_type::m);

  const int P = s.size();
  int n = 0, r = 0;
  typename locatable_temp_vector<L,real>::type lw1s(P), lw2s(P);
  typename locatable_temp_vector<L,int>::type as(P);

  init(theta, lw1s, lw2s, as);
  while (this->getTime() < T) {
    prepare(T, theta, s, theta1, s1);
    r = this->getTime() < T && resample(theta, s, lw1s, lw2s, as, resam,
        relEss);
    propose(as, lw2s);
    particle_filter_type::predict(T, theta, s);
    correct(s, lw2s);
    output(n, theta, s, r, lw1s, lw2s, as);
    ++n;
  }
  synchronize();
  term(theta);
}

template<class B, class IO1, class IO2, class IO3, bi::Location CL,
    bi::StaticHandling SH>
template<bi::Location L, class R, class V1>
void bi::AuxiliaryConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH>::filter(
    const real T, const V1 x0, Static<L>& theta, State<L>& s, R* resam,
    const real relEss) {
  assert (false);
}

template<class B, class IO1, class IO2, class IO3, bi::Location CL,
    bi::StaticHandling SH>
template<bi::Location L, class M1, class R>
void bi::AuxiliaryConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH>::filter(
    const real T, Static<L>& theta, State<L>& s, M1& xd, M1& xc, M1& xr,
    R* resam, const real relEss) {
  assert (false);
}

template<class B, class IO1, class IO2, class IO3, bi::Location CL,
    bi::StaticHandling SH>
template<class T1, class V1, class V2>
void bi::AuxiliaryConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH>::summarise(
    T1* ll, V1* lls, V2* ess) {
  summarise_apf(stage1LogWeightsCache, this->logWeightsCache, ll, lls, ess);
}

template<class B, class IO1, class IO2, class IO3, bi::Location CL,
    bi::StaticHandling SH>
template<bi::Location L, class V1, class V2>
void bi::AuxiliaryConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH>::init(
    Static<L>& theta, V1& lw1s, V1& lw2s, V2& as) {
  /* pre-condition */
  assert (lw2s.size() == as.size());

  ConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH>::init(theta, lw1s,
      as);
  bi::fill(lw2s.begin(), lw2s.end(), 0.0);
}

template<class B, class IO1, class IO2, class IO3, bi::Location CL,
    bi::StaticHandling SH>
template<bi::Location L, class V1, class V2, class R>
bool bi::AuxiliaryConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH>::resample(
    Static<L>& theta, State<L>& s, V1& lw1s, V1& lw2s, V2& as, R* resam,
    const real relEss) {
  /* pre-condition */
  assert (lw1s.size() == lw2s.size());

  bool r = false;
  this->normalise(lw2s);
  lw1s = lw2s;
  if (resam != NULL) {
    lookahead(lw1s);
    if (relEss >= 1.0 || ess(lw1s) <= s.size()*relEss) {
      resam->resample(lw1s, lw2s, as, theta, s);
      r = true;
    } else {
      lw1s = lw2s;
    }
  }
  return r;
}

template<class B, class IO1, class IO2, class IO3, bi::Location CL,
    bi::StaticHandling SH>
template<bi::Location L, class V1, class V2, class R>
bool bi::AuxiliaryConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH>::resample(
    Static<L>& theta, State<L>& s, const int a, V1& lw1s, V1& lw2s, V2& as,
    R* resam, const real relEss) {
  /* pre-condition */
  assert (lw1s.size() == lw2s.size());
  assert (a >= 0 && a < lw1s.size());

  bool r = false;
  this->normalise(lw2s);
  lw1s = lw2s;
  if (resam != NULL) {
    this->lookahead(lw1s);
    if (relEss >= 1.0 || ess(lw1s) <= s.size()*relEss) {
      resam->resample(a, lw1s, lw2s, as, theta, s);
      r = true;
    } else {
      lw1s = lw2s;
    }
  }
  return r;
}

template<class B, class IO1, class IO2, class IO3, bi::Location CL,
    bi::StaticHandling SH>
template<class V1>
void bi::AuxiliaryConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH>::lookahead(
    V1& lw1s) {
  if (this->nupdates > 0) {
    /* next obs likelihoods from UKF lookahead */
    #pragma omp parallel for
    for (int p = 0; p < this->P1; ++p) {
      BOOST_AUTO(RY1, columns(this->RY, p*this->W, this->W));

      this->ldetRY(p) = log(bi::prod(diagonal(RY1).begin(),
          diagonal(RY1).end(), 1.0));
      lw1s(p) += -0.5*dot(column(this->J1, p), column(this->J2, p));
    }

    /* stage-1 weights */
    axpy(-1.0, this->ldetRY, lw1s);
  }
}

template<class B, class IO1, class IO2, class IO3, bi::Location CL,
    bi::StaticHandling SH>
template<bi::Location L, class V1, class V2>
void bi::AuxiliaryConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH>::output(
    const int k, const Static<L>& theta, const State<L>& s, const int r,
    const V1& lw1s, const V1& lw2s, const V2& as) {
  ConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH>::output(k, theta,
      s, r, lw2s, as);
  if (particle_filter_type::haveOut) {
    stage1LogWeightsCache.put(k, lw1s);
  }
}

template<class B, class IO1, class IO2, class IO3, bi::Location CL,
    bi::StaticHandling SH>
void bi::AuxiliaryConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH>::flush() {
  ConditionalUnscentedParticleFilter<B,IO1,IO2,IO3,CL,SH>::flush();
  if (particle_filter_type::haveOut) {
    assert (stage1LogWeightsCache.isValid());
    for (int p = 0; p < stage1LogWeightsCache.size(); ++p) {
      particle_filter_type::getOutput()->writeStage1LogWeights(p,
          stage1LogWeightsCache.get(p));
    }
    stage1LogWeightsCache.clean();
  }
}

#endif
