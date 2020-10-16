// storage for set of permutations of elements 0...(n-1)
// - set ordered by decreasing swap count needed to order given permutation
//


#ifndef PermTable_H
#  define PermTable_H

#include "sort.h"

template<int n> struct PermData {
   int swaps;
   int arr[n];

   PermData(int sw, const int* dat) : swaps(sw) {
      memcpy(arr, dat, n * sizeof(int));
   }
};//PermData


template<int n> struct PermTable : std::vector< PermData<n> > {
  
   PermTable() {  init();  }

   void init() {
      // generate all permutations, count swaps, store, then sort
      int arr[n];
      for (int i = 0; i < n; i ++) arr[i] = i;
      // use Heap's algorithm (from Wikipedia)
      // - not very elegant but works
      int c[n];
      memset(c, 0, n * sizeof(int));
      this->store(arr);
      for (int i = 0; i < n; ) {
         if (c[i] < i) {
            if ((i & 1) == 0) std::swap(arr[0], arr[i]);
            else std::swap(arr[c[i]], arr[i]);
            this->store(arr);
            c[i] ++;
            i = 0;
         }
         else {
           c[i] = 0;
           i ++;
         }
      }//i
      // sort the array by decreasing swap count
      std::sort(this->begin(), this->end(),
                [](const PermData<n>& a, const PermData<n>& b) { return a.swaps > b.swaps; } );
   }//init

   void store(int* arr) {
      int swaps = sort1(n, arr);
      this->push_back( PermData<n>(swaps, arr) );
   }

};//PermTable


#endif /* ! PermTable_H */


