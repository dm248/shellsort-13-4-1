//-- shellsort & swap counting
//

#ifndef sort_H
#  define sort_H

// verify that a sequence is sorted
bool sorted(int n, const int* arr) {
   for (int i = 1; i < n; i ++) {
      if (arr[i - 1] > arr[i]) return false;
   }
   return true;
}

bool sorted25(const int* arr) { return sorted(25, arr); }

// count swaps in normal insertion sort
// - leaves arr unchanged
int sort1(int n, const int* arr) {
   int swaps = 0;
   int tmp[n];   // temporary for sorted result
   tmp[0] = arr[0];
   for (int i = 1; i < n; i ++) {
      int j;
      for (j = i - 1; j >= 0; j --) {
         if (arr[i] >= tmp[j]) break;
         tmp[j + 1] = tmp[j];  // defer writing arr[i] until we have its final position
         swaps ++;
      }
      tmp[j + 1] = arr[i];  // write arr[i] to its place
   }//i
   return swaps;
}

int sort1_25(const int* arr) {  return sort1(25, arr);  }



// count swaps in h-sort
// - h-sorts the *original* array
// - to do the sort from a nonzero offset, shift arr and reduce n
int sortH(int n, int* arr, int h) {
   int swaps = 0;
   for (int i = h; i < n; i += h) {
      int v = arr[i];
      int j;
      for (j = i - h; j >= 0; j -= h) {
         if (v >= arr[j]) break;
         arr[j + h] = arr[j];  // defer writing arr[i] until we have its final position
         swaps ++;
      }
      arr[j + h] = v;  // write arr[i] to its place
   }//i
   return swaps;
}

// count swaps in (13,4,1) shellsort
// - leaves original array intact
int sort13_4_1(int n, const int* arr) {
   int tmp[n];
   memcpy(tmp, arr, n * sizeof(int));
   int hvals[3] = { 13, 4, 1};
   int swaps = 0;
   for (int i = 0; i < 3; i ++) {
      int h = hvals[i];
      for (int offs = 0; offs < h; offs ++)  {
         swaps += sortH(n - offs, tmp + offs, h);
      }
   }
   return swaps;
}

int sort13_4_1_25(const int* arr) {  return sort13_4_1(25, arr);  }


#endif /* ! sort_H */
//EOF

