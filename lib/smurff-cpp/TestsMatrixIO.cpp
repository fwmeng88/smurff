#include "catch.hpp"

#include "matrix_io.h"
#include "MatrixUtils.h"

#include <sstream>
#include <cstdio>
#include <Eigen/Core>
#include <Eigen/SparseCore>

using namespace smurff;

TEST_CASE("matrix_io/read_matrix | matrix_io/write_matrix | .ddm")
{
   std::string matrixFilename = "matrixConfig.ddm";

   std::uint64_t matrixConfigNRow = 3;
   std::uint64_t matrixConfigNCol = 3;
   std::vector<double> matrixConfigValues = { 1, 4, 7, 2, 5, 8, 3, 6, 9 };
   MatrixConfig matrixConfig( matrixConfigNRow
                            , matrixConfigNCol
                            , std::move(matrixConfigValues)
                            , NoiseConfig()
                            );

   matrix_io::write_matrix(matrixFilename, matrixConfig);
   MatrixConfig actualMatrixConfig = matrix_io::read_matrix(matrixFilename);
   Eigen::MatrixXd actualMatrix = dense_to_eigen(actualMatrixConfig);

   Eigen::MatrixXd expectedMatrix(3, 3);
   expectedMatrix << 1, 2, 3, 4, 5, 6, 7, 8, 9;

   std::remove(matrixFilename.c_str());
   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/read_matrix | matrix_io/write_matrix | .csv")
{
   std::string matrixFilename = "matrixConfig.csv";

   std::uint64_t matrixConfigNRow = 3;
   std::uint64_t matrixConfigNCol = 3;
   std::vector<double> matrixConfigValues = { 1, 4, 7, 2, 5, 8, 3, 6, 9 };
   MatrixConfig matrixConfig( matrixConfigNRow
                            , matrixConfigNCol
                            , std::move(matrixConfigValues)
                            , NoiseConfig()
                            );

   matrix_io::write_matrix(matrixFilename, matrixConfig);
   MatrixConfig actualMatrixConfig = matrix_io::read_matrix(matrixFilename);
   Eigen::MatrixXd actualMatrix = dense_to_eigen(actualMatrixConfig);

   Eigen::MatrixXd expectedMatrix(3, 3);
   expectedMatrix << 1, 2, 3, 4, 5, 6, 7, 8, 9;

   std::remove(matrixFilename.c_str());
   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/read_matrix | matrix_io/write_matrix | .sdm")
{
   std::string matrixFilename = "matrixConfig.sdm";

   std::uint64_t matrixConfigNRow = 3;
   std::uint64_t matrixConfigNCol = 3;
   std::vector<std::uint32_t> matrixConfigRows = { 0, 0, 0, 2, 2, 2 };
   std::vector<std::uint32_t> matrixConfigCols = { 0, 1, 2, 0, 1, 2 };
   std::vector<double> matrixConfigValues      = { 1, 2, 3, 7, 8, 9 };
   MatrixConfig matrixConfig( matrixConfigNRow
                            , matrixConfigNCol
                            , std::move(matrixConfigRows)
                            , std::move(matrixConfigCols)
                            , std::move(matrixConfigValues)
                            , NoiseConfig()
                            );

   matrix_io::write_matrix(matrixFilename, matrixConfig);
   MatrixConfig actualMatrixConfig = matrix_io::read_matrix(matrixFilename);
   Eigen::SparseMatrix<double> actualMatrix = sparse_to_eigen(actualMatrixConfig);

   Eigen::SparseMatrix<double> expectedMatrix(3, 3);
   std::vector<Eigen::Triplet<double> > expectedMatrixTriplets;
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 0, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 1, 2));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 2, 3));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 0, 7));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 1, 8));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 2, 9));
   expectedMatrix.setFromTriplets(expectedMatrixTriplets.begin(), expectedMatrixTriplets.end());

   std::remove(matrixFilename.c_str());
   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/read_matrix | matrix_io/write_matrix | .mtx")
{
   std::string matrixFilename = "matrixConfig.mtx";

   std::uint64_t matrixConfigNRow = 3;
   std::uint64_t matrixConfigNCol = 3;
   std::vector<std::uint32_t> matrixConfigRows = { 0, 0, 0, 2, 2, 2 };
   std::vector<std::uint32_t> matrixConfigCols = { 0, 1, 2, 0, 1, 2 };
   std::vector<double> matrixConfigValues      = { 1, 2, 3, 7, 8, 9 };
   MatrixConfig matrixConfig( matrixConfigNRow
                            , matrixConfigNCol
                            , std::move(matrixConfigRows)
                            , std::move(matrixConfigCols)
                            , std::move(matrixConfigValues)
                            , NoiseConfig()
                            );

   matrix_io::write_matrix(matrixFilename, matrixConfig);
   MatrixConfig actualMatrixConfig = matrix_io::read_matrix(matrixFilename);
   Eigen::SparseMatrix<double> actualMatrix = sparse_to_eigen(actualMatrixConfig);

   Eigen::SparseMatrix<double> expectedMatrix(3, 3);
   std::vector<Eigen::Triplet<double> > expectedMatrixTriplets;
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 0, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 1, 2));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 2, 3));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 0, 7));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 1, 8));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 2, 9));
   expectedMatrix.setFromTriplets(expectedMatrixTriplets.begin(), expectedMatrixTriplets.end());

   std::remove(matrixFilename.c_str());
   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/read_matrix | matrix_io/write_matrix | .sbm")
{
   std::string matrixFilename = "matrixConfig.sbm";

   std::uint64_t matrixConfigNRow = 3;
   std::uint64_t matrixConfigNCol = 3;
   std::vector<std::uint32_t> matrixConfigRows = { 0, 0, 0, 2, 2, 2 };
   std::vector<std::uint32_t> matrixConfigCols = { 0, 1, 2, 0, 1, 2 };
   MatrixConfig matrixConfig( matrixConfigNRow
                            , matrixConfigNCol
                            , std::move(matrixConfigRows)
                            , std::move(matrixConfigCols)
                            , NoiseConfig()
                            );

   matrix_io::write_matrix(matrixFilename, matrixConfig);
   MatrixConfig actualMatrixConfig = matrix_io::read_matrix(matrixFilename);
   Eigen::SparseMatrix<double> actualMatrix = sparse_to_eigen(actualMatrixConfig);

   Eigen::SparseMatrix<double> expectedMatrix(3, 3);
   std::vector<Eigen::Triplet<double> > expectedMatrixTriplets;
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 0, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 1, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 2, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 0, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 1, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 2, 1));
   expectedMatrix.setFromTriplets(expectedMatrixTriplets.begin(), expectedMatrixTriplets.end());

   std::remove(matrixFilename.c_str());
   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

