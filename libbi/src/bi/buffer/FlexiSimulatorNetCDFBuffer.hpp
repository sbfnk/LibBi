/**
 * @file
 *
 * @author Lawrence Murray <lawrence.murray@csiro.au>
 * $Rev: 2597 $
 * $Date: 2012-05-24 22:33:24 +0800 (Thu, 24 May 2012) $
 */
#ifndef BI_BUFFER_FLEXISIMULATORNETCDFBUFFER_HPP
#define BI_BUFFER_FLEXISIMULATORNETCDFBUFFER_HPP

#include "NetCDFBuffer.hpp"
#include "NcVarBuffer.hpp"
#include "../state/State.hpp"
#include "../method/misc.hpp"

#include <vector>

namespace bi {
/**
 * NetCDF buffer for storing, reading and writing results of Simulator in
 * a flexible file format that permits the number of trajectories at each
 * time step to change.
 *
 * @ingroup io
 *
 * @section Concepts
 *
 * #concept::SimulatorBuffer
 */
class FlexiSimulatorNetCDFBuffer : public NetCDFBuffer {
public:
  /**
   * Constructor.
   *
   * @param m Model.
   * @param file NetCDF file name.
   * @param mode File open mode.
   */
  FlexiSimulatorNetCDFBuffer(const Model& m, const std::string& file,
      const FileMode mode = READ_ONLY);

  /**
   * Constructor.
   *
   * @param m Model.
   * @param T Number of time points to hold in file.
   * @param file NetCDF file name.
   * @param mode File open mode.
   */
  FlexiSimulatorNetCDFBuffer(const Model& m, const int T,
      const std::string& file, const FileMode mode = READ_ONLY);

  /**
   * Destructor.
   */
  virtual ~FlexiSimulatorNetCDFBuffer();

  /**
   * @copydoc concept::SimulatorBuffer::size2()
   */
  int size2() const;

  /**
   * @copydoc concept::SimulatorBuffer::readTime()
   */
  real readTime(const int t);

  /**
   * @copydoc concept::SimulatorBuffer::writeTime()
   */
  void writeTime(const int t, const real& x);

  /**
   * @copydoc concept::SimulatorBuffer::readStart()
   */
  int readStart(const int t);

  /**
   * @copydoc concept::SimulatorBuffer::writeStart()
   */
  void writeStart(const int t, const int& x);

  /**
   * @copydoc concept::SimulatorBuffer::readLen()
   */
  int readLen(const int t);

  /**
   * @copydoc concept::SimulatorBuffer::writeLen()
   */
  void writeLen(const int t, const int& x);

  /**
   * @copydoc concept::SimulatorBuffer::readTime()
   */
  template<class V1>
  void readTimes(const int t, const int T, V1 x);

  /**
   * @copydoc concept::SimulatorBuffer::writeTime()
   */
  template<class V1>
  void writeTimes(const int t, const int T, const V1 x);

  /**
   * @copydoc concept::SimulatorBuffer::readState()
   */
  template<class M1>
  void readState(const VarType type, const int t, M1 X);

  /**
   * @copydoc concept::SimulatorBuffer::writeState()
   */
  template<class M1>
  void writeState(const VarType type, const int t, const M1 X,
      const int p = 0);

protected:
  /**
   * Set up structure of NetCDF file.
   *
   * @param T Number of time points.
   */
  void create(const long T);

  /**
   * Map structure of existing NetCDF file.
   *
   * @param T Number of time points.
   */
  void map(const long T = -1);

  /**
   * Model.
   */
  const Model& m;

  /**
   * Time dimension.
   */
  NcDim* nrDim;

  /**
   * Model dimensions.
   */
  std::vector<NcDim*> nDims;

  /**
   * Serialised time and trajectory dimension.
   */
  NcDim* nrpDim;

  /**
   * Time variable.
   */
  NcVar* tVar;

  /**
   * Starting index into #nrpDim for each time.
   */
  NcVar* startVar;

  /**
   * Lengths along #nrpDim for each time.
   */
  NcVar* lenVar;

  /**
   * Node variables, indexed by type.
   */
  std::vector<std::vector<NcVarBuffer<real>*> > vars;
};
}

#include "../math/view.hpp"
#include "../math/temp_matrix.hpp"

inline int bi::FlexiSimulatorNetCDFBuffer::size2() const {
  return nrDim->size();
}

template<class V1>
void bi::FlexiSimulatorNetCDFBuffer::readTimes(const int t, const int T, V1 x) {
  /* pre-condition */
  assert (t >= 0 && t + T <= nrDim->size());

  typedef typename V1::value_type temp_value_type;
  typedef typename temp_host_vector<temp_value_type>::type temp_vector_type;

  BI_UNUSED NcBool ret;
  ret = tVar->set_cur(t);
  BI_ASSERT(ret, "Indexing out of bounds reading variable " << tVar->name());

  if (V1::on_device || x.inc() != 1) {
    temp_vector_type x1(x.size());
    ret = tVar->get(x1.buf(), T);
    BI_ASSERT(ret, "Inconvertible type reading variable " << tVar->name());
    x = x1;
  } else {
    ret = tVar->get(x.buf(), T);
    BI_ASSERT(ret, "Inconvertible type reading variable " << tVar->name());
  }
}

