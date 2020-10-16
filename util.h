// some utility functions

#ifndef util_H
#  define util_H

#include <iostream>


typedef unsigned __int128 u128;


//-- array output, optional extra space after each group of 'gap' elements

void printfN(std::ostream& f, const int* p, int N, int gap = -1) {
   for (int i = 0; i < N; i ++) {
      if (i > 0) f << ',';
      if (gap > 0  && (i % gap) == 0) f << ' ';
      f << p[i];
   }
}

void printN(const int* p, int N, int gap = -1) {   printfN(std::cout, p, N, gap);  }


//-- pack/unpack 25-element array into/from 16-byte int

u128 packArr25(const int* arr) {
   u128 v = 0;
   for (int i = 0; i < 25; i ++) v = (v << 5) + arr[i]; 
   return v;
}

void unpackArr25(u128 v, int* arr) {
   for (int i = 0; i < 25; i ++) {
      arr[24 - i] = v & 0x1f;
      v >>= 5;
   }
}



#endif /* ! util_H */
