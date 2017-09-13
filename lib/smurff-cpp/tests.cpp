#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>

#include "linop.h"
#include "chol.h"
#include "mvnormal.h"
#include "session.h"
#include "utils.h"
#include "data.h"
#include "model.h"
#include "sparsetensor.h"
#include "inv_norm_cdf.h"
#include "truncnorm.h"
#include "MatrixConfig.h"
#include "matrix_io.h"
#include "gen_random.h"

#include "ILatentPrior.h"
#include "MacauPrior.h"
#include "MacauOnePrior.h"

/* master
#include <Eigen/Dense>
#include <cmath>

#include "bpmfutils.h"
*/

using namespace smurff;

TEST_CASE( "SparseFeat/At_mul_A_bcsr", "[At_mul_A] for BinaryCSR" ) {
  int rows[9] = { 0, 3, 3, 2, 5, 4, 1, 2, 4 };
  int cols[9] = { 1, 0, 2, 1, 3, 0, 1, 3, 2 };
  SparseFeat sf(6, 4, 9, rows, cols);

  REQUIRE( sf.M.nrow == 6 );
  REQUIRE( sf.M.ncol == 4 );

  Eigen::MatrixXd AA(4, 4);
  At_mul_A(AA, sf);
  REQUIRE( AA(0,0) == 2 );
  REQUIRE( AA(1,1) == 3 );
  REQUIRE( AA(2,2) == 2 );
  REQUIRE( AA(3,3) == 2 );
  REQUIRE( AA(1,0) == 0 );
  REQUIRE( AA(2,0) == 2 );
  REQUIRE( AA(3,0) == 0 );

  REQUIRE( AA(2,1) == 0 );
  REQUIRE( AA(3,1) == 1 );

  REQUIRE( AA(3,2) == 0 );
}

TEST_CASE( "SparseFeat/At_mul_A_csr", "[At_mul_A] for CSR" ) {
  int rows[9] = { 0, 3, 3, 2, 5, 4, 1, 2, 4 };
  int cols[9] = { 1, 0, 2, 1, 3, 0, 1, 3, 2 };
  double vals[9] = { 0.6 , -0.76,  1.48,  1.19,  2.44,  1.95, -0.82,  0.06,  2.54 };
  SparseDoubleFeat sf(6, 4, 9, rows, cols, vals);

  REQUIRE( sf.M.nrow == 6 );
  REQUIRE( sf.M.ncol == 4 );

  Eigen::MatrixXd AA(4, 4);
  At_mul_A(AA, sf);
  REQUIRE( AA(0,0) == Approx(4.3801) );
  REQUIRE( AA(1,1) == Approx(2.4485) );
  REQUIRE( AA(2,2) == Approx(8.6420) );
  REQUIRE( AA(3,3) == Approx(5.9572) );

  REQUIRE( AA(1,0) == 0 );
  REQUIRE( AA(2,0) == Approx(3.8282) );
  REQUIRE( AA(3,0) == 0 );

  REQUIRE( AA(2,1) == 0 );
  REQUIRE( AA(3,1) == Approx(0.0714) );

  REQUIRE( AA(3,2) == 0 );
}

TEST_CASE( "linop/A_mul_Bx(csr)", "A_mul_Bx for CSR" ) {
  int rows[9] = { 0, 3, 3, 2, 5, 4, 1, 2, 4 };
  int cols[9] = { 1, 0, 2, 1, 3, 0, 1, 3, 2 };
  double vals[9] = { 0.6 , -0.76,  1.48,  1.19,  2.44,  1.95, -0.82,  0.06,  2.54 };
  SparseDoubleFeat sf(6, 4, 9, rows, cols, vals);
  Eigen::MatrixXd B(2, 4), X(2, 6), Xtr(2, 6);
  B << -1.38,  1.04, -0.28, -0.18,
        0.03,  0.88,  1.32, -0.31;
  Xtr << 0.624 , -0.8528,  1.2268,  0.6344, -3.4022, -0.4392,
         0.528 , -0.7216,  1.0286,  1.9308,  3.4113, -0.7564;
  A_mul_Bx<2>(X, sf.M,  B);
  REQUIRE( (X - Xtr).norm() == Approx(0) );
}

TEST_CASE( "linop/AtA_mul_Bx(csr)", "AtA_mul_Bx for CSR" ) {
  int rows[9] = { 0, 3, 3, 2, 5, 4, 1, 2, 4 };
  int cols[9] = { 1, 0, 2, 1, 3, 0, 1, 3, 2 };
  double vals[9] = { 0.6 , -0.76,  1.48,  1.19,  2.44,  1.95, -0.82,  0.06,  2.54 };
  SparseDoubleFeat sf(6, 4, 9, rows, cols, vals);
  Eigen::MatrixXd B(2, 4), tmp(2, 6), out(2, 4), outtr(2, 4), X(6, 4);
  B << -1.38,  1.04, -0.28, -0.18,
        0.03,  0.88,  1.32, -0.31;
  double reg = 0.6;

  X <<  0.  ,  0.6 ,  0.  ,  0.  ,
        0.  , -0.82,  0.  ,  0.  ,
        0.  ,  1.19,  0.  ,  0.06,
       -0.76,  0.  ,  1.48,  0.  ,
        1.95,  0.  ,  2.54,  0.  ,
        0.  ,  0.  ,  0.  ,  2.44;

  AtA_mul_Bx<2>(out, sf, reg, B, tmp);
  outtr = (X.transpose() * X * B.transpose() + reg * B.transpose()).transpose();
  REQUIRE( (out - outtr).norm() == Approx(0) );
}

TEST_CASE( "SparseFeat/compute_uhat", "compute_uhat" ) {
  int rows[9] = { 0, 3, 3, 2, 5, 4, 1, 2, 4 };
  int cols[9] = { 1, 0, 2, 1, 3, 0, 1, 3, 2 };
  SparseFeat sf(6, 4, 9, rows, cols);
  Eigen::MatrixXd beta(3, 4), uhat(3, 6), uhat_true(3, 6);

  beta << 0.56,  0.55,  0.3 , -1.78,
          1.63, -0.71,  0.8 , -0.28,
          0.47,  0.37, -1.36,  0.86;
  uhat_true <<  0.55,  0.55, -1.23,  0.86,  0.86, -1.78,
               -0.71, -0.71, -0.99,  2.43,  2.43, -0.28,
                0.37,  0.37,  1.23, -0.89, -0.89,  0.86;

  compute_uhat(uhat, sf, beta);
  for (int i = 0; i < uhat.rows(); i++) {
    for (int j = 0; j < uhat.cols(); j++) {
      REQUIRE( uhat(i,j) == Approx(uhat_true(i,j)) );
    }
  }
}

TEST_CASE( "SparseFeat/solve_blockcg", "BlockCG solver (1rhs)" ) {
  int rows[9] = { 0, 3, 3, 2, 5, 4, 1, 2, 4 };
  int cols[9] = { 1, 0, 2, 1, 3, 0, 1, 3, 2 };
  SparseFeat sf(6, 4, 9, rows, cols);
  Eigen::MatrixXd B(1, 4), X(1, 4), X_true(1, 4);

  B << 0.56,  0.55,  0.3 , -1.78;
  X_true << 0.35555556,  0.40709677, -0.16444444, -0.87483871;
  int niter = solve_blockcg(X, sf, 0.5, B, 1e-6);
  for (int i = 0; i < X.rows(); i++) {
    for (int j = 0; j < X.cols(); j++) {
      REQUIRE( X(i,j) == Approx(X_true(i,j)) );
    }
  }
  REQUIRE( niter <= 4);
}


TEST_CASE( "SparseFeat/solve_blockcg_1_0", "BlockCG solver (3rhs separately)" ) {
  int rows[9] = { 0, 3, 3, 2, 5, 4, 1, 2, 4 };
  int cols[9] = { 1, 0, 2, 1, 3, 0, 1, 3, 2 };
  SparseFeat sf(6, 4, 9, rows, cols);
  Eigen::MatrixXd B(3, 4), X(3, 4), X_true(3, 4);

  B << 0.56,  0.55,  0.3 , -1.78,
       0.34,  0.05, -1.48,  1.11,
       0.09,  0.51, -0.63,  1.59;

  X_true << 0.35555556,  0.40709677, -0.16444444, -0.87483871,
            1.69333333, -0.12709677, -1.94666667,  0.49483871,
            0.66      , -0.04064516, -0.78      ,  0.65225806;

  solve_blockcg(X, sf, 0.5, B, 1e-6, 1, 0);
  for (int i = 0; i < X.rows(); i++) {
    for (int j = 0; j < X.cols(); j++) {
      REQUIRE( X(i,j) == Approx(X_true(i,j)) );
    }
  }
}

