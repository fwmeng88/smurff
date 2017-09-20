#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <iostream>
#include <cassert>
#include <fstream>
#include <string>
#include <algorithm>
#include <random>
#include <chrono>
#include <memory>
#include <cmath>

#include <unsupported/Eigen/SparseExtra>
#include <Eigen/Sparse>

#if defined(_OPENMP)
#include <omp.h>
#endif

#include <signal.h>

#include "model.h"
#include "utils.h"
#include "data.h"
#include "mvnormal.h"

#include "Noiseless.h"

using namespace std;
using namespace Eigen;

namespace smurff {


////----- Data below

//
//-- ScarceMatrixData specific stuff


//
//-- FullMatrixData specific stuff
//

void SparseMatrixData::center(double global_mean)
{
    this->global_mean = global_mean;
    if (center_mode == CENTER_GLOBAL) {
        Yc.at(0).coeffs() -= global_mean;
        Yc.at(1).coeffs() -= global_mean;
    } else if (center_mode == CENTER_VIEW) {
        Yc.at(0).coeffs() -= cwise_mean;
        Yc.at(1).coeffs() -= cwise_mean;
    } else if (center_mode == CENTER_COLS) {
        // you cannot col/row center fully know sparse matrices
        // without converting to dense
        assert(false);
    } else if (center_mode == CENTER_ROWS) {
        // you cannot col/row center fully know sparse matrices
        // without converting to dense
        assert(false);
    }
}

double SparseMatrixData::train_rmse(const SubModel &model) const {
    double se = 0.;
#pragma omp parallel for schedule(guided) reduction(+:se)
    for(int c=0; c<Y.cols();++c) {
        int r = 0;
        for (SparseMatrix<double>::InnerIterator it(Y, c); it; ++it) {
            while (r<it.row()) se += square(predict({c,r++}, model));
            se += square(it.value() - predict({c,r}, model));
        }
        for(;r<Y.rows();r++) se += square(predict({c,r}, model));
    }
    return sqrt( se / Y.rows() / Y.cols() );
}

void DenseMatrixData::center(double global_mean)
{
    this->global_mean = global_mean;
    if (center_mode == CENTER_GLOBAL) {
        Yc.at(0).array() -= global_mean;
        Yc.at(1).array() -= global_mean;
    } else if (center_mode == CENTER_VIEW) {
        Yc.at(0).array() -= cwise_mean;
        Yc.at(1).array() -= cwise_mean;
    } else if (center_mode == CENTER_COLS) {
        Yc.at(0).rowwise() -= mode_mean.at(0).transpose();
        Yc.at(1) = Yc.at(0).transpose();
    } else if (center_mode == CENTER_ROWS) {
        Yc.at(1).rowwise() -= mode_mean.at(1).transpose();
        Yc.at(0) = Yc.at(1).transpose();
    }
}

double DenseMatrixData::train_rmse(const SubModel &model) const
{
    double se = 0.;
#pragma omp parallel for schedule(guided) reduction(+:se)
    for(int c=0; c<Y.cols();++c) {
        for(int m=0; m<Y.rows(); ++m) {
            se += square(Y(m,c) - predict({c,m}, model));
        }
    }
    return sqrt( se / Y.rows() / Y.cols() );
}



} //end namespace smurff

#ifdef BENCH

#include "utils.h"

int main()
{
    const int N = 32 * 1024;
    const int K = 96;
    const int R = 20;
    {
        init_bmrng(1234);
        MatrixXd U(K,N);
        bmrandn(U);

        MatrixXd M(K,K) ;
        double start = tick();
        for(int i=0; i<R; ++i) {
            M.setZero();
            for(int j=0; j<N;++j) {
                const auto &col = U.col(j);
                M.noalias() += col * col.transpose();
            }
        }
        double stop = tick();
        std::cout << "norm U: " << U.norm() << std::endl;
        std::cout << "norm M: " << M.norm() << std::endl;
        std::cout << "MatrixXd: " << stop - start << std::endl;
    }

    {
        init_bmrng(1234);
        Matrix<double, K, Dynamic> U(K,N);
        U = nrandn(K,N);

        Matrix<double,K,K> M;
        double start = tick();
        for(int i=0; i<R; ++i) {
            M.setZero();
            for(int j=0; j<N;++j) {
                const auto &col = U.col(j);
                M.noalias() += col * col.transpose();
            }
        }
        double stop = tick();
        std::cout << "norm U: " << U.norm() << std::endl;
        std::cout << "norm M: " << M.norm() << std::endl;
        std::cout << "MatrixNNd: " << stop - start << std::endl;
    }

    return 0;
}
#endif
