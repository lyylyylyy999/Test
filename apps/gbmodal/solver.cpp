#include <petscksp.h>
#include <slepceps.h>
#include <petscvec.h>  // For Vec operations
#include <petscmat.h>  // For Mat operations
#include <petscsys.h>
#include <iostream>
#include <vector>
#include <cmath>  // For sqrt and M_PI
#include <Eigen/Dense>
#include <mpi.h>
#include <fstream>
#include <sstream>
#include <string>


PetscErrorCode LoadMatrixFromMTX(const char *filename, Mat *A, MPI_Comm comm)
{
    PetscErrorCode ierr;
    std::ifstream file(filename);
    if (!file.is_open()) {
        SETERRQ1(comm, PETSC_ERR_FILE_OPEN, "Cannot open file: %s", filename);
    }

    std::string line;
    PetscInt m = 0, n = 0, nz = 0;
    PetscInt read_nz = 0;

// 第一遍：跳过注释，读取头部
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '%') continue;
        std::istringstream iss(line);
        if (!(iss >> m >> n >> nz)) {
            SETERRQ1(comm, PETSC_ERR_FILE_READ, "Invalid header in %s", filename);
        }
        break;
    }
    if (m == 0 || n == 0 || nz == 0) {
        SETERRQ1(comm, PETSC_ERR_FILE_READ, "Empty or invalid header in %s", filename);
    }

    // 统计每行 nnz（第一遍扫描数据）
    std::vector<PetscInt> d_nz(m, 0);  // 每行预分配计数
    std::streampos header_pos = file.tellg();  // 记录头部位置，用于第二遍重置
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '%') continue;

        std::istringstream iss(line);
        PetscInt i;
        PetscScalar dummy_v;  // 只需 i 和 j，不需 v
        PetscInt j;
        if (!(iss >> i >> j >> dummy_v)) continue;

        i--;  // 1-based to 0-based
        if (i >= 0 && i < m) {
            d_nz[i]++;  // 计数（忽略 j 越界，实际插入时检查）
        }
        read_nz++;
    }

    // 检查计数
    if (read_nz == 0) {
        SETERRQ1(comm, PETSC_ERR_FILE_READ, "No data read from %s", filename);
    }
    if (read_nz != nz) {
        PetscPrintf(comm, "Warning: Header nz=%D, but scanned %D entries from %s\n", nz, read_nz, filename);
    }

    // 重置文件到数据开始（头部后）
    file.clear();  // 清空 EOF 标志
    file.seekg(header_pos, std::ios::beg);  // 跳过头部，读数据
    std::getline(file, line);  // 跳过头部行本身
    read_nz = 0;  // 重置计数

    // 创建矩阵
    ierr = MatCreateSeqAIJ(PETSC_COMM_SELF, m, n, PETSC_DEFAULT, NULL, A); CHKERRABORT(PETSC_COMM_SELF, ierr);

    // 设置选项（关闭检查，作为备份）
    ierr = MatSetOption(*A, MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE); CHKERRABORT(PETSC_COMM_SELF, ierr);

    // 精确预分配：d_nz 为每行 nnz 数组
    ierr = MatSeqAIJSetPreallocation(*A, 0, d_nz.data()); CHKERRABORT(PETSC_COMM_SELF, ierr);  // 0 表示用 d_nz 数组

    ierr = MatSetFromOptions(*A); CHKERRABORT(PETSC_COMM_SELF, ierr);
    ierr = MatSetUp(*A); CHKERRABORT(PETSC_COMM_SELF, ierr);

    // 第二遍：读取并插入值
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '%') continue;

        std::istringstream iss(line);
        PetscInt i, j;
        PetscScalar v;
        if (!(iss >> i >> j >> v)) continue;

        i--; j--;
        if (i < 0 || j < 0 || i >= m || j >= n) {
            PetscPrintf(comm, "Warning: Skipping invalid index i=%D j=%D\n", i, j);
            continue;
        }

        ierr = MatSetValue(*A, i, j, v, INSERT_VALUES); CHKERRABORT(PETSC_COMM_SELF, ierr);

        // 如果对称且文件只存上三角，取消注释
        // if (i != j) { MatSetValue(*A, j, i, v, INSERT_VALUES); CHKERRABORT(PETSC_COMM_SELF, ierr); }

        if (read_nz % 100000 == 0 && read_nz > 0) {
            PetscPrintf(comm, "Read %D / %D non-zeros...\n", read_nz, nz);
        }
        read_nz++;
    }
    file.close();

    MatAssemblyBegin(*A, MAT_FINAL_ASSEMBLY);
    CHKERRABORT(PETSC_COMM_SELF, ierr);
    MatAssemblyEnd(*A, MAT_FINAL_ASSEMBLY);
    CHKERRABORT(PETSC_COMM_SELF, ierr);

    // 验证
    // if (read_nz == 0) {
    //     SETERRQ1(comm, PETSC_ERR_FILE_READ, "No data read from %s", filename);
    // }
    // if (read_nz != nz) {
    //     PetscPrintf(comm, "Warning: Header nz=%D, but read %D entries from %s\n", nz, read_nz, filename);
    // }
    // PetscPrintf(comm, "Loaded matrix from %s: %D x %D, nnz=%D\n", filename, m, n, read_nz);

    return 0;
}