TEST_CASE( "MatrixXd/compute_uhat", "compute_uhat for MatrixXd" ) {
  Eigen::MatrixXd beta(2, 4), feat(6, 4), uhat(2, 6), uhat_true(2, 6);
  beta << 0.56,  0.55,  0.3 , -1.78,
          1.63, -0.71,  0.8 , -0.28;
  feat <<  -0.83,  -0.26,  -0.52,  -0.27,
            0.91,  -0.48,   0.50,  -0.20,
           -0.59,   1.94,  -1.09,   0.86,
           -0.08,   0.62,  -1.10,   0.96,
            1.44,   0.89,  -0.45,   0.2,
           -1.33,  -1.42,   0.03,  -2.32;
  uhat_true <<  -0.2832,  0.7516,  -1.1212,  -1.7426,  0.8049,   2.6128,
                -1.5087,  2.2801,  -3.4519,  -1.7194,  1.2993,  -0.4861;
  compute_uhat(uhat, feat, beta);
  for (int i = 0; i < uhat.rows(); i++) {
    for (int j = 0; j < uhat.cols(); j++) {
      REQUIRE( uhat(i,j) == Approx(uhat_true(i,j)) );
    }
  }
}

TEST_CASE( "linop/solve_blockcg_dense", "BlockCG solver for dense (3rhs separately)" ) {
  int rows[9] = { 0, 3, 3, 2, 5, 4, 1, 2, 4 };
  int cols[9] = { 1, 0, 2, 1, 3, 0, 1, 3, 2 };
  Eigen::MatrixXd B(3, 4), X(3, 4), X_true(3, 4), sf(6, 4);

	sf = Eigen::MatrixXd::Zero(6, 4);
	for (int i = 0; i < 9; i++) {
		sf(rows[i], cols[i]) = 1.0;
	}

  B << 0.56,  0.55,  0.3 , -1.78,
       0.34,  0.05, -1.48,  1.11,
       0.09,  0.51, -0.63,  1.59;

  X_true << 0.35555556,  0.40709677, -0.16444444, -0.87483871,
            1.69333333, -0.12709677, -1.94666667,  0.49483871,
            0.66      , -0.04064516, -0.78      ,  0.65225806;

  solve_blockcg(X, sf, 0.5, B, 1e-6);
  for (int i = 0; i < X.rows(); i++) {
    for (int j = 0; j < X.cols(); j++) {
      REQUIRE( X(i,j) == Approx(X_true(i,j)) );
    }
  }
}

TEST_CASE( "chol/chol_solve_t", "[chol_solve_t]" ) {
  Eigen::MatrixXd m(3,3), rhs(5,3), xopt(5,3);
  m << 7, 0, 0,
       2, 5, 0,
       6, 1, 6;
  
  rhs << -1.227, -0.890,  0.293,
          0.356, -0.733, -1.201,
         -0.003, -0.091, -1.467,
          0.819,  0.725, -0.719,
         -0.485,  0.955,  1.707;
  chol_decomp(m);
  chol_solve_t(m, rhs);
  xopt << -1.67161,  0.151609,  1.69517,
           2.10217, -0.545174, -2.21148,
           1.80587, -0.34187,  -1.99339,
           1.71883, -0.180826, -1.80852,
          -2.93874,  0.746739,  3.09878;

  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 3; j++) {
      REQUIRE( rhs(i,j) == Approx(xopt(i,j)) );
    }
  }
}

TEST_CASE( "mvnormal/rgamma", "generaring random gamma variable" ) {
  init_bmrng(1234);
  double g = rgamma(100.0, 0.01);
  REQUIRE( g > 0 );
}

TEST_CASE( "latentprior/sample_lambda_beta", "sampling lambda beta from gamma distribution" ) {
  init_bmrng(1234);
  Eigen::MatrixXd beta(2, 3), Lambda_u(2, 2);
  beta << 3.0, -2.00,  0.5,
          1.0,  0.91, -0.2;
  Lambda_u << 0.5, 0.1,
              0.1, 0.3;
  auto post = smurff::posterior_lambda_beta(beta, Lambda_u, 0.01, 0.05);
  REQUIRE( post.first  == Approx(3.005) );
  REQUIRE( post.second == Approx(0.2631083888) );

  double lambda_beta = smurff::sample_lambda_beta(beta, Lambda_u, 0.01, 0.05);
  REQUIRE( lambda_beta > 0 );
}

TEST_CASE( "linop/A_mul_At_omp", "A_mul_At with OpenMP" ) {
  init_bmrng(12345);
  Eigen::MatrixXd A(2, 42);
  Eigen::MatrixXd AAt(2, 2);
  bmrandn(A);
  A_mul_At_omp(AAt, A);
  Eigen::MatrixXd AAt_true(2, 2);
  AAt_true.triangularView<Eigen::Lower>() = A * A.transpose();

  REQUIRE( AAt(0,0) == Approx(AAt_true(0,0)) );
  REQUIRE( AAt(1,1) == Approx(AAt_true(1,1)) );
  REQUIRE( AAt(1,0) == Approx(AAt_true(1,0)) );
}

TEST_CASE( "linop/A_mul_At_combo", "A_mul_At with OpenMP (returning matrix)" ) {
  init_bmrng(12345);
  Eigen::MatrixXd A(2, 42);
  Eigen::MatrixXd AAt_true(2, 2);
  bmrandn(A);
  Eigen::MatrixXd AAt = A_mul_At_combo(A);
  AAt_true = A * A.transpose();

  REQUIRE( AAt.rows() == 2);
  REQUIRE( AAt.cols() == 2);

  REQUIRE( AAt(0,0) == Approx(AAt_true(0,0)) );
  REQUIRE( AAt(1,1) == Approx(AAt_true(1,1)) );
  REQUIRE( AAt(0,1) == Approx(AAt_true(0,1)) );
  REQUIRE( AAt(1,0) == Approx(AAt_true(1,0)) );
}

TEST_CASE( "linop/A_mul_B_omp", "Fast parallel A_mul_B for small A") {
  Eigen::MatrixXd A(2, 2);
  Eigen::MatrixXd B(2, 5);
  Eigen::MatrixXd C(2, 5);
  Eigen::MatrixXd Ctr(2, 5);
  A << 3.0, -2.00,
       1.0,  0.91;
  B << 0.52, 0.19, 0.25, -0.73, -2.81,
      -0.15, 0.31,-0.40,  0.91, -0.08;
  A_mul_B_omp(0, C, 1.0, A, B);
  Ctr = A * B;
  REQUIRE( (C - Ctr).norm() == Approx(0.0) );
}

TEST_CASE( "linop/A_mul_B_omp/speed", "Speed of A_mul_B_omp") {
  Eigen::MatrixXd B(32, 1000);
  Eigen::MatrixXd X(32, 1000);
  Eigen::MatrixXd Xtr(32, 1000);
  Eigen::MatrixXd A(32, 32);
  for (int col = 0; col < B.cols(); col++) {
    for (int row = 0; row < B.rows(); row++) {
      B(row, col) = sin(row * col);
    }
  }
  for (int col = 0; col < A.cols(); col++) {
    for (int row = 0; row < A.rows(); row++) {
      A(row, col) = sin(row*(row+0.2)*col);
    }
  }
  Xtr = A * B;
  A_mul_B_omp(0, X, 1.0, A, B);
  REQUIRE( (X - Xtr).norm() == Approx(0.0) );
}

TEST_CASE( "linop/A_mul_B_add", "Fast parallel A_mul_B with adding") {
  Eigen::MatrixXd A(2, 2);
  Eigen::MatrixXd B(2, 5);
  Eigen::MatrixXd C(2, 5);
  Eigen::MatrixXd Ctr(2, 5);
  A << 3.0, -2.00,
       1.0,  0.91;
  B << 0.52, 0.19, 0.25, -0.73, -2.81,
      -0.15, 0.31,-0.40,  0.91, -0.08;
  C << 0.21, 0.70, 0.53, -0.18, -2.14,
      -0.35,-0.82,-0.27,  0.15, -0.10;
  Ctr = C;
  A_mul_B_omp(1.0, C, 1.0, A, B);
  Ctr += A * B;
  REQUIRE( (C - Ctr).norm() == Approx(0.0) );
}

TEST_CASE( "linop/At_mul_Bt/SparseFeat", "At_mul_Bt of single col for SparseFeat") {
  int rows[9] = { 0, 3, 3, 2, 5, 4, 1, 2, 4 };
  int cols[9] = { 1, 0, 2, 1, 3, 0, 1, 3, 2 };
  SparseFeat sf(6, 4, 9, rows, cols);
  Eigen::MatrixXd B(2, 6);
  Eigen::VectorXd Y(2), Y_true(2);
  B << -0.23, -2.89, -1.04, -0.52, -1.45, -1.42,
       -0.16, -0.62,  1.19,  1.12,  0.11,  0.61;
  Y_true << -4.16, 0.41;

  At_mul_Bt(Y, sf, 1, B);
  REQUIRE( Y(0) == Approx(Y_true(0)) );
  REQUIRE( Y(1) == Approx(Y_true(1)) );
}

TEST_CASE( "linop/At_mul_Bt/SparseDoubleFeat", "At_mul_Bt of single col for SparseDoubleFeat") {
  int rows[9] = { 0, 3, 3, 2, 5, 4, 1, 2, 4 };
  int cols[9] = { 1, 0, 2, 1, 3, 0, 1, 3, 2 };
  double vals[9] = { 0.6 , -0.76,  1.48,  1.19,  2.44,  1.95, -0.82,  0.06,  2.54 };
  SparseDoubleFeat sf(6, 4, 9, rows, cols, vals);

  Eigen::MatrixXd B(2, 6);
  Eigen::VectorXd Y(2), Y_true(2);
  B << -0.23, -2.89, -1.04, -0.52, -1.45, -1.42,
       -0.16, -0.62,  1.19,  1.12,  0.11,  0.61;
  Y_true << 0.9942,  1.8285;

  At_mul_Bt(Y, sf, 1, B);
  REQUIRE( Y(0) == Approx(Y_true(0)) );
  REQUIRE( Y(1) == Approx(Y_true(1)) );
}