// ===

TEST_CASE("matrix_io/read_dense_float64_bin | matrix_io/write_dense_float64_bin")
{
   std::uint64_t matrixConfigNRow = 3;
   std::uint64_t matrixConfigNCol = 3;
   std::vector<double> matrixConfigValues = { 1, 4, 7, 2, 5, 8, 3, 6, 9 };
   MatrixConfig matrixConfig( matrixConfigNRow
                            , matrixConfigNCol
                            , std::move(matrixConfigValues)
                            , NoiseConfig()
                            );

   std::stringstream matrixStream;
   matrix_io::write_dense_float64_bin(matrixStream, matrixConfig);
   MatrixConfig actualMatrixConfig = matrix_io::read_dense_float64_bin(matrixStream);
   Eigen::MatrixXd actualMatrix = dense_to_eigen(actualMatrixConfig);

   Eigen::MatrixXd expectedMatrix(3, 3);
   expectedMatrix << 1, 2, 3, 4, 5, 6, 7, 8, 9;

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/read_dense_float64_csv | matrix_io/write_dense_float64_csv")
{
   std::uint64_t matrixConfigNRow = 3;
   std::uint64_t matrixConfigNCol = 3;
   std::vector<double> matrixConfigValues = { 1, 4, 7, 2, 5, 8, 3, 6, 9 };
   MatrixConfig matrixConfig( matrixConfigNRow
                            , matrixConfigNCol
                            , std::move(matrixConfigValues)
                            , NoiseConfig()
                            );

   std::stringstream matrixConfigStream;
   matrix_io::write_dense_float64_csv(matrixConfigStream, matrixConfig);
   MatrixConfig actualMatrixConfig = matrix_io::read_dense_float64_csv(matrixConfigStream);
   Eigen::MatrixXd actualMatrix = dense_to_eigen(actualMatrixConfig);

   Eigen::MatrixXd expectedMatrix(3, 3);
   expectedMatrix << 1, 2, 3, 4, 5, 6, 7, 8, 9;

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/read_sparse_float64_bin | matrix_io/write_sparse_float64_bin")
{
   std::uint64_t matrixConfigNRow = 3;
   std::uint64_t matrixConfigNCol = 3;
   std::vector<std::uint32_t> matrixConfigRows = { 0, 0, 0, 2, 2, 2 };
   std::vector<std::uint32_t> matrixConfigCols = { 0, 1, 2, 0, 1, 2 };
   std::vector<double> matrixConfigValues      = { 1, 2, 3, 7, 8, 9 };
   MatrixConfig matrixConfig( matrixConfigNRow
                            , matrixConfigNCol
                            , std::move(matrixConfigRows)
                            , std::move(matrixConfigCols)
                            , std::move(matrixConfigValues)
                            , NoiseConfig()
                            );

   std::stringstream matrixConfigStream;
   matrix_io::write_sparse_float64_bin(matrixConfigStream, matrixConfig);
   MatrixConfig actualMatrixConfig = matrix_io::read_sparse_float64_bin(matrixConfigStream);
   Eigen::SparseMatrix<double> actualMatrix = sparse_to_eigen(actualMatrixConfig);

   Eigen::SparseMatrix<double> expectedMatrix(3, 3);
   std::vector<Eigen::Triplet<double> > expectedMatrixTriplets;
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 0, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 1, 2));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 2, 3));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 0, 7));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 1, 8));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 2, 9));
   expectedMatrix.setFromTriplets(expectedMatrixTriplets.begin(), expectedMatrixTriplets.end());

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/read_sparse_float64_mtx | matrix_io/write_sparse_float64_mtx")
{
   std::uint64_t matrixConfigNRow = 3;
   std::uint64_t matrixConfigNCol = 3;
   std::vector<std::uint32_t> matrixConfigRows = { 0, 0, 0, 2, 2, 2 };
   std::vector<std::uint32_t> matrixConfigCols = { 0, 1, 2, 0, 1, 2 };
   std::vector<double> matrixConfigValues      = { 1, 2, 3, 7, 8, 9 };
   MatrixConfig matrixConfig( matrixConfigNRow
                            , matrixConfigNCol
                            , std::move(matrixConfigRows)
                            , std::move(matrixConfigCols)
                            , std::move(matrixConfigValues)
                            , NoiseConfig()
                            );

   std::stringstream matrixConfigStream;
   matrix_io::write_sparse_float64_mtx(matrixConfigStream, matrixConfig);
   MatrixConfig actualMatrixConfig = matrix_io::read_sparse_float64_mtx(matrixConfigStream);
   Eigen::SparseMatrix<double> actualMatrix = sparse_to_eigen(actualMatrixConfig);

   Eigen::SparseMatrix<double> expectedMatrix(3, 3);
   std::vector<Eigen::Triplet<double> > expectedMatrixTriplets;
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 0, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 1, 2));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 2, 3));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 0, 7));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 1, 8));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 2, 9));
   expectedMatrix.setFromTriplets(expectedMatrixTriplets.begin(), expectedMatrixTriplets.end());

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/read_sparse_binary_bin | matrix_io/write_sparse_binary_bin")
{
   std::uint64_t matrixConfigNRow = 3;
   std::uint64_t matrixConfigNCol = 3;
   std::vector<std::uint32_t> matrixConfigRows = { 0, 0, 0, 2, 2, 2 };
   std::vector<std::uint32_t> matrixConfigCols = { 0, 1, 2, 0, 1, 2 };
   MatrixConfig matrixConfig( matrixConfigNRow
                            , matrixConfigNCol
                            , std::move(matrixConfigRows)
                            , std::move(matrixConfigCols)
                            , NoiseConfig()
                            );

   std::stringstream matrixConfigStream;
   matrix_io::write_sparse_binary_bin(matrixConfigStream, matrixConfig);
   MatrixConfig actualMatrixConfig = matrix_io::read_sparse_binary_bin(matrixConfigStream);
   Eigen::SparseMatrix<double> actualMatrix = sparse_to_eigen(actualMatrixConfig);

   Eigen::SparseMatrix<double> expectedMatrix(3, 3);
   std::vector<Eigen::Triplet<double> > expectedMatrixTriplets;
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 0, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 1, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 2, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 0, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 1, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 2, 1));
   expectedMatrix.setFromTriplets(expectedMatrixTriplets.begin(), expectedMatrixTriplets.end());

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

