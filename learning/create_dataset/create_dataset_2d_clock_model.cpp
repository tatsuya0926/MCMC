#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <algorithm>
const double pi = 3.141592653589793;
const long int monte_carlo_step = 100000;
const int L = 64;
const int nx = L; // number of sites along x-direction
const int ny = L; // number of sites along y-direction
const int niter = monte_carlo_step * nx * ny;
// const int Q = 4;
const int Q = 6;
const double coupling_J = 1.0;
// const int nconf = 50;
const int nconf = 80;
const int ndata = 1000;
// const double t_start = 0.9;
const double t_start = 0.4;
const int nskip = nx * ny * 100; // Frequency of measurement
const int nconfig = 0;

double calc_action_change(const int spin[nx][ny], const int next_spin, const double coupling_J, const double temperature, const int ix, const int iy)
{
    double action_change = 0e0;
    // double sum_change=0;

    int ixp1 = (ix + 1) % nx;      // ixp1=ix+1;
    int iyp1 = (iy + 1) % ny;      // iyp1=iy+1;
    int ixm1 = (ix - 1 + nx) % nx; // ixm1=ix-1;
    int iym1 = (iy - 1 + ny) % ny; // iym1=iy-1;

    double sum_change = std::cos(2 * pi * spin[ix][iy] / Q - 2 * pi * spin[ixp1][iy] / Q) +
                        std::cos(2 * pi * spin[ix][iy] / Q - 2 * pi * spin[ix][iyp1] / Q) +
                        std::cos(2 * pi * spin[ix][iy] / Q - 2 * pi * spin[ixm1][iy] / Q) +
                        std::cos(2 * pi * spin[ix][iy] / Q - 2 * pi * spin[ix][iym1] / Q) -
                        std::cos(2 * pi * next_spin / Q - 2 * pi * spin[ixp1][iy] / Q) -
                        std::cos(2 * pi * next_spin / Q - 2 * pi * spin[ix][iyp1] / Q) -
                        std::cos(2 * pi * next_spin / Q - 2 * pi * spin[ixm1][iy] / Q) -
                        std::cos(2 * pi * next_spin / Q - 2 * pi * spin[ix][iym1] / Q);

    action_change = sum_change * coupling_J / temperature;

    return action_change;
}

int main()
{
    double temperature[nconf + 1];
    double sum = t_start;
    for (int i = 0; i < nconf + 1; i++)
    {
        temperature[i] = sum;
        sum += 0.01;
        std::cout << temperature[i] << std::endl;
    }
    for (int conf = 0; conf < nconf + 1; conf++)
    {
        double T = temperature[conf];
        int data_num = 0;
        int spin[nx][ny];
        srand((unsigned)time(NULL));
        // 初期化
        if (nconfig == 0)
        {
            std::ifstream inputconfig("input/2d_Clock_q=" + std::to_string(Q) + "_output_config.txt");
            if (!inputconfig)
            {
                std::cout << "inputfile not found" << std::endl;
                exit(1);
            }
            for (int ix = 0; ix != nx; ix++)
            {
                for (int iy = 0; iy != ny; iy++)
                {
                    inputconfig >> ix >> iy >> spin[ix][iy];
                }
            }
            inputconfig.close();
        }
        // 各温度でモンテカルロシミュレーション
        for (long int iter = 0; iter != niter; iter++)
        {
            double rand_site = (double)rand() / RAND_MAX;
            rand_site = rand_site * nx * ny;
            int ix = (int)rand_site / ny;
            int iy = (int)rand_site % ny;
            double metropolis = (double)rand() / RAND_MAX;
            int next_spin = rand() % Q;
            double action_change = calc_action_change(spin, next_spin, coupling_J, T, ix, iy);
            if (exp(-action_change) > metropolis)
            {
                // accept
                spin[ix][iy] = next_spin; // flip
            }
            else
            {
                // reject
            }

            if (iter > 1000 * nx * ny && (iter + 1) % nskip == 0 && data_num < ndata)
            {
                std::ofstream outputfile("../txtfile/2d_Clock/q=" + std::to_string(Q) + "/L" + std::to_string(L) + "T" + std::to_string(conf) + "_" + std::to_string(data_num + ndata) + ".txt");
                for (int ix = 0; ix != nx; ix++)
                {
                    for (int iy = 0; iy != ny; iy++)
                    {
                        outputfile << ix << ' ' << iy << ' ' << spin[ix][iy] << ' ' << std::endl;
                    }
                }
                outputfile.close();
                data_num++;
            }
        }
    }
    return 0;
}