TEST_CASE( "linop/add_Acol_mul_bt/SparseFeat", "add_Acol_mul_bt for SparseFeat") {
  int rows[9] = { 0, 3, 3, 2, 5, 4, 1, 2, 4 };
  int cols[9] = { 1, 0, 2, 1, 3, 0, 1, 3, 2 };
  SparseFeat sf(6, 4, 9, rows, cols);
  Eigen::MatrixXd Z(2, 6), Z_added(2, 6);
  Eigen::VectorXd b(2);
  Z << -0.23, -2.89, -1.04, -0.52, -1.45, -1.42,
       -0.16, -0.62,  1.19,  1.12,  0.11,  0.61;
  b << -4.16, 0.41;
  Z_added << -4.39, -7.05, -5.2 , -0.52, -1.45, -1.42,
              0.25, -0.21,  1.6 ,  1.12,  0.11,  0.61;

  add_Acol_mul_bt(Z, sf, 1, b);
  REQUIRE( (Z - Z_added).norm() == Approx(0.0) );
}

// computes Z += A[:,col] * b', where a and b are vectors
TEST_CASE( "linop/add_Acol_mul_bt/SparseDoubleFeat", "add_Acol_mul_bt for SparseDoubleFeat") {
  int rows[9] = { 0, 3, 3, 2, 5, 4, 1, 2, 4 };
  int cols[9] = { 1, 0, 2, 1, 3, 0, 1, 3, 2 };
  double vals[9] = { 0.6 , -0.76,  1.48,  1.19,  2.44,  1.95, -0.82,  0.06,  2.54 };
  SparseDoubleFeat sf(6, 4, 9, rows, cols, vals);

  Eigen::MatrixXd Z(2, 6), Z_added(2, 6);
  Eigen::VectorXd b(2);
  Z << -0.23, -2.89, -1.04, -0.52, -1.45, -1.42,
       -0.16, -0.62,  1.19,  1.12,  0.11,  0.61;
  b << -4.16, 0.41;
  Z_added << -2.726 ,  0.5212, -5.9904, -0.52  , -1.45  , -1.42,
              0.086 , -0.9562,  1.6779,  1.12  ,  0.11  ,  0.61;

  add_Acol_mul_bt(Z, sf, 1, b);
  REQUIRE( (Z - Z_added).norm() == Approx(0.0) );
}

TEST_CASE( "linop/At_mul_A_blas", "A'A with BLAS is correct") {
  Eigen::MatrixXd A(3, 2);
  Eigen::MatrixXd AtA(2, 2);
  Eigen::MatrixXd AtAtr(2, 2);
  A <<  1.7, -3.1,
        0.7,  2.9,
       -1.3,  1.5;
  AtAtr = A.transpose() * A;
  At_mul_A_blas(A, AtA.data());
  makeSymmetric(AtA);
  REQUIRE( (AtA - AtAtr).norm() == Approx(0.0) );
}

TEST_CASE( "linop/A_mul_At_blas", "AA' with BLAS is correct") {
  Eigen::MatrixXd A(3, 2);
  Eigen::MatrixXd AA(3, 3);
  Eigen::MatrixXd AAtr(3, 3);
  A <<  1.7, -3.1,
        0.7,  2.9,
       -1.3,  1.5;
  AAtr = A * A.transpose();
  A_mul_At_blas(A, AA.data());
  makeSymmetric(AA);
  REQUIRE( (AA - AAtr).norm() == Approx(0.0) );
}

TEST_CASE( "linop/A_mul_B_blas", "A_mul_B_blas is correct") {
  Eigen::MatrixXd A(3, 2);
  Eigen::MatrixXd B(2, 5);
  Eigen::MatrixXd C(3, 5);
  Eigen::MatrixXd Ctr(3, 5);
  A << 3.0, -2.00,
       1.0,  0.91,
       1.9, -1.82;
  B << 0.52, 0.19, 0.25, -0.73, -2.81,
      -0.15, 0.31,-0.40,  0.91, -0.08;
  C << 0.21, 0.70, 0.53, -0.18, -2.14,
      -0.35,-0.82,-0.27,  0.15, -0.10,
      +2.34,-0.81,-0.47,  0.31, -0.14;
  A_mul_B_blas(C, A, B);
  Ctr = A * B;
  REQUIRE( (C - Ctr).norm() == Approx(0.0) );
}

TEST_CASE( "linop/At_mul_B_blas", "At_mul_B_blas is correct") {
  Eigen::MatrixXd A(2, 3);
  Eigen::MatrixXd B(2, 5);
  Eigen::MatrixXd C(3, 5);
  Eigen::MatrixXd Ctr(3, 5);
  A << 3.0, -2.00,  1.0,
       0.91, 1.90, -1.82;
  B << 0.52, 0.19, 0.25, -0.73, -2.81,
      -0.15, 0.31,-0.40,  0.91, -0.08;
  C << 0.21, 0.70, 0.53, -0.18, -2.14,
      -0.35,-0.82,-0.27,  0.15, -0.10,
      +2.34,-0.81,-0.47,  0.31, -0.14;
  Ctr = C;
  At_mul_B_blas(C, A, B);
  Ctr = A.transpose() * B;
  REQUIRE( (C - Ctr).norm() == Approx(0.0) );
}

TEST_CASE( "linop/A_mul_Bt_blas", "A_mul_Bt_blas is correct") {
  Eigen::MatrixXd A(3, 2);
  Eigen::MatrixXd B(5, 2);
  Eigen::MatrixXd C(3, 5);
  Eigen::MatrixXd Ctr(3, 5);
  A << 3.0, -2.00,
       1.0,  0.91,
       1.9, -1.82;
  B << 0.52,  0.19,
       0.25, -0.73,
      -2.81, -0.15,
       0.31, -0.40,
       0.91, -0.08;
  C << 0.21, 0.70, 0.53, -0.18, -2.14,
      -0.35,-0.82,-0.27,  0.15, -0.10,
      +2.34,-0.81,-0.47,  0.31, -0.14;
  A_mul_Bt_blas(C, A, B);
  Ctr = A * B.transpose();
  REQUIRE( (C - Ctr).norm() == Approx(0.0) );
}
	
TEST_CASE( "utils/split_work_mpi", "Test if work splitting is correct") {
   int work3[3], work5[5];
   split_work_mpi(96, 3, work3);
   REQUIRE( work3[0] == 32 );
   REQUIRE( work3[1] == 32 );
   REQUIRE( work3[2] == 32 );

   split_work_mpi(97, 3, work3);
   REQUIRE( work3[0] == 33 );
   REQUIRE( work3[1] == 32 );
   REQUIRE( work3[2] == 32 );

   split_work_mpi(95, 3, work3);
   REQUIRE( work3[0] == 32 );
   REQUIRE( work3[1] == 32 );
   REQUIRE( work3[2] == 31 );

   split_work_mpi(80, 3, work3);
   REQUIRE( work3[0] == 28 );
   REQUIRE( work3[1] == 26 );
   REQUIRE( work3[2] == 26 );

   split_work_mpi(11, 5, work5);
   REQUIRE( work5[0] == 3 );
   REQUIRE( work5[1] == 2 );
   REQUIRE( work5[2] == 2 );
   REQUIRE( work5[3] == 2 );
   REQUIRE( work5[4] == 2 );
}

// smurff

TEST_CASE( "utils/sparseFromIJV", "Convert triplets to Eigen SparseMatrix") {
  int rows[3] = {0, 1, 2};
  int cols[3] = {2, 1, 0};
  double vals[3] = {1.0, 0.0, 2.0};
  SparseDoubleMatrix S = {3, 3, 3, rows, cols, vals};
  Eigen::SparseMatrix<double> Y = sparse_to_eigen(S);
  REQUIRE( Y.nonZeros() == 3 );
}

/* master
TEST_CASE( "bpmfutils/sparseFromIJV", "Convert triplets to Eigen SparseMatrix") {
  int rows[3] = {0, 1, 2};
  int cols[3] = {2, 1, 0};
  double vals[3] = {1.0, 0.0, 2.0};
  Eigen::SparseMatrix<double> Y;
  Eigen::MatrixXd Ytrue(3, 3), Ydense(3, 3);
  Ytrue << 0.0, 0.0, 1.0,
           0.0, 0.0, 0.0,
           2.0, 0.0, 0.0;

  Y.resize(3, 3);
  sparseFromIJV(Y, rows, cols, vals, 3);
  REQUIRE( Y.nonZeros() == 3 );

  Ydense = Eigen::MatrixXd(Y);
  REQUIRE( (Ytrue - Ydense).norm() == Approx(0.0));

  // testing idx version of sparseFromIJV
  Eigen::MatrixXi idx(3, 2);
  Eigen::VectorXd valx(3);
  idx << 0, 2,
         1, 1,
         2, 0;
  valx << 1.0, 0.0, 2.0;

  Eigen::SparseMatrix<double> Y2;
  Y2.resize(3, 3);
  sparseFromIJV(Y2, idx, valx);

  REQUIRE( Y2.nonZeros() == 3 );
  Ydense = Eigen::MatrixXd(Y2);
  REQUIRE( (Ytrue - Ydense).norm() == Approx(0.0));
}
*/

