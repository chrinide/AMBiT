#ifndef HAMILTONIAN_MATRIX_H
#define HAMILTONIAN_MATRIX_H

#include "RelativisticConfiguration.h"
#include "NonRelConfiguration.h"
#include "HartreeFock/HFOperator.h"
#include "Level.h"
#include "Universal/Enums.h"
#include "Universal/Matrix.h"
#include "ManyBodyOperator.h"
#include "MBPT/OneElectronIntegrals.h"
#include "MBPT/TwoElectronCoulombOperator.h"
#include "MBPT/Sigma3Calculator.h"
#include <Eigen/Eigen>

typedef ManyBodyOperator<pHFIntegrals, pTwoElectronCoulombOperator> TwoBodyHamiltonianOperator;
typedef std::shared_ptr<TwoBodyHamiltonianOperator> pTwoBodyHamiltonianOperator;

typedef ManyBodyOperator<pHFIntegrals, pTwoElectronCoulombOperator, pSigma3Calculator> ThreeBodyHamiltonianOperator;
typedef std::shared_ptr<ThreeBodyHamiltonianOperator> pThreeBodyHamiltonianOperator;

class HamiltonianMatrix : public Matrix
{
public:
    /** Hamiltonian with one and two-body operators. */
    HamiltonianMatrix(pHFIntegrals hf, pTwoElectronCoulombOperator coulomb, pRelativisticConfigList relconfigs);
    /** "Non-square" Hamiltonian with dimensions (Nsmall, N) where part of the N*N matrix is zero. */
    HamiltonianMatrix(pHFIntegrals hf, pTwoElectronCoulombOperator coulomb, pRelativisticConfigList relconfigs, unsigned int configsubsetend);
    /** Hamiltonian with additional effective three-body operator. */
    HamiltonianMatrix(pHFIntegrals hf, pTwoElectronCoulombOperator coulomb, pSigma3Calculator sigma3, pConfigListConst leadconfigs, pRelativisticConfigList relconfigs, unsigned int configsubsetend = 0);
    virtual ~HamiltonianMatrix();

    virtual void MatrixMultiply(int m, double* b, double* c) const;
    virtual void GetDiagonal(double* diag) const;

    /** Generate Hamiltonian matrix. */
    virtual void GenerateMatrix();

    /** Print upper triangular part of matrix (text). Lower triangular part is zeroed. */
    friend std::ostream& operator<<(std::ostream& stream, const HamiltonianMatrix& matrix);

    /** Write binary file. */
    virtual void Write(const std::string& filename) const;

    /** Return proportion of elements that have magnitude greater than epsilon. */
    virtual double PollMatrix(double epsilon = 1.e-15) const;

    /** Solve the matrix that has been generated. Note that this may destroy the matrix. */
    virtual LevelVector SolveMatrix(pHamiltonianID hID, unsigned int num_solutions);

#ifdef AMBIT_USE_SCALAPACK
    /** Solve using ScaLAPACK. Note that this destroys the matrix.
        If use_energy_limit is true, only return eigenvalues below energy_limit (up to num_solutions of them).
     */
    virtual LevelVector SolveMatrixScalapack(pHamiltonianID hID, unsigned int num_solutions, bool use_energy_limit, double energy_limit = 0.0);

    virtual LevelVector SolveMatrixScalapack(pHamiltonianID hID, double energy_limit)
    {   return SolveMatrixScalapack(hID, N, true, energy_limit);
    }
#endif

    /** Clear matrix and recover memory. */
    virtual void Clear() { chunks.clear(); }

protected:
    pRelativisticConfigList configs;
    pTwoBodyHamiltonianOperator H_two_body;

    pConfigListConst leading_configs;           //!< Leading configs for sigma3
    pThreeBodyHamiltonianOperator H_three_body; //!< Three-body operator is null if sigma3 not used

    unsigned int Nsmall;            //!< For non-square CI, the smaller matrix size
    unsigned int configsubsetend;   //!< End of smaller RelativisticConfigList

protected:
    typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> RowMajorMatrix;

    /** MatrixChunk is a rectangular section of the lower triangular part of the HamiltonianMatrix.
        The top left corner of the section is at (start_row, 0).
        The number of rows is num_rows, and the section goes to the diagonal of the Hamiltonian matrix (or Nsmall if chunk is in the extra part).
        The rows correspond to a number of RelativisticConfigurations in configs, as distributed by GenerateMatrix().
        RelativisticConfigurations included are [config_indices.first, config_indices.second).
     */
    class MatrixChunk
    {
    public:
        MatrixChunk(unsigned int config_index_start, unsigned int config_index_end, unsigned int row_start, unsigned int num_rows, unsigned int Nsmall):
            start_row(row_start), num_rows(num_rows)
        {   config_indices.first = config_index_start;
            config_indices.second = config_index_end;
            chunk = RowMajorMatrix::Zero(num_rows, mmin(start_row + num_rows, Nsmall));
        }

        std::pair<unsigned int, unsigned int> config_indices;
        unsigned int start_row;
        unsigned int num_rows;
        RowMajorMatrix chunk;

        /** Make upper triangle part of the matrix chunk match the lower. */
        void Symmetrize()
        {
            if(start_row < chunk.cols())
            {
                for(unsigned int i = 0; i < num_rows-1; i++)
                    for(unsigned int j = i+start_row+1; j < chunk.cols(); j++)
                        chunk(i, j) = chunk(j - start_row, i + start_row);
            }
        }
    };

    std::vector<MatrixChunk> chunks;
    unsigned int most_chunk_rows;
};

#endif
