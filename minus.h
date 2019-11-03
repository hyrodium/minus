#ifndef minus_h_
#define minus_h_
// 
// \brief MInimal problem NUmerical continuation package
// \author Ricardo Fabbri based on original code by Anton Leykin 
// \date Created: Fri Feb  8 17:42:49 EST 2019
//
// \verbatim
// Modifications
//    Leykin Feb82019: Initial sketch as simplified code from Macaulay e/NAG.*
//    Tim    Feb2019:  Chicago-specific prototype in Macaulay2
//    Fabbri Mar162019: fully templated code
// \endverbatim
//
// OPTIMIZATIONS
//  - see trifocal.key in bignotes for basic results
//  - see CMakeLists.txt and README.md

//#define NSOLS 312  /* template these */
//#define NNN   14    /* system size */
//#define NNNPLUS1 15 
//#define NNNPLUS2 16
//#define NNN2 196  /* NNN squared */
//#define NPARAMS 56 /* Number of parameters in parameter homotopy - eg, coefficients, etc, to represent NNNxNNN sys */
#include <complex>
#include <random>

// typedef std::complex<double> complex;
// might have to define eval.hxx through define
// so

template <typename F=double>
using C = std::complex<F>;

// The problem solvers that this solver template currently supports
enum problem {chicago14a, chicago6a, standard};

template <unsigned NSOLS, unsigned NNN, unsigned NPARAMS, problem P, typename F=double>
class minus_core { // fully static, not to be instantiated - just used for templating
  public: // ----------- Data structures --------------------------------------
  
  // Tracker parameters
  // Default values obtained in M2 by doing 
  // eg DEFAULT#tStep
  // or peek DEFAULT
  // 
  // We use underscore in case we want to make setters/getters with same name,
  // or members of Tracker class if more complete C++ desired
  
  struct track_settings; 
  
  enum solution_status {
    UNDETERMINED,
    PROCESSING,
    REGULAR,
    SINGULAR,
    INFINITY_FAILED,
    MIN_STEP_FAILED,
    ORIGIN_FAILED,
    INCREASE_PRECISION,
    DECREASE_PRECISION
  };
  
  struct solution
  {
    C<F> x[NNN];    // array of n coordinates
    F t;            // last value of parameter t used
    solution_status status;
    //  unsigned num_steps;  // number of steps taken along the path
    solution() : status(UNDETERMINED) { }
  };

  static const track_settings DEFAULT;
  static constexpr unsigned nnn = NNN;          // the size of the system
  static constexpr unsigned nsols = NSOLS;      // the number of solutions
  static constexpr unsigned nparams = NPARAMS;  // the number of parameters
  
  public: // ----------- Functions --------------------------------------------
  
  ///// THE MEAT /////
  static void track(const track_settings &s, const C<F> s_sols[NNN*NSOLS], 
      const C<F> params[2*NPARAMS], solution raw_solutions[NSOLS], unsigned sol_min, unsigned sol_max);
  
  // helper function: tracks all, no begin or end to specify
  static void track_all(const track_settings &s, const C<F> s_sols[NNN*NSOLS], 
      const C<F> params[2*NPARAMS], solution raw_solutions[NSOLS])
  { track(s, s_sols, params, raw_solutions, 0, NSOLS); }
  
  private: // -----------------------------------------------------------------
  static constexpr unsigned NNNPLUS1 = NNN+1;
  static constexpr unsigned NNNPLUS2 = NNN+2;
  static constexpr unsigned NNN2 = NNN*NNN;
  static void evaluate_Hxt(const C<F> * __restrict__ x /*x, t*/,    const C<F> * __restrict__ params, C<F> * __restrict__ y /*HxH*/);
  static void evaluate_HxH(const C<F> * __restrict__ x /*x and t*/, const C<F> * __restrict__ params, C<F> * __restrict__ y /*HxH*/);
};