TEST_CASE( "utils/eval_rmse", "Test if prediction variance is correctly calculated") {
  int rows[1] = {0};
  int cols[1] = {0};
  double vals[1] = {4.5};
  Result p;
  Model model;
  SparseDoubleMatrix S = {1,1,1,rows, cols, vals};
  ScarceMatrixData data(sparse_to_eigen(S));
  p.set(sparse_to_eigen(S));
  data.init();
  data.setCenterMode("global");
  model.init(2, PVec(1, 1), "zero");

  auto &t = p.predictions.at(0);

  // first iteration
  model.U(0) << 1.0, 0.0;
  model.U(1) << 1.0, 0.0;
  p.update(model, data, false);
  REQUIRE(t.pred_avg == Approx(4.5 + 1.0));
  REQUIRE(t.var      == Approx(0.0));
  REQUIRE(p.rmse     == Approx(1.0));
  REQUIRE(p.rmse_avg == Approx(1.0));

  //// second iteration
  model.U(0) << 2.0, 0.0;
  model.U(1) << 1.0, 0.0;
  p.update(model, data, false);
  REQUIRE(t.pred_avg == Approx(4.5 + (1.0 + 2.0) / 2));
  REQUIRE(t.var      == Approx(0.5));
  REQUIRE(p.rmse     == 2.0);
  REQUIRE(p.rmse_avg == 1.5);

  //// third iteration
  model.U(0) << 2.0, 0.0;
  model.U(1) << 3.0, 0.0;
  p.update(model, data, false);
  REQUIRE(t.pred_avg == Approx(4.5 + (1.0 + 2.0 + 6.0) / 3));
  REQUIRE(t.var      == Approx(14.0)); // accumulated variance
  REQUIRE(p.rmse     == 6.0);
  REQUIRE(p.rmse_avg == 3.0);
}

TEST_CASE( "utils/row_mean_var", "Test if row_mean_var is correct") {
  Eigen::VectorXd mean(3), var(3), mean_tr(3), var_tr(3);
  Eigen::MatrixXd C(3, 5);
  C << 0.21, 0.70, 0.53, -0.18, -2.14,
      -0.35,-0.82,-0.27,  0.15, -0.10,
      +2.34,-0.81,-0.47,  0.31, -0.14;
  row_mean_var(mean, var, C);
  mean_tr = C.rowwise().mean();
  var_tr  = (C.colwise() - mean).cwiseAbs2().rowwise().mean();
  REQUIRE( (mean - mean_tr).norm() == Approx(0.0) );
  REQUIRE( (var  - var_tr).norm()  == Approx(0.0) );
}

/*
TEST_CASE("utils/auc","AUC ROC") {
  Eigen::VectorXd pred(20);
  Eigen::VectorXd test(20);
  test << 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 
          0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0;
  pred << 20.0, 19.0, 18.0, 17.0, 16.0, 15.0, 14.0, 13.0, 12.0, 11.0,
          10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0;
  REQUIRE ( auc(pred, test) == Approx(0.84) );
}
*/

