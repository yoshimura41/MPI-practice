/*
    Modified Eratosthenes Algorithm
    - Delete Even Integers
    - Eliminate Broadcast
    - Reorganize Loops
*/

#include"mpi.h"
#include<iostream>
#include<cmath>
#include<vector>
using namespace std;
using ll=long long;

constexpr ll BLOCK_LOW(int id, int p, ll n) { return id*((n-1)/2)/p*2; }
constexpr ll BLOCK_HIGH(int id, int p, ll n) { return BLOCK_LOW(id+1, p, n)-1; }
constexpr ll BLOCK_SIZE(int id, int p, ll n) { return BLOCK_LOW(id+1, p, n)-BLOCK_LOW(id, p, n); }

int main(int argc, char* argv[]) {
    ll count;
    double elapsed_time;
    ll global_count;
    ll high_value;
    ll i;
    int id;
    ll low_value;
    ll n;
    int proc;
    ll proc0_size;
    ll prime;
    ll size;
    vector<ll> primes_before_sqrt_n;
    vector<ll> primes;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &proc);

    if(!id) {
        cout << "Enter the value of n: ";
        cin >> n;
    }

    MPI_Bcast(&n, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();

    for(prime = 3; prime * prime <= n; prime += 2) {
        bool is_prime = true;
        for(auto &p: primes_before_sqrt_n) {
            if(prime % p == 0){
                is_prime = false;
                break;
            }
        }
        if(is_prime) {
            primes_before_sqrt_n.push_back(prime);
        }
    }

    low_value = 3 + BLOCK_LOW(id, proc, n);
    high_value = 3 + BLOCK_HIGH(id, proc, n);
    size = BLOCK_SIZE(id, proc, n);

    cout << "id: " << id << ", low_value: " << low_value << ", high_value: " << high_value << ", size: " << size << endl;

    proc0_size = (n-1)/proc;

    if ((2 + proc0_size) < (int) sqrt((double) n)) {
        if (!id) cout << "Too many processes\n";
        MPI_Finalize();
        exit(1);
    }

    count = 0;
    for (i = low_value; i <= high_value; i += 2) {
        bool is_prime = true;
        for(auto &p: primes_before_sqrt_n) {
            if(i == p)break;
            if(i % p == 0){
                is_prime = false;
                break;
            }
        }
        if (is_prime) {
            primes.push_back(i);
            count++;
        }
    }

    MPI_Reduce(&count, &global_count, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    elapsed_time += MPI_Wtime();

    if (!id) {
        cout << global_count + 1 << " primes are less than or equal to " << n << endl;
        cout << "Total elapsed time: " << elapsed_time << " seconds" << endl;
    }

    MPI_Finalize();
    return 0;
}