template <unsigned NSOLS, unsigned NNN, unsigned NPARAMS, problem P, typename F>
struct minus_core<NSOLS, NNN, NPARAMS, P, F>::track_settings {
  track_settings():
    init_dt_(0.05),   // m2 tStep, t_step, raw interface code initDt
    min_dt_(1e-7),        // m2 tStepMin, raw interface code minDt
    end_zone_factor_(0.05),
    epsilon_(.000001), // m2 CorrectorTolerance (chicago.m2, track.m2), raw interface code epsilon (interface2.d, NAG.cpp:rawSwetParametersPT)
    epsilon2_(epsilon_ * epsilon_), 
    max_corr_steps_(3),  // m2 maxCorrSteps (track.m2 param of rawSetParametersPT corresp to max_corr_steps in NAG.cpp)
    dt_increase_factor_(2.),  // m2 stepIncreaseFactor
    dt_decrease_factor_(1./dt_increase_factor_),  // m2 stepDecreaseFactor not existent in DEFAULT, using what is in track.m2:77 
    num_successes_before_increase_(20), // m2 numberSuccessesBeforeIncrease
    infinity_threshold_(1e7), // m2 InfinityThreshold
    infinity_threshold2_(infinity_threshold_ * infinity_threshold_)
  { }
  
  F init_dt_;   // m2 tStep, t_step, raw interface code initDt
  F min_dt_;        // m2 tStepMin, raw interface code minDt
  F end_zone_factor_;
  F epsilon_; // m2 CorrectorTolerance (chicago.m2, track.m2), raw interface code epsilon (interface2.d, NAG.cpp:rawSwetParametersPT)
  F epsilon2_; 
  unsigned max_corr_steps_;  // m2 maxCorrSteps (track.m2 param of rawSetParametersPT corresp to max_corr_steps in NAG.cpp)
  F dt_increase_factor_;  // m2 stepIncreaseFactor
  F dt_decrease_factor_;  // m2 stepDecreaseFactor not existent in DEFAULT, using what is in track.m2:77 
  unsigned num_successes_before_increase_; // m2 numberSuccessesBeforeIncrease
  F infinity_threshold_; // m2 InfinityThreshold
  F infinity_threshold2_;
};
// Original settings from Tim: Fri Feb 22 12:00:06 -03 2019 Git 0ec3340
// o9 = MutableHashTable{AffinePatches => DynamicPatch     }
//                      Attempts => 5
//                      Bits => infinity
//                      CorrectorTolerance => .000001
//                      EndZoneFactor => .05
//                      ErrorTolerance => 1e-8
//                      Field => CC
//                      gamma => 1
//                      InfinityThreshold => 1e7
//                      Iterations => 30
//                      maxCorrSteps => 3
//                      maxNumberOfVariables => 50
//                      MultistepDegree => 3
//                      NoOutput => true
//                      Normalize => false
//                      numberSuccessesBeforeIncrease => 2
//                      Precision => 53
//                      Predictor => RungeKutta4
//                      Projectivize => false
//                      ResidualTolerance => .0001
//                      SingularConditionNumber => 100000
//                      SLP => false
//                      SLPcorrector => false
//                      SLPpredictor => false
//                      Software => M2engine
//                      stepIncreaseFactor => 2
//                      tDegree => 1
//                      Tolerance => .000001
//                      tStep => .05
//                      tStepMin => 1e-7

// The user specializes this to their problem inside problem.hxx
// Needed to create this class since functions do not always support partial
// specialization
template <problem P, typename F>
struct eval {
  static void Hxt(const C<F> * __restrict__ x /*x, t*/,    const C<F> * __restrict__ params, C<F> * __restrict__ y /*HxH*/);
  static void HxH(const C<F> * __restrict__ x /*x and t*/, const C<F> * __restrict__ params, C<F> * __restrict__ y /*HxH*/);
};

template <unsigned NSOLS, unsigned NNN, unsigned NPARAMS, problem P, typename F>
void minus_core<NSOLS, NNN, NPARAMS, P, F>::evaluate_Hxt(const C<F> * __restrict__ x /*x, t*/, const C<F> * __restrict__ params, C<F> * __restrict__ y /*HxH*/)
{
  eval<P,F>::Hxt(x, params, y);
}

