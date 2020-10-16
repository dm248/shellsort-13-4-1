// finds the worst starting configuration for (13,4,1)-shellsort with N=25 elements
//
// STRATEGY:
//
// i) enumerate all (13,4)-sorted sequences  - O(8*10^12)
//
// ii) undo the 4-sort in all possible ways, and keep only those that are 
//
//     - 13-sorted, and
//
//     - require high enough swap count to order, against a known bad sequence
//
// Enumeration in i) goes by iterative generation of all 4-sorted arrangements, 
// and skipping those that are not 13-sorted. A 4-sort implies that elements 
// [0,4,...24], [1,5,...17], [2,6,...18], [3,7,...19] are separately in sorted 
// order. To generate these, just distribute the 25 elements into groups of 7-6-6-6.
// First generate a group of 6 out of 25, then a group of 6 out of still free 19,
// and finally group of 6 out of remaining 13 (the leftover 7 is the last group):
// 
//    C(25,6) * C(19,6) * C(13,6) = 25! / (6!^3 * 7!)  ~ 8*10^12 possibilities
//
//
// To undo the 4-sort in ii), given a (13,4)-sorted arrangement, generate all
// permutations within the groups of 7-6-6-6 => 7! * 6!^3  ~ 2*10^12 possibilities.
// One can drastically prune this by knowing that the worst case sequence must be
// or must beat
//
//    [18, 24, 5, 11, 17, 23, 4, 10, 16, 22, 3, 9, 12, 15, 21, 2, 8, 14, 20, 1,
//      7, 13, 19, 0, 6]
//
// which takes 12 + 57 + 93 = 162 swaps to order, so among 13-sorted sequences 
// 
//    [15, 21, 2, 8, 14, 20, 1, 7, 13, 19, 0, 6, 12, 18, 24, 5, 11, 17, 23, 4,
//     10, 16, 22, 3, 9]  
// 
// needs y + x = 57 + 93 = 150 swaps. So if our (13,4)-ordered sequence takes x
// swaps to fully order, then require that undoing the 4-sort takes y = (150 - x + 1)
// swaps or more. Counting swaps in 4-sort is trivial, it is the sum of the swaps
// taken within each group 7-6-6-6, which can be precomputed for each of those mini-
// permutations, and mini-permutations can be ordered by decreasing swap count.
// In the nested loop over four minipermutations one can then bail out at any
// level whenever the theoretical upper bound on the swaps necessary to complete
// tehe 4-sort falls below the minimally required value. 
//
// The highest possible swap count in a 4-sort is 66, so we must have x >= 85,
// which also prunes radically the (13,4)-sorted arrangements one needs to
// investigate, from ~8*10^12 down to ~1.6*10^8.
//
// --
//
// SPEEDUPS: 
//
// part i):   - move invariants out of each loop, calculate those in the previous loop
//            - precalculate state in innermost loop
//
// part ii):  - impose minimal swap count to finish (13,4)-sorted arrays
//            - early bailout in undoing 4-sort (drastic pruning) 
//
// generally: - run 8 threads
//

#include <algorithm>
#include <iostream>
#include <cstring>
#include <vector>
#include <thread>
#include <future>

#include "Counter.h"
#include "PermTable.h"
#include "sort.h"


//#define DBG


/*************************
  fast iteration through 4-sorted results
 **************************/


const int period1 = calculatePeriod(25, 6);
const int period2 = calculatePeriod(19, 6);
const int period3 = calculatePeriod(13, 6);


// precompute the 13-element state in loop 3,
// and store the result in an array
// - 0..5 give the positions for arr[4 * i + 3]
// - 6..12 give the positions for arr[4 * i]

template<class T> 
T* precomputeLoop3() {
   CounterFast<13,6> ctr3;
   // allocate
   int period3 = ctr3.getPeriod();
   T* table = new T[13 * period3];

   // C(13,6) sequence, for positions 3, 7, 11, 15, 19, 23
   // remainder fills positions 0, 4, 8, .... 24 (7 locations)

   for (int j3 = 0; j3 < period3; j3 ++) {
      // storage for tracking used elements (used if < 0)
      int used3[13];
      for (int i = 0; i < 13; i ++) used3[i] = i;
      // set idx 0..6
      const int* state3 = ctr3.getState(j3);
      T* row = table + 13 * j3;
      for (int i = 0; i < 6; i ++) {
         row[i] = state3[i];
         used3[state3[i]] = -1;
      }
      // set idx 7..12
      for (int i = 0, pos = 0; i < 7; i ++, pos ++) {
         while (used3[pos] < 0) pos ++;
         row[i + 6] = used3[pos];
      }
   }
   // return it
   return table;
}//precomputeLoop3


