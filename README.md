# shellsort-13-4-1

Here is some code that finds the worst possible input for (13-4-1) shellsort on 25 elements. This was an undergrad homework problem somewhere in space and time, 
which turns out to be doable afterall, in reasonable O(1 day) runtime, with enough tricks and stuff... and 8 cores. See https://dm248.github.io/posts/20201015_shellsort.html for details on how it works.

**Solvers:**
* *solve.C*: deterministic solver (~1.5 days on a Core i7 using 8 threads, you need to 13-unsort its output by hand and add +12 swaps)
* *find_cand.C*: stochastic algorithm that finds near-optimal solution candidates (finds actual solution too on minute timescale, you just do not know that it is the best ;)

**Design/development steps:**
* *opt4.py*: prototype code for step 2 of the solver
* *z_histogram.C*: computes z histograms = some auxiliary info for design aid (also takes ~1.5 days with 8 threads)
* *y_search.py*: some trivial analysis on the output of z_histogram.C (design aid) 
* *count_swaps.py*: some test cases for counting swaps in h-sort (development aid)  

**Naive "solution":**
* *naive_solver.java*: simple bruteforce attempt that was offerered as solution (only works on day timescale if you can run on 100 billion cores...)
