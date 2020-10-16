// simple adaptive random generation of bad (13,4,1) sort inputs
// - only keeps one candidate, no pool
//
// highest results:
//
// 158 158 15,24,5,11,14,23,4,10,16,22,3,9,13,18,21,2,7,17,20,1,8,12,19,0,6
// 159 159 18,24,15,5,17,23,13,4,16,22,11,3,6,9,21,14,2,8,20,12,1,7,19,10,0
// 160 18,24,8,11,17,23,3,10,16,22,2,9,12,15,21,7,6,14,20,1,5,13,19,0,4
// 162 162 18,24,5,11,17,23,4,10,16,22,3,9,12,15,21,2,8,14,20,1,7,13,19,0,6


#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "sort.h"


typedef unsigned long int u64;


void generate(int* arr) {
   // start from ordered state
   for (int i = 0; i < 25; i ++) arr[i] = i;
   // randomize by swapping randomly chosen elements to end
   for (int i = 0; i < 24; i ++) {
      int j = rand() % (25 - i); // very slightly biased (rand range is 2^n)
      std::swap(arr[j], arr[24 - i]);
   }
}

// maximize the h-13 swaps
void max13(int* arr) {
   for (int i = 0; i < 12; i ++) {
      if (arr[i] < arr[i + 13]) std::swap(arr[i], arr[i + 13]);
   }
}

void show(const int* arr) {
   for (int i = 0; i < 25; i ++) {
      if (i > 0) std::cout << ',';
      std::cout << arr[i];
   }
   std::cout << '\n' << std::flush;
}

// do a mutation chain, return the number of (13,4,1) sort swaps
// -1 if no valid 13-sorted mutation found
int mutate(int* arr, int Nmutate, int swapsMIN) {
   for (int j = 0; j < Nmutate; j ++) {
      int i1 = rand() % 25;
      int i2 = rand() % 24;
      if (i2 >= i1) i2 ++;
      std::swap(arr[i1], arr[i2]);
      max13(arr);
      int swaps = sort13_4_1_25(arr);
      if (swaps > swapsMIN) return swaps;
   }
   return -1;
}

void arrcpy25(int* dst, const int* src) {
   memcpy(dst, src, 25 * sizeof(int));
}

// search
int search(int* bestArr, u64 tries, int Nmutate) {
   // initial candidate
   for (int i = 0; i < 25; i ++) bestArr[i] = i;
   // iterate
   int best = -1;
   int arr[25];
   for (u64 i = 0; i < tries; i ++) {
      // mutate the best
      arrcpy25(arr, bestArr);
      int swaps = mutate(arr, Nmutate, best);
      // if did not improve, generate a new one and mutate that
      if (swaps <= best) {
         generate(arr);
         max13(arr);
         swaps = sort13_4_1_25(arr);
         if (swaps <= best) swaps = mutate(arr, Nmutate, best);
      }
      // check for improvement either way
      if (swaps > best) {
         best = swaps;
         arrcpy25(bestArr, arr);
      }
   }

   return best;
}//search


// == MAIN 

int main(const int argc, const char** const argv) {

   srand(time(0));  // init rng

   // cline params
   u64 tries   = (argc > 1) ? atoll(argv[1]) : 100000;
   int Nmutate = (argc > 2) ? atoi(argv[2])  : 10;

   // infinite loop retries
   int bestArr[25];
   while (true) {
      int best = search(bestArr, tries, Nmutate);
      int swaps = sort13_4_1_25(bestArr);
      std::cout << best  << ' ' << swaps << ' ';
      show(bestArr);
   }

   // OK
   return 0;
}




//EOF