// take an array, and store in avail those 'n' elements that are nonnegative
// - ASSUMES that we do have n such elements in used (no range checks)
void setAvail(int n, const int* used, int* avail) {
   for (int i = 0, pos = 0; i < n; i ++, pos ++) {
      while (used[pos] < 0) pos ++;
      avail[i] = used[pos];
   } 
}


// reset the used index array via memcopy
void resetUsed(int n, const int* prevAvail, int* used) {
   memcpy(&(used[0]), &(prevAvail[0]), n * sizeof(int));
}



// do one batch of calculations, for a given outer loop range
// - specified in terms of the initial ctr1, and up to how many increments ctr1 can have

// pack the 25-element array to a 16-byte int (5 bits/element), and also store swaps
typedef unsigned __int128 u128;
typedef std::vector< std::pair<u128,int> > ResultType;  


template <class T>
ResultType doBatch(int roll0, const T* table3, int swapsMIN0, int batchsize) {

   //std::cout << "HERE\n";

   ResultType ret;

   // roll counter 1 to starting value
   Counter<25,6> ctr1;
   for (int i = 0; i < roll0; i ++) ctr1.inc();
   Counter<19,6> ctr2;

   int arr[25];    // partially sorted array we reconstruct
   memset(&(arr[0]), 0, 25 * sizeof(int));
   //short int arr[25];
   //memset(&(arr[0]), 0, 25 * sizeof(short int));

   // for speedup, precalculate available index arrays
   // in outer loops
   int avail1[25];
   int avail2[19];
   int avail3[13];
   int used1[25];
   int used2[19];

   for (int i = 0; i < 25; i ++) avail1[i] = i;


   // 1: C(25,6) sequence, for positions 1, 5, 9, 13, 17, 21

   for (int j1 = 0; j1 < batchsize; j1 ++, ctr1.inc()) {
      if (ctr1.seq[5 - 3] < 4 || ctr1.seq[5 - 4] < 6) { // must have 0,1,2,4,6,8 or later to pass check13
        continue;
      }
      resetUsed(25, avail1, used1);
      // nothing to skip
      for (int i = 0; i < 6; i ++) {
         int pos = ctr1.seq[5 - i];
         arr[4 * i + 1] = pos;
         used1[pos] = -1;
      }
      // - set avail for next loop
      setAvail(19, used1, avail2);


      // 2: C(19,6) sequence, for positions 2, 6, 10, 14, 18, 22

      do {
         resetUsed(19, avail2, used2);
         // use avail2
         for (int i = 0; i < 6; i ++) {
            int pos = ctr2.seq[5 - i];
            arr[4 * i + 2] = avail2[pos];
            used2[pos] = -1;
         }
         // check 13-sortedness
         bool check13 = arr[14] < arr[1] || arr[18] < arr[5] || arr[22] < arr[9];
         if (check13) continue;

         // set avail for next loop
         setAvail(13, used2, avail3);


         // C(13,6) sequence, for positions 3, 7, 11, 15, 19, 23

         for (int j3 = 0; j3 < period3; j3 ++) {  // NOTE: iteration count incr
            // use state table
            auto row = table3 + (13 * j3);
            for (int i = 0; i < 6; i ++) {
               int pos = row[i];
               arr[4 * i + 3] = avail3[pos];
            }
            // check 13-sortedness
            check13 = arr[15] < arr[2] || arr[19] < arr[6] || arr[23] < arr[10];
            if (check13) continue;

            // use second half of row to fill positions 0, 4, 8, .... 24 (7 locations)

            for (int i = 0; i < 7; i ++) {
               int pos = row[6 + i];
               arr[4 * i] = avail3[pos];
            }
            // check 13-sortedness
            check13 = arr[13] < arr[0] || arr[17] < arr[4] || arr[21] < arr[8]
                      || arr[16] < arr[3] || arr[20] < arr[7] || arr[24] < arr[11];
            if (check13) continue;

            // if swap count is high enough, store it
            int swaps = sort1_25(arr);
            if (swaps >= swapsMIN0) {
               u128 v = packArr25(arr); // compress to 16-byte int
               ret.push_back( std::pair<u128,int>(v, swaps) );
            }

         }//j3
      } while(! ctr2.inc());

   } while(! ctr1.inc());

   return ret;
}


// further analyze configuration that passed min swaps cut
//


std::mutex mtx;
u64 totalProcessed = 0;


PermTable<6> permTbl6;
PermTable<7> permTbl7;