// ===

TEST_CASE("matrix_io/eigen::read_matrix(const std::string& filename, Eigen::VectorXd& V) | matrix_io/eigen::write_matrix(const std::string& filename, const Eigen::MatrixXd& X) | .ddm")
{
   REQUIRE(false);
}

TEST_CASE("matrix_io/eigen::read_matrix(const std::string& filename, Eigen::VectorXd& V) | matrix_io/eigen::write_matrix(const std::string& filename, const Eigen::MatrixXd& X) | .csv")
{
   REQUIRE(false);
}

TEST_CASE("matrix_io/eigen::read_matrix(const std::string& filename, Eigen::VectorXd& V) | matrix_io/eigen::write_matrix(const std::string& filename, const Eigen::MatrixXd& X) | .sdm")
{
   REQUIRE(false);
}

TEST_CASE("matrix_io/eigen::read_matrix(const std::string& filename, Eigen::VectorXd& V) | matrix_io/eigen::write_matrix(const std::string& filename, const Eigen::MatrixXd& X) | .mtx")
{
   REQUIRE(false);
}

TEST_CASE("matrix_io/eigen::read_matrix(const std::string& filename, Eigen::VectorXd& V) | matrix_io/eigen::write_matrix(const std::string& filename, const Eigen::MatrixXd& X) | .sbm")
{
   REQUIRE(false);
}

