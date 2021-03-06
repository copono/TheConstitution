#pragma once
#include <Eigen/Dense> 

//
// Helpers and miscelaneous functions. 
//

namespace TC
{
//------------------------------------------------------------------------------------
//Some useful conversion of elastic constants 
//Warning: I could change these names for their descriptions. For example E -> ElasticModulus
template <typename Scalar>
inline Scalar lambda(Scalar E, Scalar nu) {return E*nu / (1. + nu) / (1. - 2.*nu);}
template <typename Scalar>
inline Scalar mu(Scalar E, Scalar nu) {return E/2. / (1. + nu);}
template <typename Scalar>
inline Scalar kappa(Scalar E, Scalar nu) {return E/3. /(1. - 2.*nu);}
template <typename Scalar>
inline Scalar E(Scalar lambda, Scalar mu) {return mu * (3*lambda + 2*mu)/ (lambda + mu);}
template <typename Scalar>
inline Scalar nu(Scalar lambda, Scalar mu) {return 0.5*lambda / (lambda + mu);}

//------------------------------------------------------------------------------------
// Voigt functions and expresssions
// The exact order doesn't matter, but the first 3 components are the diagonal of the 3D
//tensor (even in 2D). The off diagonal terms can have arbitrary size.

//trace of a voigt vector. 
template <typename T>
inline typename T::Scalar vtrace(const T& vec){
  static_assert(T::RowsAtCompileTime > 3, "Bad vtrace() use. Voigt vector must always have more than 3 components.");
  return vec.template head<3>().sum();
}
//identity in voigt form
template <typename Scalar, int Length>
inline Eigen::Matrix<Scalar, Length, 1> videntity(){
  static_assert(Length > 3, "Bad videntity() use. Voigt identity must always have more than 3 components.");
  Eigen::Matrix<Scalar, Length, 1> vec;
  vec.setZero();
  vec.template head<3>() << 1,1,1;
  return vec;
}
//doubles the off diagonal terms (can help with multiplications)
template <typename T>
inline T vmult_off_diag(const T& vec, typename T::Scalar scale = 2.){
  static_assert(T::RowsAtCompileTime > 3, "Bad vmult_off_diag() use. Voigt vector must always have more than 3 components.");
  T doubled = vec;
  doubled.template tail<T::RowsAtCompileTime - 3>() *=scale;
  return doubled;
}
//norm of the tensor 
template <typename T>
inline typename T::Scalar vnorm(const T& vec){
  static_assert(T::RowsAtCompileTime > 3, "Bad vnorm() use. Voigt vector must always have more than 3 components.");
  return vmult_off_diag(vec, sqrt(2)).norm();
}
//voigt matrix-vector product must double the off diagonal terms in the vector
template <typename M, typename V>
inline V vprod(const M& matrix, const V& vector){
  static_assert(M::ColsAtCompileTime == V::RowsAtCompileTime, "Bad vprod() use. Matrix and Vector dimensions don't match.");
  return matrix * vmult_off_diag(vector);
}


//------------------------------------------------------------------------------------

//polar decomopositions:
//right polar decomposition
template <typename Scalar, int Dim>
class PolarDecompositionRU{
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  using MatrixType = Eigen::Matrix<Scalar, Dim, Dim>;
  //storage
  MatrixType R;
  MatrixType U;
  MatrixType Uinv;
  template<typename derived>
  PolarDecompositionRU(const Eigen::MatrixBase<derived>& F){
    static_assert(derived::RowsAtCompileTime == Dim && derived::ColsAtCompileTime == Dim, "Bad matrix size in polar decomposition.");
    Eigen::SelfAdjointEigenSolver<derived> eigensolver(F.transpose()*F);
    U = eigensolver.operatorSqrt();
    Uinv = eigensolver.operatorInverseSqrt();
    R = F*Uinv;
    }
};
//left polar decomposition
template <typename Scalar, int Dim>
class PolarDecompositionVR{
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  using MatrixType = Eigen::Matrix<Scalar, Dim, Dim>;
  //storage
  MatrixType R;
  MatrixType V;
  MatrixType Vinv;
  template<typename derived>
  PolarDecompositionVR(const Eigen::MatrixBase<derived>& F){
    static_assert(derived::RowsAtCompileTime == Dim && derived::ColsAtCompileTime == Dim, "Bad matrix size in polar decomposition.");
    Eigen::SelfAdjointEigenSolver<derived> eigensolver(F*F.transpose());
    V = eigensolver.operatorSqrt();
    Vinv = eigensolver.operatorInverseSqrt();
    R = Vinv*F;
    }
};

//------------------------------------------------------------------------------------
//These functions convert stress and strains from and to voigt form.
//Stress and strain have separate functions to consider the 2 multipliying the shear 
//  terms in the strain. They could easily be implemented in the same function, but I 
//  prefer the resulting function names this way.
template <typename Scalar>
Eigen::Matrix<Scalar, 6, 1> StrainToVoigt(const Eigen::Matrix<Scalar, 3, 3>& strain)
  {
  Eigen::Matrix<Scalar, 6, 1> V;
  V << strain(0,0), strain(1,1), strain(2,2), 2.*strain(0,1), 2.*strain(1,2), 2.*strain(0,2);
  return V;
  }
template <typename Scalar>
Eigen::Matrix<Scalar, 3, 1> StrainToVoigt(const Eigen::Matrix<Scalar, 2, 2>& strain)
  {
  Eigen::Matrix<Scalar, 3, 1> V;
  V << strain(0,0), strain(1,1), 2.*strain(0,1);
  return V;
  }
template <typename Scalar>
Eigen::Matrix<Scalar, 3, 3> VoigtToStrain(const Eigen::Matrix<Scalar, 6, 1>& voigt)
  {
  Eigen::Matrix<Scalar, 3, 3> S;
  S <<    voigt(0), .5*voigt(3), .5*voigt(5),
       .5*voigt(3),    voigt(1), .5*voigt(4),
       .5*voigt(5), .5*voigt(4),    voigt(2);
  return S;
  }
template <typename Scalar>
Eigen::Matrix<Scalar, 2, 2> VoigtToStrain(const Eigen::Matrix<Scalar, 3, 1>& voigt)
  {
  Eigen::Matrix<Scalar, 2, 2> S;
  S << voigt(0), .5*voigt(2), .5*voigt(2), voigt(1);
  return S;
  }
  
template <typename Scalar>
Eigen::Matrix<Scalar, 6, 1> StressToVoigt(const Eigen::Matrix<Scalar, 3, 3>& strain)
  {
  Eigen::Matrix<Scalar, 6, 1> V;
  V << strain(0,0), strain(1,1), strain(2,2), strain(0,1), strain(1,2), strain(0,2);
  return V;
  }
template <typename Scalar>
Eigen::Matrix<Scalar, 3, 1> StressToVoigt(const Eigen::Matrix<Scalar, 2, 2>& strain)
  {
  Eigen::Matrix<Scalar, 6, 1> V;
  V << strain(0,0), strain(1,1), strain(2,2), strain(0,1), strain(1,2), strain(0,2);
  return V;
  }
template <typename Scalar>
Eigen::Matrix<Scalar, 3, 3> VoigtToStress(const Eigen::Matrix<Scalar, 6, 1>& voigt)
  {
  Eigen::Matrix<Scalar, 3, 3> S;
  S << voigt(0), voigt(3), voigt(5),
       voigt(3), voigt(1), voigt(4),
       voigt(5), voigt(4), voigt(2);
  return S;
  }
template <typename Scalar>
Eigen::Matrix<Scalar, 2, 2> VoigtToStress(const Eigen::Matrix<Scalar, 3, 1>& voigt)
  {
  Eigen::Matrix<Scalar, 2, 2> S;
  S << voigt(0), voigt(2), voigt(2), voigt(1);
  return S;
  }


//------------------------------------------------------------------------------------
//Expander (to be moved to minifem) W!!
template <typename ArgType, typename ExpandType>
class expand_functor {
  enum{
    expRows = ExpandType::RowsAtCompileTime,
    expCols = ExpandType::ColsAtCompileTime
    };
  const ArgType& _arg;
  const ExpandType& _exp;
public:
  typedef Eigen::Matrix<typename ArgType::Scalar,
      ArgType::RowsAtComplieTime * ExpandType::RowsAtCompileTime,
      ArgType::ColsAtComplieTime * ExpandType::ColsAtComplieTime> MatrixType;
  expand_functor(const ArgType& arg, const ExpandType& exp): _arg(arg), _exp(exp) {}
  const typename ArgType::Scalar& operator() (Eigen::Index row, Eigen::Index col) const {
    return (_arg(row/expRows, col/expCols) * _exp(row%expRows, col%expCols));
  }
};

template <typename ArgType, typename ExpandType>
Eigen::CwiseNullaryOp<expand_functor<ArgType, ExpandType>, typename expand_functor<ArgType,ExpandType>::MatrixType>
expand_matrix(const Eigen::MatrixBase<ArgType>& arg, const Eigen::MatrixBase<ExpandType>& exp)
{
  typedef expand_functor<ArgType,ExpandType> Func;
  typedef typename Func::MatrixType MatrixType;
  return MatrixType::NullaryExpr(MatrixType::RowsAtCompileTime, MatrixType::ColsAtCompileTime, Func(arg.derived(), exp.derived()));
}


//Warning: this function is here just for testing purposes. To be removed soon!
template<typename D1, typename D2>
Eigen::Matrix<typename D1::Scalar, D1::ColsAtCompileTime, D2::ColsAtCompileTime>
  pdist2(const Eigen::MatrixBase<D1>& _X, const Eigen::MatrixBase<D2>& _Y){
  Eigen::Ref<const typename D1::PlainObject> X(_X);
  Eigen::Ref<const typename D2::PlainObject> Y(_Y);
  using matrixXt = Eigen::Matrix<typename D1::Scalar, Eigen::Dynamic, Eigen::Dynamic>;
  using retMatrix = Eigen::Matrix<typename D1::Scalar, D1::ColsAtCompileTime, D2::ColsAtCompileTime>;
  retMatrix dists = X.colwise().squaredNorm().transpose() * matrixXt::Ones(1, Y.cols()) +
    matrixXt::Ones(X.cols(), 1) * Y.colwise().squaredNorm() -
    2 * X.transpose() * Y;
  return dists;
  }

//Warning: this function is here just for testing purposes. To be removed soon!
template<typename D1, typename D2>
Eigen::Matrix<typename D1::Scalar, 3, 3>
p3(const Eigen::MatrixBase<D1>& X, const Eigen::MatrixBase<D2>& Y) {
  //Eigen::Ref<const typename D1::PlainObject> X(_X);
  //Eigen::Ref<const typename D2::PlainObject> Y(_Y);
  using Vector3t = Eigen::Matrix<typename D1::Scalar, 3, 1>;
  using retMatrix = Eigen::Matrix<typename D1::Scalar, 3, 3>;
  EIGEN_STATIC_ASSERT_SAME_MATRIX_SIZE(D1, retMatrix);
  EIGEN_STATIC_ASSERT_SAME_MATRIX_SIZE(D2, retMatrix);
  assert(X.derived().cols() == 3 && X.derived().rows() == 3 &&
         Y.derived().cols() == 3 && Y.derived().rows() == 3);
  retMatrix dists = X.colwise().squaredNorm().transpose() * Vector3t::Ones().transpose() +
    Vector3t::Ones() * Y.colwise().squaredNorm() -
    2 * X.transpose() * Y;
  return dists;
  }

}//namespace TC