void searchResult(const ResultType& ret, int swapsMIN1) {
   int arr[25];
   int iter = 0;
   ResultType good;
   for (auto dat: ret) {
      // unpack array
      unpackArr25(dat.first, arr);
      // undo 4-sort
      int swaps = dat.second;
      int tmp[25];
      for (auto p1: permTbl6) {
         int swap1 = swaps + p1.swaps;
         if (swap1 + 51 <= swapsMIN1) break;
         memcpy(tmp, arr, 25 * sizeof(int));
         for (int i = 0; i < 6; i ++) tmp[4 * i + 1] = arr[4 * p1.arr[i] + 1];
         for (auto p2: permTbl6) {
            int swap2 = swap1 + p2.swaps;
            if (swap2 + 36 <= swapsMIN1) break;
            for (int i = 0; i < 6; i ++) tmp[4 * i + 2] = arr[4 * p2.arr[i] + 2];
            if (tmp[1] > tmp[14] || tmp[5] > tmp[18] || tmp[9] > tmp[22]) continue;
            for (auto p3: permTbl6) {
               int swap3 = swap2 + p3.swaps;
               if (swap3 + 21 <= swapsMIN1) break;
               for (int i = 0; i < 6; i ++) tmp[4 * i + 3] = arr[4 * p3.arr[i] + 3];
               if (tmp[2] > tmp[15] || tmp[6] > tmp[19] || tmp[10] > tmp[23]) continue;
               for (auto p4: permTbl7) {
                  int swap4 = swap3 + p4.swaps;
                  for (int i = 0; i < 7; i ++) tmp[4 * i] = arr[4 * p4.arr[i]];
                  if (tmp[3] > tmp[16] || tmp[7] > tmp[20] || tmp[11] > tmp[24]) continue;
                  if (tmp[0] > tmp[13] || tmp[4] > tmp[17] || tmp[8] > tmp[21]) continue;

                  if (swap4 <= swapsMIN1) break;
                  u128 v = packArr25(tmp); // compress to 16-byte int
                  good.push_back(std::pair<u128,int>(v, swap4) );
               }
            }//p3
         }//p2
      }//p1
   }//v
   // print good results
   mtx.lock();
   for (auto dat: good) {
      unpackArr25(dat.first, arr);
      std::cout << dat.second << ' ';
      printN(arr, 25, 4);
      std::cout << '\n';
   }
   std::cout << std::flush;
   mtx.unlock();
}


// dispatcher loop for a given thread/core


std::atomic<int> ctr0rolls;
//const int Nthreads = 4;
const int Nthreads = 8;


template<class T>
void dispatch(int id, const T* table3, const int swapsMIN[2], int batchsize, int maxRolls) {
   while (true) {
      // grab next job slice
      int nextEnd = (ctr0rolls += batchsize);
      int roll0 = nextEnd - batchsize;
      if (roll0 >= maxRolls) return;
      if (nextEnd > maxRolls) nextEnd = maxRolls;
      int batchReal = nextEnd - roll0;
      mtx.lock();
      std::cout << "#disp" << id << ": roll0=" << roll0 << " +" << batchReal
                << '\n' << std::flush;
      mtx.unlock();
      // run job
      auto fut = std::async(std::launch::async, doBatch<T>,
                            roll0, table3, swapsMIN[0], batchReal);
      // update results
      ResultType res = fut.get();
      mtx.lock();
      std::cout << "#res" << id << ": " << res.size() << "\n";
      totalProcessed += res.size();
      mtx.unlock();
      // search 4-sorts
      searchResult(res, swapsMIN[1]);
   }
}//dispatch



//===MAIN==

int main() {

   std::cout << "#periods: " << period1 << ' ' << period2 << ' ' << period3 << '\n';

   const int swapsMIN[2] = { 85, 149 };
   //const int swapsMIN[2] = { 93, 149 };

   // precalculate counter 3
   //auto table3 = precomputeLoop3<int>();
   //auto table3 = precomputeLoop3<short int>();
   auto table3 = precomputeLoop3<unsigned char>();


   ctr0rolls = 0;

   // run one dispatcher per thread
   //int totalWork = period1 / 1000;
   //int batchsize = 20;
   int totalWork = period1;
   int batchsize = 50;
   // start dispatcher threads
   std::future<void> futs[Nthreads];  // futures in outer scope, so things run concurrently
   for (int i = 0; i < Nthreads; i ++) {
      futs[i] = std::async(std::launch::async, dispatch<unsigned char>,
                           i, table3, swapsMIN, batchsize, totalWork);
   }
   // wait for threads to finish
   for (int i = 0; i < Nthreads; i ++) {
      futs[i].get();
   }

   // print final result
   std::cout << "#BEST: you need to sort the candidates in the printout by hand\n";
   std::cout << "TOTAL: " << totalProcessed << '\n' << std::flush;


   // cleanup
   delete[] table3;

   // ok
   return 0;
}

//EOF

