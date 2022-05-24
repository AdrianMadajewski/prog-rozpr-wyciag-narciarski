#include <iostream>
#include <mpi.h>

#include "Skyer.h"
#include "File.h"

// mpic++ Main.cpp Skyer.cpp Utility.cpp -o skyers -pthread -D DEBUG -Wall -Wextra -std=c++17
// mpirun -np 4 --hostfile mpi_hosts --map-by node skyers weights.txt
// mpirun -np 4 skyers weights.txt

int main(int argc, char **argv)
{
    // Read from file
    if(argc != 2)
    {
        std::cerr << "Usage: mpirun -n {sample_size} {weights_filename}" << std::endl;
        return EXIT_FAILURE;
    }

    std::string filename(argv[1]);
    std::vector<int> skiersWeights = loadDataFromFile(filename);

    int threads;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &threads);
    if(threads != MPI_THREAD_MULTIPLE) {
        std::cerr << "[MPI_Init_thread] Failed - err: Too little threads" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    // Predefines
    MAX_SKI_LIFT = 200;
    MAX_SLEEP_TIME = 10;
    MIN_SLEEP_TIME = 1;

    // Shared
    Skyer skyer;
    skyer.m_data.clock = 0;
    srand(time(nullptr) + skyer.m_data.ID);

    // MPI initialize

    // Initialize global size
    MPI_Comm_size(MPI_COMM_WORLD, &MAX_SKYERS);

    // Initialize process own rank
    MPI_Comm_rank(MPI_COMM_WORLD, &skyer.m_data.ID);

    skyer.m_data.weight = skiersWeights.at(skyer.m_data.ID);

    skyer.mainActivity();

    MPI_Finalize();
    return EXIT_SUCCESS;
}