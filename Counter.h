// counter to iterate through a C(n,k) combination (= ordered set of k distinct elements out of n)
//


#ifndef Counter_H
#  define Counter_H

#include "util.h"


typedef unsigned long long int u64;


//period calculator - FIXME: no overflow protection
u64 calculatePeriod(int n, int k) {
   u64 ret = 1;
   for (int i = 0; i < k; i ++) ret *= n - i;
   for (int i = 0; i < k; i ++) ret /= i + 1;
   return ret;
}

// regular version of counter
template<int n, int k> struct Counter {
   int seq[k];  // in-order sequence, strictly DECREASING

   Counter() {  init();   }

   void init() {  
      for (int i = 0; i < k; i ++) seq[i] = k - i - 1;
   }

   bool inc() {
      for (int i = 0; i < k; i ++) {
         seq[i] ++;
         if (seq[i] < n - i)  {
            for (; i > 0; i --) seq[i - 1] = seq[i] + 1;  // if good, initialize tail
            return false;  // no rollover yet
         }
      }
      // upon rollover reinitialize
      init();
      return true;
   }

   u64 getPeriod() const { return calculatePeriod(n, k); }

   void print(std::ostream& f) const {
      printfN(f, seq, k);
      f << '\n';
   } 
};//Counter



// fast version of counter that pulls up state from a precomputed array
template<int n, int k> struct CounterFast {
   int period;
   int* idxsArray;

   CounterFast() {
      Counter<n,k> ctr;
      period = ctr.getPeriod();
      idxsArray = new int[period * k];
      // fill state array, invert element order too j -> (k-1)-j
      for (int i = 0; i < period; i ++) {
         for (int j = 0; j < k; j ++) idxsArray[k * i + j] = ctr.seq[(k-1) - j];
         ctr.inc();
      }
   }

   ~CounterFast() {   delete[] idxsArray; }

   const int* getState(int i) const {  return idxsArray + (k * i); }

   int getPeriod() const { return period; }

};//CounterFast


#endif /* ! Counter_H */



