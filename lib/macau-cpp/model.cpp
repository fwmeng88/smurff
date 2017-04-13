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

#include "utils.h"
#include "model.h"
#include "mvnormal.h"

using namespace std; 
using namespace Eigen;

namespace Macau {

int Factors::num_latent = -1;

void Factors::setRelationDataTest(int* rows, int* cols, double* values, int N, int nrows, int ncols) {
    for(int i=0; i<N; ++i) Ytest.push_back({rows[i], cols[i], values[i]});
    Ytestrows = nrows;
    Ytestcols = ncols;
    init_predictions();
}
 
void Factors::setRelationDataTest(SparseDoubleMatrix &Y) {
    for(unsigned i=0; i<Y.nnz; ++i) Ytest.push_back({Y.rows[i], Y.cols[i], Y.vals[i]});
    Ytestrows = Y.nrow;
    Ytestcols = Y.ncol;
    init_predictions();
}

void Factors::setRelationDataTest(SparseMatrixD Y) {
    for (int k = 0; k < Y.outerSize(); ++k) {
        for (Eigen::SparseMatrix<double>::InnerIterator it(Y,k); it; ++it) {
            Ytest.push_back({(int)it.row(), (int)it.col(), it.value()});
        }
    }
    Ytestrows = Y.rows();
    Ytestcols = Y.cols();
    init_predictions();
}

void Factors::init_predictions() {
    const int N = Ytest.size();
    permutation.resize(N);
    for(unsigned int i = 0; i < Ytest.size(); i++) permutation[i] = i;
}

//--- output model to files

void Factors::savePredictions(std::string save_prefix, int iter, int burnin) {
    std::string fname_pred = save_prefix + "-predictions.csv";
    std::ofstream predfile;
    predfile.open(fname_pred);
    predfile << "row,col,y,y_pred,y_pred_std\n";
    for (unsigned i = 0; i < Ytest.size(); i++) {
        auto &t = Ytest[i];
        predfile        << to_string( t.row );
        predfile << "," << to_string( t.col );
        predfile << "," << to_string( t.val );
        predfile << "," << to_string( t.pred );
        predfile << "," << to_string( t.stds );
        predfile << "\n";
    }
    predfile.close();
    printf("Saved predictions into '%s'.\n", fname_pred.c_str());

}

void Factors::saveGlobalParams(std::string save_prefix) {
  VectorXd means(1);
  means << mean_rating;
  writeToCSVfile(save_prefix + "-meanvalue.csv", means);
}

void Factors::saveModel(std::string save_prefix, int iter, int burnin) {
    int i = 0;
    for(auto &U : factors) {
        writeToCSVfile(save_prefix + "-U" + std::to_string(i++) + "-latents.csv", U);
    }
    savePredictions(save_prefix, iter, burnin);
}

///--- update RMSE and AUC

void Factors::update_predictions(int iter, int burnin)
{
    if (Ytest.size() == 0) return;
    assert(last_iter <= iter);

    if (last_iter == iter) return;

    assert(last_iter + 1 == iter);

    double se = 0.0, se_avg = 0.0;
    const double inorm = 1.0 / (iter - burnin - 1);
#pragma omp parallel for schedule(guided) reduction(+:se, se_avg)
    for (unsigned k = 0; k < Ytest.size(); ++k) {
        auto &t = Ytest[k];
        const double pred = col(0,t.col).dot(col(1,t.row)) + mean_rating;
        se += square(t.val - pred);

        // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Online_algorithm
        double pred_avg;
        if (iter <= burnin) {
            pred_avg = pred;
        } else {
            double delta = pred - t.pred;
            pred_avg = (t.pred + delta / (iter - burnin + 1));
            t.var += delta * (pred - pred_avg);
        }
        se_avg += square(t.val - pred_avg);
        t.pred = pred_avg;
        t.stds = sqrt(t.var * inorm);
    }

    const unsigned N = Ytest.size();
    rmse = sqrt( se / N );
    rmse_avg = sqrt( se_avg / N );
    last_iter = iter;
}

std::pair<double,double> Factors::getRMSE(int iter, int burnin)
{
    update_predictions(iter, burnin);
    return std::make_pair(rmse, rmse_avg);
}

double Factors::auc(double threshold)
{
    if (Ytest.size() == 0) return NAN;

    auto start = tick();
    std::sort(Ytest.begin(), Ytest.end(), [this](const YTestItem &a, const YTestItem &b) { return a.pred < b.pred;});

    //Build stack_x and stack_y
    double stack_x = Ytest[0].val;
    double stack_y = 1-stack_x;
    int num_positive = 0;
    int num_negative = 0;
    double auc = .0;
    for(auto &t : Ytest) {
        int is_positive = t.val > threshold;
        int is_negative = !is_positive; 
        stack_x += is_positive;
        stack_y += is_negative; 
        num_positive += is_positive;
        num_negative += is_negative;
        auc += is_positive * stack_y;
    }

    auc /= num_positive * num_negative;


    auto stop = tick();
    std::cout << "AUC time: " << stop-start << std::endl;

    return auc;
}

std::ostream &Factors::printInitStatus(std::ostream &os, std::string indent)
{
    os << indent << "Type: " << name << "\n";
    os << indent << "Num-latents: " << num_latent << "\n";
    double train_fill_rate = 100. * Ynnz() / Yrows() / Ycols();
    os << indent << "Train data: " << Ynnz() << " [" << Yrows() << " x " << Ycols() << "] (" << train_fill_rate << "%)\n";
    if (Ytest.size()) {
        double test_fill_rate = 100. * Ytest.size() / Ytestrows / Ytestcols;
        os << indent << "Test data: " << Ytest.size() << " [" << Ytestrows << " x " << Ytestcols << "] (" << test_fill_rate << "%)\n";
    } else {
        os << indent << "Test data: -\n";
    }
    return os;
}

template<typename YType>
void MF<YType>::init_base()
{
    assert(Yrows() > 0 && Ycols() > 0);
    if (Ytest.size() > 0) {
        assert(Ytestrows == Yrows() && Ytestcols == Ycols() && "Size of train must be equal to size of test");
    }

    mean_rating = Y.sum() / Y.nonZeros();

    U(0).resize(num_latent, Y.cols());
    U(1).resize(num_latent, Y.rows());

    bmrandn(U(0));
    bmrandn(U(1));

    Yc.push_back(Y);
    Yc.push_back(Y.transpose());
}

template<>
void MF<SparseMatrixD>::init()
{
    init_base();
    Yc.at(0).coeffs() -= mean_rating;
    Yc.at(1).coeffs() -= mean_rating;
    name = name + " [with NAs]";
}


template<>
void MF<Eigen::MatrixXd>::init()
{
    init_base();
    Yc.at(0).array() -= this->mean_rating;
    Yc.at(1).array() -= this->mean_rating;
    name = "Dense" + name;
}

template<typename YType>
void MF<YType>::setRelationData(YType Y) {
    this->Y = Y;
}


template<>
double MF<Eigen::MatrixXd>::var_total() const {
    auto &Y = Yc.at(0);
    double se = Y.array().square().sum();

    double var_total = se / Y.nonZeros();
    if (var_total <= 0.0 || std::isnan(var_total)) {
        // if var cannot be computed using 1.0
        var_total = 1.0;
    }

    return var_total;
}

template<>
double MF<SparseMatrixD>::var_total() const {
    double se = 0.0;
    auto &Y = Yc.at(0);

#pragma omp parallel for schedule(dynamic, 4) reduction(+:se)
    for (int k = 0; k < Y.outerSize(); ++k) {
        for (SparseMatrix<double>::InnerIterator it(Y,k); it; ++it) {
            se += square(it.value());
        }
    }

    double var_total = se / Y.nonZeros();
    if (var_total <= 0.0 || std::isnan(var_total)) {
        // if var cannot be computed using 1.0
        var_total = 1.0;
    }

    return var_total;
}

// for the adaptive gaussian noise
template<>
double MF<Eigen::MatrixXd>::sumsq() const {
    double sumsq = 0.0;

#pragma omp parallel for schedule(dynamic, 4) reduction(+:sumsq)
    for (int j = 0; j < this->Y.cols(); j++) {
        auto Vj = this->U(1).col(j);
        for (int i = 0; i < this->Y.rows(); i++) {
            double Yhat = Vj.dot( this->U(0).col(j) ) + this->mean_rating;
            sumsq += square(Yhat - this->Y(i,j));
        }
    }

    return sumsq;
}

template<>
double MF<SparseMatrixD>::sumsq() const {
    double sumsq = 0.0;

#pragma omp parallel for schedule(dynamic, 4) reduction(+:sumsq)
    for (int j = 0; j < Y.outerSize(); j++) {
        auto Uj = col(0, j);
        for (SparseMatrix<double>::InnerIterator it(Y, j); it; ++it) {
            double Yhat = Uj.dot( col(1, it.row()) ) + mean_rating;
            sumsq += square(Yhat - it.value());
        }
    }

    return sumsq;
}

template<>
void MF<SparseMatrixD>::setRelationData(int* rows, int* cols, double* values, int N, int nrows, int ncols) {
    Y.resize(nrows, ncols);
    sparseFromIJV(Y, rows, cols, values, N);
}

template<>
void MF<SparseMatrixD>::setRelationData(SparseDoubleMatrix &Y) {
    setRelationData(Y.rows, Y.cols, Y.vals, Y.nnz, Y.nrow, Y.ncol);
}

//
//-- SparseMF specific stuff
//
//
//

struct pnm_perf_item {
    int local_nnz, total_nnz;
    bool in_parallel;
    double start, stop;
};

static std::vector<pnm_perf_item> pnm_perf;

void SparseMF::get_pnm(int f, int n, VectorXd &rr, MatrixXd &MM) {
    auto &Y = Yc.at(f);
    MatrixXd &Vf = V(f);
    const int local_nnz = Y.col(n).nonZeros();
    const int total_nnz = Y.nonZeros();
    bool in_parallel = (local_nnz >10000) || ((double)local_nnz > (double)total_nnz / 100.);
    // extra parallization for samples with >1% of nonzeros
    //std::cout << "local_nnz : " << local_nnz << std::endl;
    //std::cout << "total_nnz / 1000: " << total_nnz/1000. << std::endl;
    if (in_parallel) {
#if 0
#pragma omp critical
        printf("%d-%d with %d nnz started on thread %d\n", f, n, local_nnz, thread_num());
#endif
        const int task_size = ceil(local_nnz / 100.0);
        auto from = Y.outerIndexPtr()[n];
        auto to = Y.outerIndexPtr()[n+1];
        thread_vector<VectorXd> rrs(VectorXd::Zero(num_latent));
        thread_vector<MatrixXd> MMs(MatrixXd::Zero(num_latent, num_latent)); 
        for(int j=from; j<to; j+=task_size) {
#pragma omp task shared(Y,Vf,rrs,MMs)
            {
#if 0
#pragma omp critical
                printf("%d-%d with %d nnz running a task on thread %d\n", f, n, local_nnz, thread_num());
#endif
                auto &my_rr = rrs.local();
                auto &my_MM = MMs.local();

                for(int i=j; i<std::min(j+task_size,to); ++i)
                {
                    auto val = Y.valuePtr()[i];
                    auto idx = Y.innerIndexPtr()[i];
                    const auto &col = Vf.col(idx);
                    my_rr.noalias() += col * val;
                    my_MM.triangularView<Eigen::Lower>() += col * col.transpose();
                }
            }
        }
#if 0
        printf("%d-%d with %d nnz is waiting\n", f, n, local_nnz);
#endif
#pragma omp taskwait
#if 0 
#pragma omp critical
        printf("%d-%d with %d nnz finished waiting\n", f, n, local_nnz);
#endif
        MM += MMs.combine();
        rr += rrs.combine();
    } else {
        for (SparseMatrix<double>::InnerIterator it(Y, n); it; ++it) {
            const auto &col = Vf.col(it.row());
            rr.noalias() += col * it.value();
            MM.triangularView<Lower>() += col * col.transpose();
        }
    }

    double stop = tick();

    MM.triangularView<Upper>() = MM.transpose();

#if 0
#pragma omp critical
    pnm_perf.push_back({local_nnz, total_nnz, in_parallel, start, stop});
#endif
}

void SparseMF::update_pnm(int f) {
    return;
    if (pnm_perf.size()) {
        printf("==========\n"); 
        for(auto &item: pnm_perf) printf("%d;%d;%d;%f;%f\n", item.local_nnz, item.total_nnz, item.in_parallel, item.start, item.stop);
        pnm_perf.clear();
    }

    auto &Y = Yc.at(f);

    int bin = 1;
    int count = 0;
    int total = 0;
    while (count < Y.cols()) {
        count = 0;
        for(int i=0; i<Y.cols();++i) if (Y.col(i).nonZeros() < bin) count++;
        auto bin_count = count - total;
        auto bin_nnz = bin_count * bin;
        auto bin_percent = (100. * bin_nnz) / Y.nonZeros();
            printf("fac: %d\t%5d < bin < %5d;\t#samples: %4d;\t%5d < #nnz < %5d;\t %.1f < %%nnz < %.1f\n",
                    f, bin/2, bin, bin_count, bin_nnz/2, bin_nnz, bin_percent/2, bin_percent);
        bin *= 2;
        total = count;
    }

    std::cout << "Total samples: " << Y.cols() << std::endl;
    std::cout << "Total nnz: " << Y.nonZeros() << std::endl;
}

void SparseBinaryMF::get_pnm(int f, int n, VectorXd &rr, MatrixXd &MM)
{
    auto u = U(f).col(n);
    for (SparseMatrix<double>::InnerIterator it(Yc.at(f), n); it; ++it) {
        const auto &col = V(f).col(it.row());
        MM.noalias() += col * col.transpose();
        auto z = (2 * it.value() - 1) * fabs(col.dot(u) + bmrandn_single());
        rr.noalias() += col * z;
    }
}


//
//-- DenseMF specific stuff
//

template<class YType>
void DenseMF<YType>::get_pnm(int f, int d, VectorXd &rr, MatrixXd &MM) {
    auto &Y = this->Yc.at(f);
    rr.noalias() += (this->V(f) * Y.col(d));
    MM.noalias() += VV.at(f); 
}

template<class YType>
void DenseMF<YType>::update_pnm(int f) {
    auto &Vf = this->V(f);
    thread_vector<MatrixXd> VVs(MatrixXd::Zero(this->num_latent, this->num_latent));

#pragma omp parallel for schedule(dynamic, 8) shared(VVs)
    for(int n = 0; n < Vf.cols(); n++) {
        auto v = Vf.col(n);
        VVs.local() += v * v.transpose();
    }

    VV.at(f) = VVs.combine();
}

template struct MF<SparseMatrix<double>>;
template struct MF<MatrixXd>;
template struct DenseMF<Eigen::MatrixXd>;
template struct DenseMF<SparseMatrixD>;

} //end namespace Macau

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
