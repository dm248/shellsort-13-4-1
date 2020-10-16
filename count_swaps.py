# python swap countering and sorting benchmarks, for testing
#

# whether a sequence is sorted
def sorted(arr):
   return all( [ arr[j] < arr[j + 1]  for j in range(len(arr) - 1) ] ) 

# do an h-sort, count swaps
def countH(arr, h, offs):
   n = len(arr)
   swaps = 0
   for i in range(offs + h, n, h):
      v = arr[i]
      j = i - h
      while j >= offs:
         if v > arr[j]:  break
         arr[j + h] = arr[j]
         swaps += 1
         j -= h
      arr[j + h] = v
   return swaps, arr

# do a 13,4,1 shellsort, count swaps in each stage and the total
def count(arr):
   swapsStages = []
   for h in (13, 4, 1):
      swaps = 0
      for p in range(0, h):
         sw, arr = countH(arr, h, p)
         swaps += sw
      swapsStages.append(swaps)
   assert sorted(arr)
   return sum(swapsStages), swapsStages


# normal sort swap count
def countNormal(arr):
   tmp = arr.copy()
   return countH(tmp, 1, 0)[0]


if __name__ == "__main__":

   # a bunch of test vectors
   # 12+57+93 = 162
   arr0 = [18, 24, 5, 11, 17, 23, 4, 10, 16, 22, 3, 9, 12, 15, 21, 2, 8, 14, 20, 1, 7, 13, 19, 0, 6]
   # 0+0+0 = 0
   arr1 = [ i for i in range(25) ]
   # 12+24+36 = 72, 300 in 1-sort
   arr2 = [ 24 - i for i in range(25) ]
   arr3 = [13 + i  for i in range(12)] + [i for i in range(13)]
   arr4 = [3, 0, 16, 10,   5, 1, 18, 12,   7, 2, 20, 14,   9, 4, 22, 17,
           11, 6, 23, 19,   13, 8, 24, 21,   15 ]

   arr5 = [15, 21, 2, 8, 14, 20, 1, 7, 13, 19, 0, 6, 12, 18, 24, 5, 11, 17, 23, 4, 10, 16, 22, 3, 9]


   print(f"swaps={count(arr0.copy())}, normal={countNormal(arr0)}, {arr0}")
   print(f"swaps={count(arr1.copy())}, normal={countNormal(arr1)}, {arr1}")
   print(f"swaps={count(arr2.copy())}, normal={countNormal(arr2)}, {arr2}")
   print(f"swaps={count(arr3.copy())}, normal={countNormal(arr3)}, {arr3}")
   print(f"swaps={count(arr4.copy())}, normal={countNormal(arr4)}, {arr4}")
   print(f"swaps={count(arr5.copy())}, normal={countNormal(arr5)}, {arr5}")


#EOF