/* master
TEST_CASE("sparsetensor/sparsemode", "SparseMode constructor") {
  Eigen::MatrixXi C(5, 3);
  C << 0, 1, 0,
       0, 0, 0,
       1, 3, 1,
       2, 3, 0,
       1, 0, 1;
  Eigen::VectorXd v(5);
  v << 0.1, 0.2, 0.3, 0.4, 0.5;

  // mode 0
  SparseMode sm0(C, v, 0, 4);

  REQUIRE( sm0.num_modes == 3);
  REQUIRE( sm0.row_ptr.size() == 5 ); 
  REQUIRE( sm0.nnz == 5 ); 
  REQUIRE( sm0.row_ptr(0) == 0 ); 
  REQUIRE( sm0.row_ptr(1) == 2 ); 
  REQUIRE( sm0.row_ptr(2) == 4 ); 
  REQUIRE( sm0.row_ptr(3) == 5 ); 
  REQUIRE( sm0.row_ptr(4) == 5 ); 
  REQUIRE( sm0.modeSize() == 4 );

  Eigen::MatrixXi I0(5, 2);
  I0 << 1, 0,
        0, 0,
        3, 1,
        0, 1,
        3, 0;
  Eigen::VectorXd v0(5);
  v0 << 0.1, 0.2, 0.3, 0.5, 0.4;
  REQUIRE( (sm0.indices - I0).norm() == 0 );
  REQUIRE( (sm0.values  - v0).norm() == 0 );

  // mode 1
  SparseMode sm1(C, v, 1, 4);
  Eigen::VectorXi ptr1(5);
  ptr1 << 0, 2, 3, 3, 5;
  I0   << 0, 0,
          1, 1,
          0, 0,
          1, 1,
          2, 0;
  v0 << 0.2, 0.5, 0.1, 0.3, 0.4;
  REQUIRE( sm1.num_modes == 3);
  REQUIRE( (sm1.row_ptr - ptr1).norm() == 0 );
  REQUIRE( (sm1.indices - I0).norm()   == 0 );
  REQUIRE( (sm1.values  - v0).norm()   == 0 );
  REQUIRE( sm1.modeSize() == 4 );
}

TEST_CASE("bpmfutils/eval_rmse_tensor", "Testing eval_rmse_tensor") {
  Eigen::MatrixXi C(5, 3);
  C << 0, 1, 0,
       0, 0, 0,
       1, 3, 1,
       1, 0, 1,
       2, 3, 0;
  Eigen::VectorXd v(5);
  v << 0.1, 0.2, 0.3, 0.4, 0.5;

  // mode 0
  SparseMode sm0(C, v, 0, 4);
  int nlatent = 5;
  double gmean = 0.9;

  std::vector< std::unique_ptr<Eigen::MatrixXd> > samples;
  Eigen::VectorXi dims(3);
  dims << 4, 5, 2;

  for (int d = 0; d < 3; d++) {
    Eigen::MatrixXd* x = new Eigen::MatrixXd(nlatent, dims(d));
    bmrandn(*x);
    samples.push_back( std::move(std::unique_ptr<Eigen::MatrixXd>(x)) );
  }

  Eigen::VectorXd pred(5);
  Eigen::VectorXd pred_var(5);
  pred.setZero();
  pred_var.setZero();

  eval_rmse_tensor(sm0, 0, pred, pred_var, samples, gmean);

  for (int i = 0; i < C.rows(); i++) {
    auto v0 = gmean + samples[0]->col(C(i, 0)).
                  cwiseProduct( samples[1]->col(C(i, 1)) ).
                  cwiseProduct( samples[2]->col(C(i, 2)) ).sum();
    REQUIRE(v0 == Approx(pred(i)));
  }
}

TEST_CASE("sparsetensor/sparsetensor", "TensorData constructor") {
  Eigen::MatrixXi C(5, 3);
  C << 0, 1, 0,
       0, 0, 0,
       1, 3, 1,
       2, 3, 0,
       1, 0, 1;
  Eigen::VectorXd v(5);
  v << 0.1, 0.2, 0.3, 0.4, 0.5;
  Eigen::VectorXi dims(3);
  dims << 4, 4, 2;

  TensorData st(3);
  st.setTrain(C, v, dims);
  REQUIRE( st.Y->size() == 3 );
  REQUIRE( (*st.Y)[0]->nonZeros() == 5 );
  REQUIRE( st.mean_value == Approx(v.mean()) );
  REQUIRE( st.N == 3 );
  REQUIRE( st.dims(0) == dims(0) );
  REQUIRE( st.dims(1) == dims(1) );
  REQUIRE( st.dims(2) == dims(2) );

  // test data
  Eigen::MatrixXi Cte(6, 3);
  Cte << 1, 1, 0,
         0, 0, 0,
         1, 3, 0,
         0, 3, 0,
         2, 3, 1,
         2, 0, 0;
  Eigen::VectorXd vte(6);
  vte << -0.1, -0.2, -0.3, -0.4, -0.5, -0.6;
  st.setTest(Cte, vte, dims);

  // fetch test data:
  Eigen::MatrixXd testData = st.getTestData();

  REQUIRE( st.getTestNonzeros() == Cte.rows() );
  REQUIRE( testData.rows() == Cte.rows() );
  REQUIRE( testData.cols() == 4 );

  Eigen::MatrixXd testDataTr(6, 4);
  testDataTr << 0, 0, 0, -0.2,
                0, 3, 0, -0.4,
                1, 1, 0, -0.1,
                1, 3, 0, -0.3,
                2, 3, 1, -0.5,
                2, 0, 0, -0.6;
  REQUIRE( (testDataTr - testData).norm() == 0);
}

TEST_CASE("sparsetensor/vectorview", "VectorView test") {
	std::vector<std::unique_ptr<int> > vec2;
	vec2.push_back( std::unique_ptr<int>(new int(0)) );
	vec2.push_back( std::unique_ptr<int>(new int(2)) );
	vec2.push_back( std::unique_ptr<int>(new int(4)) );
	vec2.push_back( std::unique_ptr<int>(new int(6)) );
	vec2.push_back( std::unique_ptr<int>(new int(8)) );
	VectorView<int> vv2(vec2, 1);
	REQUIRE( *vv2.get(0) == 0 );
	REQUIRE( *vv2.get(1) == 4 );
	REQUIRE( *vv2.get(2) == 6 );
	REQUIRE( *vv2.get(3) == 8 );
	REQUIRE( vv2.size() == 4 );
}

TEST_CASE("latentprior/sample_tensor", "Test whether sampling tensor is correct") {
  Eigen::MatrixXi C(5, 3);
  C << 0, 1, 0,
       0, 0, 0,
       1, 3, 1,
       2, 3, 0,
       1, 0, 1;
  Eigen::VectorXd v(5);
  v << 0.15, 0.23, 0.31, 0.47, 0.59;

	Eigen::VectorXd mu(3);
	Eigen::MatrixXd Lambda(3, 3);
	mu << 0.03, -0.08, 0.12;
	Lambda << 1.2, 0.11, 0.17,
				    0.11, 1.4, 0.08,
						0.17, 0.08, 1.7;

	double mvalue = 0.2;
	double alpha  = 7.5;
  int nlatent = 3;

  std::vector< std::unique_ptr<Eigen::MatrixXd> > samples;
	std::vector< std::unique_ptr<SparseMode> > sparseModes;

  Eigen::VectorXi dims(3);
  dims << 4, 5, 2;
  TensorData st(3);
  st.setTrain(C, v, dims);

  for (int d = 0; d < 3; d++) {
    Eigen::MatrixXd* x = new Eigen::MatrixXd(nlatent, dims(d));
    bmrandn(*x);
    samples.push_back( std::move(std::unique_ptr<Eigen::MatrixXd>(x)) );

		SparseMode* sm  = new SparseMode(C, v, d, dims(d));
		sparseModes.push_back( std::move(std::unique_ptr<SparseMode>(sm)) );
  }

	VectorView<Eigen::MatrixXd> vv0(samples, 0);
  sample_latent_tensor(samples[0], 0, sparseModes[0], vv0, mvalue, alpha, mu, Lambda);
}

TEST_CASE("macauoneprior/sample_tensor_uni", "Testing sampling tensor univariate") {
  int rows[9] = { 0, 3, 3, 2, 5, 4, 1, 2, 4 };
  int cols[9] = { 1, 0, 2, 1, 3, 0, 1, 3, 2 };
  SparseFeat* sf = new SparseFeat(6, 4, 9, rows, cols);
  auto sfptr = std::unique_ptr<SparseFeat>(sf);

  Eigen::MatrixXi C(5, 3);
  C << 0, 1, 0,
       0, 0, 0,
       1, 3, 1,
       2, 3, 0,
       1, 0, 1;
  Eigen::VectorXd v(5);
  v << 0.15, 0.23, 0.31, 0.47, 0.59;

	Eigen::VectorXd mu(3);
	Eigen::MatrixXd Lambda(3, 3);
	mu << 0.03, -0.08, 0.12;
	Lambda << 1.2, 0.11, 0.17,
				    0.11, 1.4, 0.08,
						0.17, 0.08, 1.7;

	double mvalue = 0.2;
	double alpha  = 7.5;
  int nlatent = 3;

  MacauOnePrior<SparseFeat> prior(nlatent, sfptr);

  std::vector< std::unique_ptr<Eigen::MatrixXd> > samples;

  Eigen::VectorXi dims(3);
  dims << 6, 5, 2;
  TensorData st(3);
  st.setTrain(C, v, dims);

  for (int d = 0; d < 3; d++) {
    Eigen::MatrixXd* x = new Eigen::MatrixXd(nlatent, dims(d));
    bmrandn(*x);
    samples.push_back( std::move(std::unique_ptr<Eigen::MatrixXd>(x)) );
  }

  prior.sample_latents(alpha, st, samples, 0, nlatent);
}

TEST_CASE("macauprior/make_dense_prior", "Making MacauPrior with MatrixXd") {
 	double x[6] = {0.1, 0.4, -0.7, 0.3, 0.11, 0.23};

	// ColMajor case
  auto prior = make_dense_prior(3, x, 3, 2, true, true);
  Eigen::MatrixXd Ftrue(3, 2);
  Ftrue <<  0.1, 0.3,
						0.4, 0.11,
					 -0.7, 0.23;
  REQUIRE( (*(prior->F) - Ftrue).norm() == Approx(0) );
	Eigen::MatrixXd tmp = Eigen::MatrixXd::Zero(2, 2);
	tmp.triangularView<Eigen::Lower>()  = prior->FtF;
	tmp.triangularView<Eigen::Lower>() -= Ftrue.transpose() * Ftrue;
  REQUIRE( tmp.norm() == Approx(0) );

	// RowMajor case
  auto prior2 = make_dense_prior(3, x, 3, 2, false, true);
	Eigen::MatrixXd Ftrue2(3, 2);
	Ftrue2 << 0.1,  0.4,
				   -0.7,  0.3,
					  0.11, 0.23;
  REQUIRE( (*(prior2->F) - Ftrue2).norm() == Approx(0) );
	Eigen::MatrixXd tmp2 = Eigen::MatrixXd::Zero(2, 2);
	tmp2.triangularView<Eigen::Lower>()  = prior2->FtF;
	tmp2.triangularView<Eigen::Lower>() -= Ftrue2.transpose() * Ftrue2;
  REQUIRE( tmp2.norm() == Approx(0) );
}
*/

TEST_CASE("inv_norm_cdf/inv_norm_cdf", "Inverse normal CDF") {
	REQUIRE( inv_norm_cdf(0.0)  == -1.0 / 0.0 );
	REQUIRE( inv_norm_cdf(0.5)  == Approx(0) );
	REQUIRE( inv_norm_cdf(0.9)  == Approx(1.2815515655446004) );
	REQUIRE( inv_norm_cdf(0.01) == Approx(-2.3263478740408408) );
}

TEST_CASE("truncnorm/norm_cdf", "Normal CDF") {
	REQUIRE( norm_cdf(0.0)  == Approx(0.5));
	REQUIRE( norm_cdf(-1.0) == Approx(0.15865525393145707) );
	REQUIRE( norm_cdf(-3.0) == Approx(0.0013498980316300933) );
	REQUIRE( norm_cdf(4.0)  == Approx(0.99996832875816688) );
}

TEST_CASE( "truncnorm/rand_truncnorm", "generaring random truncnorm variable" ) {
  init_bmrng(1234);
  for (int i = 0; i < 10; i++) {
    REQUIRE( rand_truncnorm(2.0) >= 2.0 );
    REQUIRE( rand_truncnorm(3.0) >= 3.0 );
    REQUIRE( rand_truncnorm(5.0) >= 5.0 );
    REQUIRE( rand_truncnorm(50.0) >= 50.0 );
    REQUIRE( rand_truncnorm(30, 2.0, 50.0) >= 50.0 );
  }
}

TEST_CASE("Comparing results of creating matrix from updated matrix config and directly reading matrix from file")
{
   // There should be a matrix file beside test executable
   const std::string matrixFilePath = "./chembl-IC50-346targets.mm";
   REQUIRE(std::ifstream(matrixFilePath));

   // Reading sparse matrix using an updated matrix config that is based on a new tensor config
   MatrixConfig matrix1Config = read_sparse(matrixFilePath);
   Eigen::SparseMatrix<double> matrix1 = sparse_to_eigen(matrix1Config);

   // Reading sparse matrix directly from file bypassing updated matrix config
   Eigen::SparseMatrix<double> matrix2;
   read_sparse(matrixFilePath, matrix2);

   // "isApprox" is Eigen matrix method that can be used to compare two matrices
   // https://codeyarns.com/2016/02/16/how-to-compare-eigen-matrices-for-equality/
   REQUIRE(matrix1.isApprox(matrix2));
}

