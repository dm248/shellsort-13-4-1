# shellsort-13-4-1

Here is some code that finds the worst possible input for (13-4-1) shellsort on 25 elements. This was an undergrad homework problem somewhere in space and time... 
which turns out to be doable afterall, in reasonable O(1 day) runtime, with enough tricks and stuff.. and 8 cores. See https://dm248.github.io/posts/20201015_shellsort.html for details on how it works.

* solve.C: deterministic solver (takes ~1.5 days on a Core i7, full 8 cores)
* find_cand.C: stochastic alg that finds near-optimal solution candidates (finds actual solution too on minute timescale, you just do not know that it is the best solution ;) 
* z_histogram.C: computes z histograms = some auxiliary info for design aid (also takes ~1.5 days on 8 cores)

* opt4.py: prototype code for step 2 of the solver
* y_search.py: some trivial analysis on the output of z_histogram.C (design aid) 
* count_swaps.py: some test cases for counting swaps in h-sort (development aid)

* naive_solver.java: naive bruteforce attempt (only works on day timescale if you can run on 100 billion cores...)