PetscErrorCode PrintMatrixType(Mat A) {
    MatType type;
    PetscErrorCode ierr;

    ierr = MatGetType(A, &type); CHKERRABORT(PETSC_COMM_SELF, ierr);
    PetscPrintf(PETSC_COMM_WORLD, "Matrix type: %s\n", type);

    return 0;
}

int main(int argc, char **argv) {
        // 初始化 SLEPc
    SlepcInitialize(&argc, &argv, NULL, NULL);
    // // 打印版本信息
    // PetscPrintf(PETSC_COMM_WORLD, "PETSc version: %d.%d.%d\n", PETSC_VERSION_MAJOR, PETSC_VERSION_MINOR, PETSC_VERSION_SUBMINOR);
    // PetscPrintf(PETSC_COMM_WORLD, "SLEPc version: %d.%d.%d\n",
    //             SLEPC_VERSION_MAJOR, SLEPC_VERSION_MINOR, SLEPC_VERSION_SUBMINOR);

    // 控制参数
    Mat A;
    Mat B;
    EPS eps;
    ST st;
    KSP ksp;
    PC pc;
    PetscErrorCode ierr;
    PetscScalar sigma = 0.0;  // 移位值
    PetscReal tol = 1e-6;  // 收敛容限
    PetscInt max_it = 10000;  // 最大迭代次数
    PetscInt neigen = 5;  // 计算的特征值数量

    LoadMatrixFromMTX("./data/box_case3_S_small.mtx", &A, PETSC_COMM_WORLD);
    LoadMatrixFromMTX("./data/box_case3_M_small.mtx", &B, PETSC_COMM_WORLD);

    // // 输出矩阵相关信息
    // // 刚度矩阵
    // PetscInt m, n;
    // MatGetSize(A, &m, &n);
    // PetscPrintf(PETSC_COMM_WORLD, "Matrix loaded for A\n");
    // PrintMatrixType(A);
    // PetscPrintf(PETSC_COMM_WORLD, "Matrix shape: %d x %d\n", m, n);
    // PetscReal norm_A;
    // MatNorm(A, NORM_FROBENIUS, &norm_A);
    // PetscPrintf(PETSC_COMM_WORLD, "Frobenius norm for A = %g\n", (double)norm_A);
    // // 质量矩阵形
    // MatGetSize(B, &m, &n);
    // PetscPrintf(PETSC_COMM_WORLD, "Matrix loaded for B\n");
    // PrintMatrixType(B);
    // PetscPrintf(PETSC_COMM_WORLD, "Matrix shape: %d x %d\n", m, n);
    // PetscReal norm_B;
    // MatNorm(B, NORM_FROBENIUS, &norm_B);
    // PetscPrintf(PETSC_COMM_WORLD, "Frobenius norm for M = %g\n", (double)norm_B);
    // MatInfo infoK, infoM;
    // ierr = MatGetInfo(B, MAT_GLOBAL_SUM, &infoK); CHKERRABORT(PETSC_COMM_SELF, ierr);
    // ierr = MatGetInfo(B, MAT_GLOBAL_SUM, &infoM); CHKERRABORT(PETSC_COMM_SELF, ierr);
    // PetscPrintf(PETSC_COMM_WORLD, "K nnz: %g, M nnz: %g\n", infoK.nz_used, infoM.nz_used);


    // ----------------------------
    // 创建 EPS 特征值求解器
    // ----------------------------
    ierr = EPSCreate(PETSC_COMM_SELF, &eps); CHKERRABORT(PETSC_COMM_SELF, ierr);
    ierr = EPSSetOperators(eps, A, B); CHKERRABORT(PETSC_COMM_SELF, ierr);
    ierr = EPSSetProblemType(eps, EPS_GHEP); // 对称/厄米问题
   // ierr = EPSSetType(eps, EPSLANCZOS);     // Lanczos 方法

   /* 获取 ST 并设为 shift-and-invert */
    ierr = EPSGetST(eps, &st); CHKERRABORT(PETSC_COMM_WORLD, ierr);
    ierr = STSetType(st, STSINVERT); CHKERRABORT(PETSC_COMM_WORLD, ierr);
    ierr = STSetShift(st, sigma); CHKERRABORT(PETSC_COMM_WORLD, ierr);

    /* 从 ST 中取得内部 KSP 来配置线性求解器（解 (A - sigma B) x = b） */
    ierr = STGetKSP(st, &ksp); CHKERRABORT(PETSC_COMM_WORLD, ierr);

    /* 对这个 KSP 使用 preonly（只用 PC 的因子分解）*/
    ierr = KSPSetType(ksp, KSPPREONLY); CHKERRABORT(PETSC_COMM_WORLD, ierr);

    /* 获取 PC 并设置为直接 LU，指定 MUMPS */
    ierr = KSPGetPC(ksp, &pc); CHKERRABORT(PETSC_COMM_WORLD, ierr);
    ierr = PCSetType(pc, PCLU); CHKERRABORT(PETSC_COMM_WORLD, ierr);
    ierr = PCFactorSetMatSolverType(pc, MATSOLVERMUMPS); CHKERRABORT(PETSC_COMM_WORLD, ierr);

    /* 可让 KSP/PC 接受命令行覆盖 */
    ierr = KSPSetFromOptions(ksp); CHKERRABORT(PETSC_COMM_WORLD, ierr);
    ierr = PCSetFromOptions(pc); CHKERRABORT(PETSC_COMM_WORLD, ierr);


    ierr = EPSSetWhichEigenpairs(eps, EPS_LARGEST_REAL);
    ierr = EPSSetDimensions(eps, neigen, PETSC_DEFAULT, PETSC_DEFAULT);
    ierr = EPSSetTolerances(eps, tol, max_it);
    ierr = EPSSetFromOptions(eps); CHKERRABORT(PETSC_COMM_SELF, ierr);

    // ----------------------------
    // 求解特征值
    // ----------------------------
    ierr = EPSSolve(eps); CHKERRABORT(PETSC_COMM_SELF, ierr);

    // 获取收敛信息
    PetscInt nconv;
    ierr = EPSGetConverged(eps, &nconv); CHKERRABORT(PETSC_COMM_SELF, ierr);
    PetscPrintf(PETSC_COMM_SELF, "Number of converged eigenvalues: %d\n", nconv);

    for (PetscInt i = 0; i < nconv; ++i) {
        PetscScalar kr, ki;
        Vec xr, xi;
        ierr = MatCreateVecs(A, &xr, &xi); CHKERRABORT(PETSC_COMM_SELF, ierr);
        ierr = EPSGetEigenpair(eps, i, &kr, &ki, xr, xi); CHKERRABORT(PETSC_COMM_SELF, ierr);
        PetscPrintf(PETSC_COMM_SELF, "Eigenvalue %d: %g\n", i, PetscRealPart(kr));
        ierr = VecDestroy(&xr); CHKERRABORT(PETSC_COMM_SELF, ierr);
        ierr = VecDestroy(&xi); CHKERRABORT(PETSC_COMM_SELF, ierr);
    }

    // ----------------------------
    // 清理
    // ----------------------------
    ierr = EPSDestroy(&eps); CHKERRABORT(PETSC_COMM_SELF, ierr);
    ierr = MatDestroy(&A); CHKERRABORT(PETSC_COMM_SELF, ierr);

    SlepcFinalize();
    return 0;
}