TEST_CASE("TensorConfig(int* columns, int nmodes, double* values, int nnz, int* dims)")
{
   std::vector<int> actualMatrixConfigColumns = { 0, 0, 0, 1, 1, 1, 2, 2, 2,
                                                  0, 1, 2, 0, 1, 2, 0, 1, 2 
                                                };
   std::vector<double> actualMatrixConfigValues = { 1, 4, 7, 2, 5, 8, 3, 6, 9 };

   MatrixConfig actualMatrixConfig(3, 3, 9, actualMatrixConfigColumns.data(), actualMatrixConfigValues.data());
   Eigen::MatrixXd actualMatrix = dense_to_eigen(actualMatrixConfig);

   Eigen::MatrixXd expectedMatrix(3, 3);
   expectedMatrix(0, 0) = 1; expectedMatrix(0, 1) = 2; expectedMatrix(0, 2) = 3;
   expectedMatrix(1, 0) = 4; expectedMatrix(1, 1) = 5; expectedMatrix(1, 2) = 6;
   expectedMatrix(2, 0) = 7; expectedMatrix(2, 1) = 8; expectedMatrix(2, 2) = 9;

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("MatrixConfig(int nrow, int ncol, double *values)")
{
   Eigen::MatrixXd matrix1(3, 3);
   matrix1(0, 0) = 1; matrix1(0, 1) = 2; matrix1(0, 2) = 3;
   matrix1(1, 0) = 4; matrix1(1, 1) = 5; matrix1(1, 2) = 6;
   matrix1(2, 0) = 7; matrix1(2, 1) = 8; matrix1(2, 2) = 9;

   double matrix2ConfigValues[9] = { 1, 4, 7, 2, 5, 8, 3, 6, 9 };
   MatrixConfig matrix2Config(3, 3, matrix2ConfigValues);
   Eigen::MatrixXd matrix2 = dense_to_eigen(matrix2Config);

   // std::cout << matrix1 << std::endl << std::endl;
   // std::cout << matrix2 << std::endl;

   REQUIRE(matrix1.isApprox(matrix2));
}

TEST_CASE("MatrixConfig(int nrow, int ncol, int nnz, int *rows, int *cols, double *values)")
{
   Eigen::SparseMatrix<double> matrix1(3, 3);
   std::vector<Eigen::Triplet<double> > matrix1Triplets;
   matrix1Triplets.push_back(Eigen::Triplet<double>(0, 0, 1));
   matrix1Triplets.push_back(Eigen::Triplet<double>(0, 1, 2));
   matrix1Triplets.push_back(Eigen::Triplet<double>(0, 2, 3));
   matrix1Triplets.push_back(Eigen::Triplet<double>(2, 0, 7));
   matrix1Triplets.push_back(Eigen::Triplet<double>(2, 1, 8));
   matrix1Triplets.push_back(Eigen::Triplet<double>(2, 2, 9));
   matrix1.setFromTriplets(matrix1Triplets.begin(), matrix1Triplets.end());

   int matrix2ConfigRows[6]      = { 0, 0, 0, 2, 2, 2 };
   int matrix2ConfigCols[6]      = { 0, 1, 2, 0, 1, 2 };
   double matrix2ConfigValues[6] = { 1, 2, 3, 7, 8, 9 };
   MatrixConfig matrix2Config(3, 3, 6, matrix2ConfigRows, matrix2ConfigCols, matrix2ConfigValues);
   Eigen::SparseMatrix<double> matrix2 = sparse_to_eigen(matrix2Config);

   REQUIRE(matrix1.isApprox(matrix2));
}

TEST_CASE("MatrixConfig(int nrow, int ncol, int nnz, int *rows, int *cols)")
{
   Eigen::SparseMatrix<double> matrix1(3, 3);
   std::vector<Eigen::Triplet<double> > matrix1Triplets;
   matrix1Triplets.push_back(Eigen::Triplet<double>(0, 0, 1));
   matrix1Triplets.push_back(Eigen::Triplet<double>(0, 1, 1));
   matrix1Triplets.push_back(Eigen::Triplet<double>(0, 2, 1));
   matrix1Triplets.push_back(Eigen::Triplet<double>(2, 0, 1));
   matrix1Triplets.push_back(Eigen::Triplet<double>(2, 1, 1));
   matrix1Triplets.push_back(Eigen::Triplet<double>(2, 2, 1));
   matrix1.setFromTriplets(matrix1Triplets.begin(), matrix1Triplets.end());

   int matrix2ConfigRows[6] = { 0, 0, 0, 2, 2, 2 };
   int matrix2ConfigCols[6] = { 0, 1, 2, 0, 1, 2 };
   MatrixConfig matrix2Config(3, 3, 6, matrix2ConfigRows, matrix2ConfigCols);
   Eigen::SparseMatrix<double> matrix2 = sparse_to_eigen(matrix2Config);

   REQUIRE(matrix1.isApprox(matrix2));   
}

TEST_CASE("gen_random/extract(SparseMatrixD)", "[!hide]")
{
   const double s = 0.3;
   const int seed = 1234;

   SparseMatrixD inMatrix1(3, 3);
   std::vector<Eigen::Triplet<double> > inMatrix1Triplets;
   inMatrix1Triplets.push_back(Eigen::Triplet<double>(0, 0, 1));
   inMatrix1Triplets.push_back(Eigen::Triplet<double>(0, 1, 2));
   inMatrix1Triplets.push_back(Eigen::Triplet<double>(0, 2, 3));
   inMatrix1Triplets.push_back(Eigen::Triplet<double>(1, 0, 4));
   inMatrix1Triplets.push_back(Eigen::Triplet<double>(1, 1, 5));
   inMatrix1Triplets.push_back(Eigen::Triplet<double>(1, 2, 6));
   inMatrix1Triplets.push_back(Eigen::Triplet<double>(2, 0, 7));
   inMatrix1Triplets.push_back(Eigen::Triplet<double>(2, 1, 8));
   inMatrix1Triplets.push_back(Eigen::Triplet<double>(2, 2, 9));
   inMatrix1.setFromTriplets(inMatrix1Triplets.begin(), inMatrix1Triplets.end());

   // std::cout << inMatrix1 << std::endl << std::endl;

   SparseMatrixD outMatrix1 = extract(inMatrix1, s, seed);

   // std::cout << inMatrix1 << std::endl << std::endl;
   // std::cout << outMatrix1 << std::endl << std::endl;
}

TEST_CASE("gen_random/extract(MatrixConfig, remove = true). Sparse matrix input", "[!hide]")
{
   const double s = 0.3;
   const bool remove = true;
   const int seed = 1234;

   double actualInMatrixConfigValues[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
   MatrixConfig actualInMatrix2Config(3, 3, actualInMatrixConfigValues);
   MatrixConfig actualOutMatrix2Config = extract(actualInMatrix2Config, s, remove, seed);

   SparseMatrixD actualInMatrix = sparse_to_eigen(actualInMatrix2Config);
   SparseMatrixD actualOutMatrix = sparse_to_eigen(actualOutMatrix2Config);

   SparseMatrixD expectedInMatrix(3, 3);
   std::vector<Eigen::Triplet<double> > expectedInMatrixTriplets;
   expectedInMatrixTriplets.push_back(Eigen::Triplet<double>(0, 0, 1));
   expectedInMatrixTriplets.push_back(Eigen::Triplet<double>(0, 1, 2));
   expectedInMatrixTriplets.push_back(Eigen::Triplet<double>(1, 0, 4));
   expectedInMatrixTriplets.push_back(Eigen::Triplet<double>(1, 1, 5));
   expectedInMatrixTriplets.push_back(Eigen::Triplet<double>(1, 2, 6));
   expectedInMatrixTriplets.push_back(Eigen::Triplet<double>(2, 0, 7));
   expectedInMatrixTriplets.push_back(Eigen::Triplet<double>(2, 1, 8));
   expectedInMatrixTriplets.push_back(Eigen::Triplet<double>(2, 2, 9));
   expectedInMatrix.setFromTriplets(expectedInMatrixTriplets.begin(), expectedInMatrixTriplets.end());

   SparseMatrixD expectedOutMatrix(3, 3);
   std::vector<Eigen::Triplet<double> > expectedOutMatrixTriplets;
   expectedOutMatrixTriplets.push_back(Eigen::Triplet<double>(0, 2, 3));
   expectedOutMatrix.setFromTriplets(expectedOutMatrixTriplets.begin(), expectedOutMatrixTriplets.end());

   // std::cout << expectedInMatrix << std::endl << std::endl;
   // std::cout << expectedOutMatrix << std::endl << std::endl;

   // std::cout << actualInMatrix << std::endl << std::endl;
   // std::cout << actualOutMatrix << std::endl << std::endl;

   REQUIRE(actualInMatrix.isApprox(expectedInMatrix));
   REQUIRE(actualOutMatrix.isApprox(expectedOutMatrix));
}

TEST_CASE("gen_random/extract(MatrixConfig, remove = false). Dense matrix input", "[!hide]")
{
   const double s = 0.3;
   const bool remove = false;
   const int seed = 1234;

   double actualInMatrixConfigValues[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
   MatrixConfig actualInMatrixConfig(3, 3, actualInMatrixConfigValues);
   MatrixConfig actualOutMatrixConfig = extract(actualInMatrixConfig, s, remove, seed);

   SparseMatrixD actualInMatrix = sparse_to_eigen(actualInMatrixConfig);
   SparseMatrixD actualOutMatrix = sparse_to_eigen(actualOutMatrixConfig);

   Eigen::MatrixXd expectedInMatrix(3, 3);
   expectedInMatrix(0, 0) = 1; expectedInMatrix(0, 1) = 2; expectedInMatrix(0, 2) = 3;
   expectedInMatrix(1, 0) = 4; expectedInMatrix(1, 1) = 5; expectedInMatrix(1, 2) = 6;
   expectedInMatrix(2, 0) = 7; expectedInMatrix(2, 1) = 8; expectedInMatrix(2, 2) = 9;

   SparseMatrixD expectedOutMatrix(3, 3);
   std::vector<Eigen::Triplet<double> > expectedOutMatrixTriplets;
   expectedOutMatrixTriplets.push_back(Eigen::Triplet<double>(0, 2, 3));
   expectedOutMatrix.setFromTriplets(expectedOutMatrixTriplets.begin(), expectedOutMatrixTriplets.end());

   REQUIRE(actualInMatrix.isApprox(expectedInMatrix));
   REQUIRE(actualOutMatrix.isApprox(expectedOutMatrix));
}

TEST_CASE("matrix_io/read_csv(std::istream& in)")
{
   std::stringstream ss;
   ss << "3" << std::endl << "3" << std::endl;
   ss << "1, 2, 3" << std::endl;
   ss << "4,5,6" << std::endl;
   ss << "7, 8, 9" << std::endl;
   MatrixConfig actualMatrixConfig = read_csv(ss);
   Eigen::MatrixXd actualMatrix = dense_to_eigen(actualMatrixConfig);

   Eigen::MatrixXd expectedMatrix(3, 3);
   expectedMatrix(0, 0) = 1; expectedMatrix(0, 1) = 2; expectedMatrix(0, 2) = 3;
   expectedMatrix(1, 0) = 4; expectedMatrix(1, 1) = 5; expectedMatrix(1, 2) = 6;
   expectedMatrix(2, 0) = 7; expectedMatrix(2, 1) = 8; expectedMatrix(2, 2) = 9;

   // std::cout << actualMatrix << std::endl << std::endl;
   // std::cout << expectedMatrix << std::endl;

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matix_io/read_ddm(std::istream& in)")
{
   long nrow = 3;
   long ncol = 3;
   std::vector<double> values = { 1, 4, 7, 2, 5, 8, 3, 6, 9 };

   std::stringstream ss;
   ss.write(reinterpret_cast<char*>(&nrow), sizeof(long));
   ss.write(reinterpret_cast<char*>(&ncol), sizeof(long));
   ss.write(reinterpret_cast<char*>(values.data()), values.size() * sizeof(double));

   MatrixConfig actualMatrixConfig = read_ddm(ss);
   Eigen::MatrixXd actualMatrix = dense_to_eigen(actualMatrixConfig);

   Eigen::MatrixXd expectedMatrix(3, 3);
   expectedMatrix(0, 0) = 1; expectedMatrix(0, 1) = 2; expectedMatrix(0, 2) = 3;
   expectedMatrix(1, 0) = 4; expectedMatrix(1, 1) = 5; expectedMatrix(1, 2) = 6;
   expectedMatrix(2, 0) = 7; expectedMatrix(2, 1) = 8; expectedMatrix(2, 2) = 9;

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/read_dense(std::istream& in, DenseMatrixType denseMatrixType). DenseMatrixType::csv")
{
   std::stringstream ss;
   ss << "3" << std::endl << "3" << std::endl;
   ss << "1, 2, 3" << std::endl;
   ss << "4,5,6" << std::endl;
   ss << "7, 8, 9" << std::endl;
   MatrixConfig actualMatrixConfig = read_dense(ss, DenseMatrixType::csv);
   Eigen::MatrixXd actualMatrix = dense_to_eigen(actualMatrixConfig);

   Eigen::MatrixXd expectedMatrix(3, 3);
   expectedMatrix(0, 0) = 1; expectedMatrix(0, 1) = 2; expectedMatrix(0, 2) = 3;
   expectedMatrix(1, 0) = 4; expectedMatrix(1, 1) = 5; expectedMatrix(1, 2) = 6;
   expectedMatrix(2, 0) = 7; expectedMatrix(2, 1) = 8; expectedMatrix(2, 2) = 9;

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/read_dense(std::istream& in, DenseMatrixType denseMatrixType). DenseMatrixType::ddm")
{
   long nrow = 3;
   long ncol = 3;
   std::vector<double> values = { 1, 4, 7, 2, 5, 8, 3, 6, 9 };

   std::stringstream ss;
   ss.write(reinterpret_cast<char*>(&nrow), sizeof(long));
   ss.write(reinterpret_cast<char*>(&ncol), sizeof(long));
   ss.write(reinterpret_cast<char*>(values.data()), values.size() * sizeof(double));

   MatrixConfig actualMatrixConfig = read_dense(ss, DenseMatrixType::ddm);
   Eigen::MatrixXd actualMatrix = dense_to_eigen(actualMatrixConfig);

   Eigen::MatrixXd expectedMatrix(3, 3);
   expectedMatrix(0, 0) = 1; expectedMatrix(0, 1) = 2; expectedMatrix(0, 2) = 3;
   expectedMatrix(1, 0) = 4; expectedMatrix(1, 1) = 5; expectedMatrix(1, 2) = 6;
   expectedMatrix(2, 0) = 7; expectedMatrix(2, 1) = 8; expectedMatrix(2, 2) = 9;

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/read_mtx(std::istream& in)")
{
   std::stringstream ss;
   ss << 3 << '\t' << 3 << '\t' << 9 << std::endl;

   ss << 1 << '\t' << 1 << '\t' << 1 << std::endl;
   ss << 1 << '\t' << 2 << '\t' << 2 << std::endl;
   ss << 1 << '\t' << 3 << '\t' << 3 << std::endl;

   ss << 2 << '\t' << 1 << '\t' << 4 << std::endl;
   ss << 2 << '\t' << 2 << '\t' << 5 << std::endl;
   ss << 2 << '\t' << 3 << '\t' << 6 << std::endl;
   
   ss << 3 << '\t' << 1 << '\t' << 7 << std::endl;
   ss << 3 << '\t' << 2 << '\t' << 8 << std::endl;
   ss << 3 << '\t' << 3 << '\t' << 9 << std::endl;

   MatrixConfig actualMatrixConfig = read_mtx(ss);
   SparseMatrixD actualMatrix = sparse_to_eigen(actualMatrixConfig);

   Eigen::MatrixXd expectedMatrix(3, 3);
   expectedMatrix(0, 0) = 1; expectedMatrix(0, 1) = 2; expectedMatrix(0, 2) = 3;
   expectedMatrix(1, 0) = 4; expectedMatrix(1, 1) = 5; expectedMatrix(1, 2) = 6;
   expectedMatrix(2, 0) = 7; expectedMatrix(2, 1) = 8; expectedMatrix(2, 2) = 9;

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/writeToCSVstream")
{
   std::stringstream actualMatrixOutputStream;
   Eigen::MatrixXd actualMatrix(3, 3);
   actualMatrix(0, 0) = 1; actualMatrix(0, 1) = 2; actualMatrix(0, 2) = 3;
   actualMatrix(1, 0) = 4; actualMatrix(1, 1) = 5; actualMatrix(1, 2) = 6;
   actualMatrix(2, 0) = 7; actualMatrix(2, 1) = 8; actualMatrix(2, 2) = 9;
   writeToCSVstream(actualMatrixOutputStream, actualMatrix);

   std::stringstream expectedMatrixOutputStream;
   expectedMatrixOutputStream << 3 << std::endl;
   expectedMatrixOutputStream << 3 << std::endl;
   expectedMatrixOutputStream << "1,2,3" << std::endl;
   expectedMatrixOutputStream << "4,5,6" << std::endl;
   expectedMatrixOutputStream << "7,8,9" << std::endl;
   
   REQUIRE(actualMatrixOutputStream.str() == expectedMatrixOutputStream.str());
}

TEST_CASE("matrix_io/readFromCSVstream")
{
   std::stringstream actualMatrixInputStream;
   actualMatrixInputStream << 3 << std::endl;
   actualMatrixInputStream << 3 << std::endl;
   actualMatrixInputStream << "1,2,3" << std::endl;
   actualMatrixInputStream << "4,5,6" << std::endl;
   actualMatrixInputStream << "7,8,9" << std::endl;
   Eigen::MatrixXd actualMatrix;
   readFromCSVstream(actualMatrixInputStream, actualMatrix);

   Eigen::MatrixXd expectedMatrix(3, 3);
   expectedMatrix(0, 0) = 1; expectedMatrix(0, 1) = 2; expectedMatrix(0, 2) = 3;
   expectedMatrix(1, 0) = 4; expectedMatrix(1, 1) = 5; expectedMatrix(1, 2) = 6;
   expectedMatrix(2, 0) = 7; expectedMatrix(2, 1) = 8; expectedMatrix(2, 2) = 9;

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/write_ddm(std::ostream& out, const Eigen::MatrixXd& matrix)")
{
   std::stringstream actualMatrixOutputStream;
   Eigen::MatrixXd actualMatrix(3, 3);
   actualMatrix(0, 0) = 1; actualMatrix(0, 1) = 2; actualMatrix(0, 2) = 3;
   actualMatrix(1, 0) = 4; actualMatrix(1, 1) = 5; actualMatrix(1, 2) = 6;
   actualMatrix(2, 0) = 7; actualMatrix(2, 1) = 8; actualMatrix(2, 2) = 9;
   write_ddm(actualMatrixOutputStream, actualMatrix);

   long expectedMatrixNRow = 3;
   long expectedMatrixNCol = 3;
   double expectedMatrixValues[9] = { 1, 4, 7, 2, 5, 8, 3, 6, 9 };
   std::stringstream expectedMatrixOutputStream;
   expectedMatrixOutputStream.write(reinterpret_cast<char*>(&expectedMatrixNRow), sizeof(long));
   expectedMatrixOutputStream.write(reinterpret_cast<char*>(&expectedMatrixNCol), sizeof(long));
   expectedMatrixOutputStream.write(reinterpret_cast<char*>(expectedMatrixValues), 9 * sizeof(double));
   
   REQUIRE(actualMatrixOutputStream.str() == expectedMatrixOutputStream.str());
}

TEST_CASE("matrix_io/read_ddm(std::istream& in, Eigen::MatrixXd& matrix)")
{
   long actualMatrixNRow = 3;
   long actualMatrixNCol = 3;
   double actualMatrixValues[9] = { 1, 4, 7, 2, 5, 8, 3, 6, 9 };
   std::stringstream actualMatrixInputStream;
   actualMatrixInputStream.write(reinterpret_cast<char*>(&actualMatrixNRow), sizeof(long));
   actualMatrixInputStream.write(reinterpret_cast<char*>(&actualMatrixNCol), sizeof(long));
   actualMatrixInputStream.write(reinterpret_cast<char*>(actualMatrixValues), 9 * sizeof(double));
   Eigen::MatrixXd actualMatrix;
   read_ddm(actualMatrixInputStream, actualMatrix);

   Eigen::MatrixXd expectedMatrix(3, 3);
   expectedMatrix(0, 0) = 1; expectedMatrix(0, 1) = 2; expectedMatrix(0, 2) = 3;
   expectedMatrix(1, 0) = 4; expectedMatrix(1, 1) = 5; expectedMatrix(1, 2) = 6;
   expectedMatrix(2, 0) = 7; expectedMatrix(2, 1) = 8; expectedMatrix(2, 2) = 9;

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/read_dense(std::istream& in, DenseMatrixType denseMatrixType, Eigen::MatrixXd& X). DenseMatrixType::ddm")
{
   long actualMatrixNRow = 3;
   long actualMatrixNCol = 3;
   std::vector<double> actualMatrixValues = { 1, 4, 7, 2, 5, 8, 3, 6, 9 };

   std::stringstream actualMatrixInputStream;
   actualMatrixInputStream.write(reinterpret_cast<char*>(&actualMatrixNRow), sizeof(long));
   actualMatrixInputStream.write(reinterpret_cast<char*>(&actualMatrixNCol), sizeof(long));
   actualMatrixInputStream.write(reinterpret_cast<char*>(actualMatrixValues.data()), actualMatrixValues.size() * sizeof(double));
   
   Eigen::MatrixXd actualMatrix;
   read_dense(actualMatrixInputStream, DenseMatrixType::ddm, actualMatrix);

   Eigen::MatrixXd expectedMatrix(3, 3);
   expectedMatrix(0, 0) = 1; expectedMatrix(0, 1) = 2; expectedMatrix(0, 2) = 3;
   expectedMatrix(1, 0) = 4; expectedMatrix(1, 1) = 5; expectedMatrix(1, 2) = 6;
   expectedMatrix(2, 0) = 7; expectedMatrix(2, 1) = 8; expectedMatrix(2, 2) = 9;

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/read_dense(std::istream& in, DenseMatrixType denseMatrixType, Eigen::MatrixXd& X). DenseMatrixType::csv")
{
   long actualMatrixNRow = 3;
   long actualMatrixNCol = 3;
   std::vector<double> actualMatrixValues = { 1, 4, 7, 2, 5, 8, 3, 6, 9 };

   std::stringstream actualMatrixInputStream;
   actualMatrixInputStream << "3" << std::endl << "3" << std::endl;
   actualMatrixInputStream << "1, 2, 3" << std::endl;
   actualMatrixInputStream << "4,5,6" << std::endl;
   actualMatrixInputStream << "7, 8, 9" << std::endl;
   Eigen::MatrixXd actualMatrix;
   read_dense(actualMatrixInputStream, DenseMatrixType::csv, actualMatrix);

   Eigen::MatrixXd expectedMatrix(3, 3);
   expectedMatrix(0, 0) = 1; expectedMatrix(0, 1) = 2; expectedMatrix(0, 2) = 3;
   expectedMatrix(1, 0) = 4; expectedMatrix(1, 1) = 5; expectedMatrix(1, 2) = 6;
   expectedMatrix(2, 0) = 7; expectedMatrix(2, 1) = 8; expectedMatrix(2, 2) = 9;

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/read_dense(std::istream& in, DenseMatrixType denseMatrixType, Eigen::VectorXd& V). DenseMatrixType::ddm")
{
   long actualVectorNRow = 3;
   long actualVectorNCol = 1;
   std::vector<double> actualVectorValues = { 1, 4, 7 };

   std::stringstream actualVectorInputStream;
   actualVectorInputStream.write(reinterpret_cast<char*>(&actualVectorNRow), sizeof(long));
   actualVectorInputStream.write(reinterpret_cast<char*>(&actualVectorNCol), sizeof(long));
   actualVectorInputStream.write(reinterpret_cast<char*>(actualVectorValues.data()), actualVectorValues.size() * sizeof(double));

   Eigen::VectorXd actualVector;
   read_dense(actualVectorInputStream, DenseMatrixType::ddm, actualVector);

   Eigen::VectorXd expectedVector(3, 1);
   expectedVector(0, 0) = 1;
   expectedVector(1, 0) = 4;
   expectedVector(2, 0) = 7;

   REQUIRE(actualVector.isApprox(expectedVector));
}

TEST_CASE("matrix_io/read_dense(std::istream& in, DenseMatrixType denseMatrixType, Eigen::VectorXd& V). DenseMatrixType::csv")
{
   long actualVectorNRow = 3;
   long actualVectorNCol = 1;
   std::vector<double> actualVectorValues = { 1, 4, 7 };

   std::stringstream actualVectorInputStream;
   actualVectorInputStream << "3" << std::endl << "1" << std::endl;
   actualVectorInputStream << "1" << std::endl;
   actualVectorInputStream << "4" << std::endl;
   actualVectorInputStream << "7" << std::endl;
   Eigen::VectorXd actualVector;
   read_dense(actualVectorInputStream, DenseMatrixType::csv, actualVector);

   Eigen::VectorXd expectedVector(3, 1);
   expectedVector(0, 0) = 1;
   expectedVector(1, 0) = 4;
   expectedVector(2, 0) = 7;

   REQUIRE(actualVector.isApprox(expectedVector));
}

TEST_CASE("matrix_io/write_dense(std::ostream& out, DenseMatrixType denseMatrixType, const Eigen::MatrixXd &X). DenseMatrixType::ddm")
{
   Eigen::MatrixXd actualMatrix(3, 3);
   actualMatrix(0, 0) = 1; actualMatrix(0, 1) = 2; actualMatrix(0, 2) = 3;
   actualMatrix(1, 0) = 4; actualMatrix(1, 1) = 5; actualMatrix(1, 2) = 6;
   actualMatrix(2, 0) = 7; actualMatrix(2, 1) = 8; actualMatrix(2, 2) = 9;

   std::stringstream actualMatrixOutputStream;
   write_dense(actualMatrixOutputStream, DenseMatrixType::ddm, actualMatrix);

   long expectedMatrixNRow = 3;
   long expectedMatrixNCol = 3;
   std::vector<double> expectedMatrixValues = { 1, 4, 7, 2, 5, 8, 3, 6, 9 };
   std::stringstream expectedMatrixOutputStream;
   expectedMatrixOutputStream.write(reinterpret_cast<char*>(&expectedMatrixNRow), sizeof(long));
   expectedMatrixOutputStream.write(reinterpret_cast<char*>(&expectedMatrixNCol), sizeof(long));
   expectedMatrixOutputStream.write(reinterpret_cast<char*>(expectedMatrixValues.data()), expectedMatrixValues.size() * sizeof(double));

   REQUIRE(actualMatrixOutputStream.str() == expectedMatrixOutputStream.str());
}

TEST_CASE("matrix_io/write_dense(std::ostream& out, DenseMatrixType denseMatrixType, const Eigen::MatrixXd &X). DenseMatrixType::csv")
{
   Eigen::MatrixXd actualMatrix(3, 3);
   actualMatrix(0, 0) = 1; actualMatrix(0, 1) = 2; actualMatrix(0, 2) = 3;
   actualMatrix(1, 0) = 4; actualMatrix(1, 1) = 5; actualMatrix(1, 2) = 6;
   actualMatrix(2, 0) = 7; actualMatrix(2, 1) = 8; actualMatrix(2, 2) = 9;

   std::stringstream actualMatrixOutputStream;
   write_dense(actualMatrixOutputStream, DenseMatrixType::csv, actualMatrix);

   std::stringstream expectedMatrixOutputStream;
   expectedMatrixOutputStream << 3 << std::endl;
   expectedMatrixOutputStream << 3 << std::endl;
   expectedMatrixOutputStream << "1,2,3" << std::endl;
   expectedMatrixOutputStream << "4,5,6" << std::endl;
   expectedMatrixOutputStream << "7,8,9" << std::endl;

   REQUIRE(actualMatrixOutputStream.str() == expectedMatrixOutputStream.str());
}