// ===

TEST_CASE("matrix_io/eigen::read_matrix(const std::string& filename, Eigen::MatrixXd& X) | matrix_io/eigen::write_matrix(const std::string& filename, const Eigen::MatrixXd& X) | .ddm")
{
   REQUIRE(false);
}

TEST_CASE("matrix_io/eigen::read_matrix(const std::string& filename, Eigen::MatrixXd& X) | matrix_io/eigen::write_matrix(const std::string& filename, const Eigen::MatrixXd& X) | .csv")
{
   REQUIRE(false);
}

TEST_CASE("matrix_io/eigen::read_matrix(const std::string& filename, Eigen::MatrixXd& X) | matrix_io/eigen::write_matrix(const std::string& filename, const Eigen::MatrixXd& X) | .sdm")
{
   REQUIRE(false);
}

TEST_CASE("matrix_io/eigen::read_matrix(const std::string& filename, Eigen::MatrixXd& X) | matrix_io/eigen::write_matrix(const std::string& filename, const Eigen::MatrixXd& X) | .mtx")
{
   REQUIRE(false);
}

TEST_CASE("matrix_io/eigen::read_matrix(const std::string& filename, Eigen::MatrixXd& X) | matrix_io/eigen::write_matrix(const std::string& filename, const Eigen::MatrixXd& X) | .sbm")
{
   REQUIRE(false);
}

// ===

TEST_CASE("matrix_io/eigen::read_matrix(const std::string& filename, Eigen::SparseMatrix<double>& X) | matrix_io/eigen::write_matrix(const std::string& filename, const Eigen::SparseMatrix<double>& X) | .ddm")
{
   REQUIRE(false);
}

TEST_CASE("matrix_io/eigen::read_matrix(const std::string& filename, Eigen::SparseMatrix<double>& X) | matrix_io/eigen::write_matrix(const std::string& filename, const Eigen::SparseMatrix<double>& X) | .csv")
{
   REQUIRE(false);
}

TEST_CASE("matrix_io/eigen::read_matrix(const std::string& filename, Eigen::SparseMatrix<double>& X) | matrix_io/eigen::write_matrix(const std::string& filename, const Eigen::SparseMatrix<double>& X) | .sdm")
{
   REQUIRE(false);
}

