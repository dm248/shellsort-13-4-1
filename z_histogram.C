// Calculates the FREQUENCY DISTRIBUTION of swaps made in the final 1-sort on all
// (13,4)-sorted inputs of n=25 elements
//
// The main result is a comma-separated list:
//
//  - the first number is the number of 4-ordered arrangements that are NOT 13-ordered
//  - numbers at position z+1 give the number of (13,4)-ordered arrangements that
//    need exactly z swaps to 1-sort
//  - 120 numbers are printed, so 0 <= z <= 119 (highest possible is 108, actually)
// 
// --
//
// The code iteratively generates all arrangements of 0...24 that are both 
// 13- and 4-ordered. 
//
// It generates 4-ordered ones first, then prunes those that are not 13-ordered.
// 4-sort implies elements [0,4,...24], [1,5,...17], [2,6,...18], [3,7,...19] are
// separately in sorted order. To generate these, we just need to distribute the 25
// elements into groups of 7-6-6-6.
//
// First generate group of 6 out of 25, then group of 6 out of still free 19,
// finally group of 6 out of remaining 13 (the leftover 7 is the last group).
// 
// C(25,6) * C(19,6) * C(13,6) = 25! / (6!^3 * 7!)  ~ 8*10^12
//
// --
//
//
// SPEEDUPS: - move invariants out of each loop, calculate those in the previous loop
//           - precalculate state in innermost loop
//           - run 8 threads
//

#include <algorithm>
#include <iostream>
#include <cstring>
#include <vector>
#include <thread>
#include <future>

#include "Counter.h"
#include "sort.h"

//#define DBG


//--- fast iteration through h=4 shellsort results
//

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

const int ResTypeSIZE = 120;
typedef std::vector<u64> ResultType;  // freq distribution


template <class T>
ResultType doBatch(int roll0, const T* table3, int batchsize) {

   ResultType ret(ResTypeSIZE);

   // roll counter 1 to starting value
   Counter<25,6> ctr1;
   for (int i = 0; i < roll0; i ++) ctr1.inc();
   Counter<19,6> ctr2;

   int arr[25];    // partially sorted array we reconstruct
   memset(&(arr[0]), 0, 25 * sizeof(int));

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
        ret[0] += period2 * period3;
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
         if (check13) {
            ret[0] += period3;
            continue;
         }
         // set avail for next loop
         setAvail(13, used2, avail3);


         // C(13,6) sequence, for positions 3, 7, 11, 15, 19, 23

         for (int j3 = 0; j3 < period3; j3 ++) {
            int entry = 0;
            // use state table
            auto row = table3 + (13 * j3);
            for (int i = 0; i < 6; i ++) {
               int pos = row[i];
               arr[4 * i + 3] = avail3[pos];
            }
            // check 13-sortedness
            check13 = arr[15] < arr[2] || arr[19] < arr[6] || arr[23] < arr[10];
            if (check13) goto cont;

            // use second half of row to fill positions 0, 4, 8, .... 24 (7 locations)

            for (int i = 0; i < 7; i ++) {
               int pos = row[6 + i];
               arr[4 * i] = avail3[pos];
            }
            // check 13-sortedness
            check13 = arr[13] < arr[0] || arr[17] < arr[4] || arr[21] < arr[8]
                      || arr[16] < arr[3] || arr[20] < arr[7] || arr[24] < arr[11];
            if (check13) goto cont;

            entry = 1 + sort1_25(arr);  // swaps + 1

cont:
            ret[entry] ++;

         }//j3
      } while(! ctr2.inc());

   } while(! ctr1.inc());

   return ret;
}



//-- show result of a batch calculations