template <unsigned NSOLS, unsigned NNN, unsigned NPARAMS, problem P, typename F>
void minus_core<NSOLS, NNN, NPARAMS, P, F>::evaluate_HxH(const C<F> * __restrict__ x /*x, t*/, const C<F> * __restrict__ params, C<F> * __restrict__ y /*HxH*/)
{
  eval<P,F>::HxH(x, params, y);
}

// IO shaping: not used in tracker, but only for shaping user data
// The user specializes this to their problem inside problem.hxx
// This is a base template class for zero and first-order problems (points,
// lines, tangents or lines at points).
//
// This is the default I/O class for problems involving 0 or more points, lines,
// tangents (or lines at points with orientation). For zero points, for example, 
// some useless functions will be generated by the current template, but 
// for now this won't affect code performance.
// 
// We suggest creating another class minus_io_shaping_second_order, etc, for
// problems that may not involve points, tangents nor lines, eg., problems
// involving only conics or higher-order features.
// 
// Note: Needed to create this class since functions do not always support partial
// specialization
template <problem P, unsigned NVIEWS, unsigned NPOINTS /* per view*/, unsigned NTANGENTS, /* add more as needed */ typename F>
struct minus_io_shaping {
  static constexpr unsigned nviews = NVIEWS; 
  static constexpr unsigned npoints = NPOINTS;
  // even though Chicago needs only 2 tangents, api assumes 3 tangents are given,
  // out of which two are selected by indexing. This is the most common use
  // case, where all features naturally have tangents. If strictly 2 tangents
  // are to be passed, you can leave the unused one as zeros throughout the API.
  static constexpr unsigned ntangents = NTANGENTS;
  static constexpr unsigned ncoords = 2;  // just a documented name for the number of inhomog coordinates
  static constexpr unsigned ncoords_h = 3;  // just a name for the usual number of homog coordinates in P^2
  // number of lines connecting each pair of points plus going through points
  // plus the number of free lines in the first order problem.
  // Note that tangent orientation may help ruling out solutions; this is why
  // we call it tangent, and not general lines at points. There is more
  // information at tangents which can be used as part of the model for curves.
  // The tangent orientation can be constrained by running without orientation
  // mattering at first, and then propagating these to neighboring features
  // along curves.
  static constexpr unsigned nvislines = (npoints*(npoints-1) >> 1 + ntangents + nfreelines) * nviews;  // just a name for the number of homog coordinates
  // nvislines = 15 for Chicago.
  
  // INPUT ---------------------------------------------------------------------
  static void point_tangents2params(F p[nview][npoints][ncoords], F tgt[nview][npoints][ncoords], unsigned id_tgt0, unsigned id_tgt1, C<F> * __restrict__ params/*[static 2*M::nparams]*/);
  static void gammify(C<F> * __restrict__ params/*[ chicago: M::nparams]*/);
  static void point_tangents2lines(F p[nview][npoints][ncoords], F tgt[nview][npoints][ncoords], unsigned id_tgt0, unsigned id_tgt1, F plines[nvislines][ncoords_h]);
  static void lines2params(F plines[nvislines][ncoords_h], C<F> * __restrict__ params/*[static M::nparams]*/);

  // OUTPUT --------------------------------------------------------------------
  static void solutions2cams(M::solution raw_solutions[M::NSOLS], F cameras[M::NSOLS][2][4][3], unsigned id_sols[M::NSOLS], unsigned *nsols_final);
  static void solution2cams(F rs[NNN], F cameras[2][4][3]);
};

// type alias used to hide a template parameter 
template<problem P>
using minus = minus_core<312, 14, 56, P, double>;  // TODO: set 312, 14, 56 conditional on P

template<problem P>
using minus_io = minus_io_shaping<P, 3, 3, 2, double>;  // TODO: set numbers conditional on P

template<problem P>
using minus6 = minus_core<312, 6, 45, P, double>;
// can now use minus<chicago14a>
// no need to do this:
// typedef minus<double, 312, 14, 56> minus_chicago14a;


#endif  // minus_h_