TEST_CASE("matrix_io/eigen::read_matrix(const std::string& filename, Eigen::SparseMatrix<double>& X) | matrix_io/eigen::write_matrix(const std::string& filename, const Eigen::SparseMatrix<double>& X) | .mtx")
{
   REQUIRE(false);
}

TEST_CASE("matrix_io/eigen::read_matrix(const std::string& filename, Eigen::SparseMatrix<double>& X) | matrix_io/eigen::write_matrix(const std::string& filename, const Eigen::SparseMatrix<double>& X) | .sbm")
{
   REQUIRE(false);
}

// ===

TEST_CASE("matrix_io/eigen::read_dense_float64_bin | matrix_io/eigen::write_dense_float64_bin")
{
   Eigen::MatrixXd expectedMatrix(3, 3);
   expectedMatrix << 1, 2, 3, 4, 5, 6, 7, 8, 9;

   std::stringstream matrixStream;
   matrix_io::eigen::write_dense_float64_bin(matrixStream, expectedMatrix);
   Eigen::MatrixXd actualMatrix(3, 3);
   matrix_io::eigen::read_dense_float64_bin(matrixStream, actualMatrix);

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/eigen::read_dense_float64_csv | matrix_io/eigen::write_dense_float64_csv")
{
   Eigen::MatrixXd expectedMatrix(3, 3);
   expectedMatrix << 1, 2, 3, 4, 5, 6, 7, 8, 9;

   std::stringstream matrixStream;
   matrix_io::eigen::write_dense_float64_csv(matrixStream, expectedMatrix);
   Eigen::MatrixXd actualMatrix(3, 3);
   matrix_io::eigen::read_dense_float64_csv(matrixStream, actualMatrix);

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/eigen::read_sparse_float64_bin | matrix_io/eigen::write_sparse_float64_bin")
{
   Eigen::SparseMatrix<double> expectedMatrix(3, 3);
   std::vector<Eigen::Triplet<double> > expectedMatrixTriplets;
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 0, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 1, 2));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 2, 3));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 0, 7));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 1, 8));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 2, 9));
   expectedMatrix.setFromTriplets(expectedMatrixTriplets.begin(), expectedMatrixTriplets.end());

   std::stringstream matrixStream;
   matrix_io::eigen::write_sparse_float64_bin(matrixStream, expectedMatrix);
   Eigen::SparseMatrix<double> actualMatrix(3, 3);
   matrix_io::eigen::read_sparse_float64_bin(matrixStream, actualMatrix);

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/eigen::read_sparse_float64_mtx | matrix_io/eigen::write_sparse_float64_mtx")
{
   Eigen::SparseMatrix<double> expectedMatrix(3, 3);
   std::vector<Eigen::Triplet<double> > expectedMatrixTriplets;
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 0, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 1, 2));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 2, 3));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 0, 7));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 1, 8));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 2, 9));
   expectedMatrix.setFromTriplets(expectedMatrixTriplets.begin(), expectedMatrixTriplets.end());

   std::stringstream matrixStream;
   matrix_io::eigen::write_sparse_float64_mtx(matrixStream, expectedMatrix);
   Eigen::SparseMatrix<double> actualMatrix(3, 3);
   matrix_io::eigen::read_sparse_float64_mtx(matrixStream, actualMatrix);

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}

TEST_CASE("matrix_io/eigen::read_sparse_binary_bin | matrix_io/eigen::write_sparse_binary_bin")
{
   Eigen::SparseMatrix<double> expectedMatrix(3, 3);
   std::vector<Eigen::Triplet<double> > expectedMatrixTriplets;
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 0, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 1, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(0, 2, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 0, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 1, 1));
   expectedMatrixTriplets.push_back(Eigen::Triplet<double>(2, 2, 1));
   expectedMatrix.setFromTriplets(expectedMatrixTriplets.begin(), expectedMatrixTriplets.end());

   std::stringstream matrixStream;
   matrix_io::eigen::write_sparse_binary_bin(matrixStream, expectedMatrix);
   Eigen::SparseMatrix<double> actualMatrix(3, 3);
   matrix_io::eigen::read_sparse_binary_bin(matrixStream, actualMatrix);

   REQUIRE(actualMatrix.isApprox(expectedMatrix));
}