// decode the initial 4-sorted array from its iteration index
void decodeArray(u64 iter, int* arr) {
   // spin the counters to the given iteration - FIXME: not optimal but workable
   int j3 = iter % period3;
   int j2 = (iter / period3) % period2;
   int j1 = iter / (u64(period2) * period3);

   Counter<25,6> ctr1;
   for (int i = 0; i < j1; i ++) ctr1.inc();
   Counter<19,6> ctr2;
   for (int i = 0; i < j2; i ++) ctr2.inc();
   Counter<13,6> ctr3;
   for (int i = 0; i < j3; i ++) ctr3.inc();
   // create the array
   int avail[25];
   for (int i = 0; i < 25; i ++) avail[i] = i;
   for (int i = 0; i < 6; i ++) {
      int pos = ctr1.seq[5 - i];
      arr[4 * i + 1] = pos;
      avail[pos] = -1;
   }
   for (int i = 0, pos = 0, prevIdx = -1; i < 6; i ++, pos ++) {
      int delta = ctr2.seq[5 - i] - prevIdx; // increase in ctr index compared to prev idx
      prevIdx = ctr2.seq[5 - i];
      while (true) {
         while (avail[pos] < 0) pos ++;  // next available position
         delta --;
         if (delta == 0) break;
         pos ++;
      }
      arr[4 * i + 2] = pos;
      avail[pos] = -1;
   }
   for (int i = 0, pos = 0, prevIdx = -1; i < 6; i ++, pos ++) {
      int delta = ctr3.seq[5 - i] - prevIdx; // increase in ctr index compared to prev idx
      prevIdx = ctr3.seq[5 - i];
      while (true) {
         while (avail[pos] < 0) pos ++;  // next available position
         delta --;
         if (delta == 0) break;
         pos ++;
      }
      arr[4 * i + 3] = pos;
      avail[pos] = -1;
   }
   for (int i = 0, pos = 0; i < 7; i ++, pos ++) {
      while (avail[pos] < 0) pos ++;
      arr[4 * i] = pos;
   }
}//decodeArray


void showResult(const ResultType& ret) {
   for (unsigned int i = 0; i < ret.size(); i ++) {
      if (i > 0) std::cout << ',';
      std::cout << ret[i];
   }
   std::cout << '\n' << std::flush;
}


std::atomic<int> ctr0rolls;
//const int Nthreads = 4;
const int Nthreads = 8;

// freq distribution, shifted by one - 0th element counts not 13-ordered entries
ResultType freqDist(ResTypeSIZE);
std::mutex mtx;

template<class T>
void dispatch(int id, const T* table3, int batchsize, int maxRolls) {
   int iter = 0;
   while (true) {
      // grab next job slice
      int nextEnd = (ctr0rolls += batchsize);
      int roll0 = nextEnd - batchsize;
      if (roll0 >= maxRolls) return;
      if (nextEnd > maxRolls) nextEnd = maxRolls;
      int batchReal = nextEnd - roll0;
      mtx.lock();
      std::cout << "#disp" << id << ": roll0=" << roll0 << " +" << batchReal
                << " iter=" << iter << '\n' << std::flush;
      mtx.unlock();
      // run job
      auto fut = std::async(std::launch::async, doBatch<T>,
                            roll0, table3, batchReal);
      // update results
      ResultType res = fut.get();
      iter ++;
      mtx.lock();
      for (unsigned int i = 0; i < res.size(); i ++) freqDist[i] += res[i];
      if (iter % 10 == 0) {
         std::cout << "#res" << id << "\n";
         showResult(res);
         std::cout << std::flush;
      }
      mtx.unlock();
   }
}//dispatch



//===MAIN==

int main() {

   std::cout << "#periods: " << period1 << ' ' << period2 << ' ' << period3 << '\n';

   // precalculate counter 3 - unsigned char is fastest (by few %)
   //auto table3 = precomputeLoop3<int>();
   //auto table3 = precomputeLoop3<short int>();
   auto table3 = precomputeLoop3<unsigned char>();

   // initialize outer loop counter
   Counter<25,6> ctr0;
   // roll it until 0,1,2,4,6,.., track counter rolls
   int rolls0 = 0;
   while(true) {
      if (ctr0.seq[5 - 3] >= 4 &&  ctr0.seq[5 - 4] >= 6) break;
      ctr0.inc();
      rolls0 ++;
   }
   ctr0rolls = rolls0;

   // run one batch per thread
   //int totalWork = (period1 / 1000) + ctr0rolls; // reduced problem for testing
   //int batchsize = 20;
   int totalWork = period1;
   int batchsize = 100;
   // start dispatcher threads
   std::future<void> futs[Nthreads];  // futures in outer scope, so things run concurrently
   for (int i = 0; i < Nthreads; i ++) {
      futs[i] = std::async(std::launch::async, dispatch<unsigned char>,
                           i, table3, batchsize, totalWork);
   }
   // wait for threads to finish
   for (int i = 0; i < Nthreads; i ++) {
      futs[i].get();
   }

   // print final result
   std::cout << "FREQ DIST: ";
   showResult(freqDist);
   u64 tot = 0;
   for (unsigned int i = 0; i < freqDist.size(); i ++) tot += freqDist[i];
   std::cout << "TOTAL: " << tot << '\n' << std::flush;


   // cleanup
   delete[] table3;

   // ok
   return 0;
}

//EOF
