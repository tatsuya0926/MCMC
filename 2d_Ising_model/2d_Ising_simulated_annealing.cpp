#include <iostream>
#include <cmath>
#include <fstream>
const long int niter = 1000000;
const long int step_length = 1000;
const int nx = 64; // number of sites along x-direction
const int ny = 64; // number of sites along y-direction
const double coupling_J = 1e0;
const double coupling_h = 0.1e0;
const double cooling_rate = 0.995;
const int nskip = 10000; // Frequency of measurement
const int nconfig = 1;   // 0 -> read 'input_config.txt'; 1 -> all up; -1 -> all down
/*********************************/
/*** Calculation of the action ***/
/*********************************/
double calc_action(const int spin[nx][ny], const double coupling_J, const double coupling_h, const double temperature)
{
    double action = 0.0;
    int sum1 = 0;
    int sum2 = 0;
    for (int ix = 0; ix != nx; ix++)
    {
        int ixp1 = (ix + 1) % nx; // ixp1=ix+1; be careful about the boundary condition.
        for (int iy = 0; iy != ny; iy++)
        {
            int iyp1 = (iy + 1) % ny; // iyp1=iy+1; be careful about the boundary condition.
            sum1 = sum1 + spin[ix][iy];
            sum2 = sum2 + spin[ix][iy] * spin[ixp1][iy] + spin[ix][iy] * spin[ix][iyp1];
        }
    }
    action = (sum2 * coupling_J + sum1 * coupling_h) / temperature * (-1e0);

    return action;
}
/***********************************************/
/*** Calculation of the change of the action ***/
/***  when the spin at (ix,iy) is flipped    ***/
/***********************************************/
double calc_action_change(const int spin[nx][ny], const double coupling_J, const double coupling_h, const double temperature, const int ix, const int iy)
{
    double action_change = 0e0;
    // int sum1_change=0;
    // int sum2_change=0;

    int ixp1 = (ix + 1) % nx;      // ixp1=ix+1; be careful about the boundary condition.
    int iyp1 = (iy + 1) % ny;      // iyp1=iy+1; be careful about the boundary condition.
    int ixm1 = (ix - 1 + nx) % nx; // ixm1=ix-1; be careful about the boundary condition.
    int iym1 = (iy - 1 + ny) % ny; // iym1=iy-1; be careful about the boundary condition.

    int sum1_change = 2 * spin[ix][iy];
    int sum2_change = 2 * spin[ix][iy] * spin[ixp1][iy] + 2 * spin[ix][iy] * spin[ix][iyp1] + 2 * spin[ix][iy] * spin[ixm1][iy] + 2 * spin[ix][iy] * spin[ix][iym1];

    action_change = (sum2_change * coupling_J + sum1_change * coupling_h) / temperature;

    return action_change;
}
/*************************************/
/*** Calculation of the total spin ***/
/*************************************/
int calc_total_spin(const int spin[nx][ny])
{
    int total_spin = 0;
    for (int ix = 0; ix != nx; ix++)
    {
        for (int iy = 0; iy != ny; iy++)
        {
            total_spin = total_spin + spin[ix][iy];
        }
    }
    return total_spin;
}

int calc_total_plus_spin(const int spin[nx][ny])
{
    int total_spin = 0;
    for (int ix = 0; ix != nx; ix++)
    {
        for (int iy = 0; iy != ny; iy++)
        {
            if (spin[ix][iy] == 1)
            {
                total_spin++;
            }
        }
    }
    return total_spin;
}

int main()
{
    int spin[nx][ny];
    double temperature = 5.0;
    srand((unsigned)time(NULL));
    /*********************************/
    /********* 初期状態の決定 ********/
    /*********************************/
    if (nconfig == 1)
    {
        for (int ix = 0; ix != nx; ix++)
        {
            for (int iy = 0; iy != ny; iy++)
            {
                spin[ix][iy] = 1;
            }
        }
    }
    if (nconfig == -1)
    {
        for (int ix = 0; ix != nx; ix++)
        {
            for (int iy = 0; iy != ny; iy++)
            {
                spin[ix][iy] = -1;
            }
        }
    }
    if (nconfig == 0)
    {
        std::ifstream inputconfig("input_config.txt");
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
    /***********************************/
    /******* simulated annealing *******/
    /***********************************/
    std::ofstream outputfile("output/2d_Ising_simulated_annealing_output.txt");

    for (long int s = 0; s < step_length; s++)
    {
        int naccept = 0; //受理した合計数
        for (long int iter = 0; iter != niter; iter++)
        {
            double rand_site = (double)rand() / RAND_MAX;
            rand_site = rand_site * nx * ny;
            int ix = (int)rand_site / ny;
            int iy = (int)rand_site % ny;
            double metropolis = (double)rand() / RAND_MAX;
            double action_change = calc_action_change(spin, coupling_J, coupling_h, temperature, ix, iy);
            if (exp(-action_change) > metropolis)
            {
                // accept
                spin[ix][iy] = -spin[ix][iy];
                naccept = naccept + 1;
            }
            else
            {
                // reject
            }
        }
        /*******************/
        /*** data output ***/
        /*******************/
        int total_spin = calc_total_spin(spin);
        int total_plus_spin = calc_total_plus_spin(spin);
        double energy = calc_action(spin, coupling_J, coupling_h, temperature) * temperature;
        std::cout << std::fixed << std::setprecision(4)
                  << total_spin << "   "
                  << total_plus_spin << "   "
                  << energy << "   "
                  << temperature << "   "
                  << (double)naccept / niter << std::endl;
        outputfile << std::fixed << std::setprecision(4)
                   << total_spin << "   "
                   << total_plus_spin << "   "
                   << energy << "   "
                   << temperature << "   "
                   << (double)naccept / niter << std::endl;
        temperature = cooling_rate * temperature;
    }
    outputfile.close();
    /*************************/
    /*** save final config ***/
    /*************************/
    std::ofstream outputconfig("output/2d_Ising_simulated_annealing_output_config.txt");
    for (int ix = 0; ix != nx; ix++)
    {
        for (int iy = 0; iy != ny; iy++)
        {
            outputconfig << ix << ' ' << iy << ' ' << spin[ix][iy] << ' ' << std::endl;
        }
    }
    outputconfig.close();
    return 0;
}