template<class V1>
void bi::FlexiSimulatorNetCDFBuffer::writeTimes(const int t, const int T,
    const V1 x) {
  /* pre-condition */
  assert (t >= 0 && t + T <= nrDim->size());

  typedef typename V1::value_type temp_value_type;
  typedef typename temp_host_vector<temp_value_type>::type temp_vector_type;

  BI_UNUSED NcBool ret;
  ret = tVar->set_cur(t);
  BI_ASSERT(ret, "Indexing out of bounds writing variable " << tVar->name());

  if (V1::on_device || x.inc() != 1) {
    temp_vector_type x1(x.size());
    x1 = x;
    synchronize(V1::on_device);
    ret = tVar->put(x1.buf(), T);
  } else {
    ret = tVar->put(x.buf(), T);
  }
  BI_ASSERT(ret, "Inconvertible type writing variable " << tVar->name());
}

template<class M1>
void bi::FlexiSimulatorNetCDFBuffer::readState(const VarType type,
    const int t, M1 X) {
  /* pre-conditions */
  assert (t >= 0 && t < nrDim->size());
  assert (X.size1() == readLen(t));

  typedef typename M1::value_type temp_value_type;
  typedef typename temp_host_matrix<temp_value_type>::type temp_matrix_type;

  Var* var;
  host_vector<long> offsets, counts;
  int start, size, id, i, j;
  BI_UNUSED NcBool ret;

  for (id = 0; id < m.getNumVars(type); ++id) {
    var = m.getVar(type, id);
    start = m.getVarStart(type, id);
    size = var->getSize();

    if (var->getIO()) {
      BOOST_AUTO(ncVar, vars[type][id]);
      BI_ERROR (ncVar != NULL, "Variable " << var->getName() <<
          " does not exist in file");

      j = 0;
      offsets.resize(ncVar->num_dims(), false);
      counts.resize(ncVar->num_dims(), false);

      for (i = 0; i < var->getNumDims(); ++i, ++j) {
        offsets[j] = 0;
        counts[j] = ncVar->get_dim(j)->size();
      }
      offsets[j] = readStart(t);;
      counts[j] = readLen(t);

      ret = ncVar->get_var()->set_cur(offsets.buf());
      BI_ASSERT(ret, "Indexing out of bounds reading variable " <<
          ncVar->name());

      if (M1::on_device || X.lead() != X.size1()) {
        temp_matrix_type X1(X.size1(), size);
        ret = ncVar->get_var()->get(X1.buf(), counts.buf());
        BI_ASSERT(ret, "Inconvertible type reading variable " <<
            ncVar->name());
        columns(X, start, size) = X1;
      } else {
        ret = ncVar->get_var()->get(columns(X, start, size).buf(),
            counts.buf());
        BI_ASSERT(ret, "Inconvertible type reading variable " <<
            ncVar->name());
      }
    }
  }
}

template<class M1>
void bi::FlexiSimulatorNetCDFBuffer::writeState(const VarType type,
    const int t, const M1 X, const int p) {
  typedef typename M1::value_type temp_value_type;
  typedef typename temp_host_matrix<temp_value_type>::type temp_matrix_type;

  Var* var;
  host_vector<long> offsets, counts;
  int start, len, offset, size, id, j;
  BI_UNUSED NcBool ret;

  /* write offset and length */
  start = (t == 0) ? 0 : readStart(t - 1) + readLen(t - 1);
  len = p + X.size1();
  writeStart(t, start);
  writeLen(t, len);

  /* write data */
  for (id = 0; id < m.getNumVars(type); ++id) {
    var = m.getVar(type, id);
    offset = m.getVarStart(type, id);
    size = var->getSize();

    if (var->getIO()) {
      BOOST_AUTO(ncVar, vars[type][id]);
      BI_ERROR (ncVar != NULL, "Variable " << var->getName() <<
          " does not exist in file");

      offsets.resize(ncVar->num_dims(), false);
      counts.resize(ncVar->num_dims(), false);

      for (j = 0; j < var->getNumDims(); ++j) {
        offsets[j] = 0;
        counts[j] = ncVar->get_dim(j)->size();
      }
      offsets[j] = start;
      counts[j] = len;

      ret = ncVar->get_var()->set_cur(offsets.buf());
      BI_ASSERT(ret, "Indexing out of bounds writing variable " <<
          ncVar->name());

      if (M1::on_device || X.lead() != X.size1()) {
        temp_matrix_type X1(X.size1(), size);
        X1 = columns(X, offset, size);
        synchronize(M1::on_device);
        ret = ncVar->get_var()->put(X1.buf(), counts.buf());
      } else {
        ret = ncVar->get_var()->put(columns(X, offset, size).buf(),
            counts.buf());
      }
      BI_ASSERT(ret, "Inconvertible type reading variable " <<
          ncVar->name());
    }
  }
}

#endif