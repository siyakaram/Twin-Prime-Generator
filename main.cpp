#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <gmpxx.h>

using namespace std;
using namespace std::chrono;

mutex file_mutex;
vector<mpz_class> valid_a_values;
vector<mpz_class> valid_b_values;

void generate_valid_ab_values(int limit = 10000000) {
    vector<bool> is_prime(limit + 1, true);
    is_prime[0] = is_prime[1] = false;

    for (int i = 2; i * i <= limit; ++i) {
        if (is_prime[i]) {
            for (int j = i * i; j <= limit; j += i)
                is_prime[j] = false;
        }
    }

    for (int i = 3; i <= limit; i += 2) {
        if (is_prime[i]) {
            int a_or_b = (i - 1) / 2;
            valid_a_values.push_back(mpz_class(a_or_b));
            valid_b_values.push_back(mpz_class(a_or_b));
        }
    }
}

bool passes_mod_conditions(const mpz_class& n_class) {
    mpz_class p_plus = 2 * n_class + 1;
    mpz_class root_p_plus;
    mpz_sqrt(root_p_plus.get_mpz_t(), p_plus.get_mpz_t());
    mpz_class upper_a = (root_p_plus - 1) / 2;

    for (const auto& a : valid_a_values) {
        if (a > upper_a) break;
        mpz_class mod = 2 * a + 1;
        if (n_class % mod == a) return false;
    }

    mpz_class p_minus = 2 * n_class - 1;
    mpz_class root_p_minus;
    mpz_sqrt(root_p_minus.get_mpz_t(), p_minus.get_mpz_t());
    mpz_class upper_b = (root_p_minus - 1) / 2;

    for (const auto& b : valid_b_values) {
        if (b > upper_b) break;
        mpz_class mod = 2 * b + 1;
        if ((n_class - 1) % mod == b) return false;
    }

    return true;
}

bool is_prime(const mpz_class& num, int reps = 25) {
    return mpz_probab_prime_p(num.get_mpz_t(), reps) >= 1;
}

vector<pair<mpz_class, mpz_class>> split_range(mpz_class n_min, mpz_class n_max, int num_threads) {
    vector<pair<mpz_class, mpz_class>> ranges;
    mpz_class total = n_max - n_min;
    mpz_class chunk = total / num_threads;
    mpz_class start = n_min;

    for (int i = 0; i < num_threads; ++i) {
        mpz_class end = (i == num_threads - 1) ? n_max : (start + chunk);
        ranges.push_back({start, end});
        start = end;
    }

    return ranges;
}

void worker(mpz_class start, mpz_class end, int tid) {
    const mpz_class threshold("200000020000000");
    mpz_class current = start;
    mpz_class one = 1;
    vector<string> buffer;

    while (current < end) {
        if (passes_mod_conditions(current)) {
            mpz_class p1 = 2 * current - 1;
            mpz_class p2 = 2 * current + 1;

            bool is_valid_pair = true;

            if (current >= threshold) {
                if (!is_prime(p1) || !is_prime(p2)) {
                    is_valid_pair = false;
                }
            }

            if (is_valid_pair) {
                buffer.push_back(p1.get_str() + ", " + p2.get_str() + "\n");
            }
        }
        current += one;
    }

    if (!buffer.empty()) {
        lock_guard<mutex> guard(file_mutex);
        ofstream fout("twin_primes.txt", ios::app);
        for (const auto& line : buffer)
            fout << line;
        fout.close();
    }
}

int main() {
    string s_min, s_max;
    cout << "Enter value of n_min (30+ digits allowed): ";
    cin >> s_min;
    cout << "Enter value of n_max: ";
    cin >> s_max;

    mpz_class n_min(s_min), n_max(s_max);
    if (n_min < 2 || n_max <= n_min) {
        cerr << "Invalid range. n_min must be >= 2 and less than n_max." << endl;
        return 1;
    }

    generate_valid_ab_values();

    ofstream fout("twin_primes.txt", ios::trunc);
    fout.close();

    int num_threads = thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;

    auto start_time = high_resolution_clock::now();

    vector<pair<mpz_class, mpz_class>> ranges = split_range(n_min, n_max, num_threads);
    vector<thread> threads;

    cout << "Starting search with " << num_threads << " threads...\n";

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker, ranges[i].first, ranges[i].second, i);
    }

    for (auto& t : threads) t.join();

    auto end_time = high_resolution_clock::now();

    // Microseconds duration
    auto elapsed_us = duration_cast<microseconds>(end_time - start_time);

    // Convert to seconds with fractional part
    double elapsed_seconds = elapsed_us.count() / 1'000'000.0;

    cout << "Output saved to: twin_primes.txt" << endl;
    cout << fixed << setprecision(6) << "Execution time: " << elapsed_seconds << " seconds" << endl;

    return 0;
}
