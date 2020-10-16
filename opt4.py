# this is some trial Python code that was used to check strategies for optimizing
# the 4-unsort generation - I did not bother with cleaning this up so ymmv...
#
# there are 7! * 6! * 6! * 6! states that give the same after 4-sort
# (these control elements at locations 4k+a, a = 0, 1, 2, 3)
#
# group these by the number of steps needed to sort them
#

import itertools
import score


def genGroup(n):
   maxCount = (n * (n-1)) // 2
   countFreqs = [0] * (maxCount + 1)
   perms = []
   for i,p in enumerate( itertools.permutations([ i for i in range(n) ]) ):
       swaps = score.countNormal(list(p))
       perms.append( (swaps, p) )
       countFreqs[swaps] += 1
   return countFreqs, perms


freqs6, perms6 = genGroup(6)
freqs7, perms7 = genGroup(7)

perms6.sort(key = lambda v: -v[0])
perms7.sort(key = lambda v: -v[0])

def groupCounts():
   print(freqs6, freqs7)

   # generate number of states that need a given number of swaps to 4-sort

   # max swaps possible 
   maxSwap6 = len(freqs6) - 1
   maxSwap7 = len(freqs7) - 1
   maxSwaps = maxSwap7 + 3 * maxSwap6

   swapCounts = [0] * (maxSwaps + 1)
   print(maxSwaps)

   for i in range(maxSwap6 + 1):
      for j in range(maxSwap6 + 1):
         for k in range(maxSwap6 + 1):
            for l in range(maxSwap7 + 1):
               sw = i + j + k + l
               cnt = freqs6[i] * freqs6[j] * freqs6[k] * freqs7[l]
               swapCounts[sw] += cnt

   print("#i, counts(i), sum(>=i)")
   for i in range(maxSwaps + 1):
      print(i, swapCounts[i], sum(swapCounts[i:]))
   print("#TOTAL:", sum(swapCounts))



### now try to maximize the 13- and 4-sort swaps for the best 13-4-sorted arrangement
#
# loop through all 4k+1 perms, and all 4k+2 perms, select those that are allowed
# by the THREE 13-sort constraints between these sets
#

def findBest4(arr, swapsMIN = 0, progress = False):
   swaps0 = score.countNormal(arr.copy())
   if swaps0 + 66 <= swapsMIN: return swaps0, None, 0, None
   cnt, best = 0, 0
   bestLst, bestArr = None, None
   tmp = arr.copy()
   for (sw1, p1) in perms6:
      swaps1 = swaps0 + sw1
      if swaps1 + 51 <= swapsMIN: break
      for i,idx in enumerate(p1):  tmp[4 * i + 1] = arr[4 * idx + 1]
      for (sw2, p2) in perms6:
         swaps2 = swaps1 + sw2
         if swaps2 + 36 <= swapsMIN: break
         for i,idx in enumerate(p2):  tmp[4 * i + 2] = arr[4 * idx + 2]
         if tmp[1] > tmp[14] or tmp[5] > tmp[18] or tmp[9] > tmp[22]: continue
         for (sw3, p3) in perms6:
            swaps3 = swaps2 + sw3
            if swaps3 + 21 <= swapsMIN: break
            for i,idx in enumerate(p3):  tmp[4 * i + 3] = arr[4 * idx + 3]
            if tmp[2] > tmp[15] or tmp[6] > tmp[19] or tmp[10] > tmp[23]: continue
            for (sw4, p4) in perms7:
               swaps4 = swaps3 + sw4
               if swaps4 <= swapsMIN: break
               for i,idx in enumerate(p4):  tmp[4 * i] = arr[4 * idx]
               if tmp[3] > tmp[16] or tmp[7] > tmp[20] or tmp[11] > tmp[24]: continue
               if tmp[0] > tmp[13] or tmp[4] > tmp[17] or tmp[8] > tmp[21]: continue
               cnt += 1
               tmp2 = tmp.copy()
               swaps, lst = score.count(tmp2)
               assert lst[0] == 0
               assert swaps == swaps4, str(swaps) + " " + str(swaps4)
               if swaps > best:
                  best, bestLst = swaps, lst
                  bestArr = tmp.copy()
                  if progress:  print(best, bestLst, cnt, bestArr)
   return best, bestLst, cnt, bestArr


def do108(swapMIN = 0):
   arr108 = [15,9,3,0, 17,11,5,1, 19,13,7,2, 21,16,10,4, 22,18,12,6, 23,20,14,8, 24]
   print( findBest4(arr108, swapMIN) )
   #135, [0, 27, 108] 31104 [24, 13, 14, 8, 23, 11, 12, 6, 22, 9, 10, 4, 21, 20, 7, 2, 19, 18, 5, 1, 17, 16, 3, 0, 15]

def do107(swapMIN):
   arrs107 = (
      [15,8,3,0, 17,11,5,1, 19,13,7,2, 21,16,10,4, 22,18,12,6, 23,20,14,9, 24],
      [15,9,3,0, 17,10,5,1, 19,13,7,2, 21,16,11,4, 22,18,12,6, 23,20,14,8, 24],
      [15,9,3,0, 17,11,5,1, 19,12,7,2, 21,16,10,4, 22,18,13,6, 23,20,14,8, 24]  )
   for arr in arrs107:
      print(findBest4(arr, swapMIN))
   #134, [0, 27, 107], 31104, [24, 13, 14, 9, 23, 11, 12, 6, 22, 8, 10, 4, 21, 20, 7, 2, 19, 18, 5, 1, 17, 16, 3, 0, 15]
   #134, [0, 27, 107], 62208, [24, 13, 14, 8, 23, 10, 12, 6, 22, 9, 11, 4, 21, 20, 7, 2, 19, 18, 5, 1, 17, 16, 3, 0, 15]
   #134, [0, 27, 107], 62208, [24, 12, 14, 8, 23, 11, 13, 6, 22, 9, 10, 4, 21, 20, 7, 2, 19, 18, 5, 1, 17, 16, 3, 0, 15]


def do106(swapMIN):
   arrs = (
      [15,7,3,0, 17,11,5,1, 19,13,8,2, 21,16,10,4, 22,18,12,6, 23,20,14,9, 24],
      [15,8,3,0, 17,10,5,1, 19,13,7,2, 21,16,11,4, 22,18,12,6, 23,20,14,9, 24],
      [15,8,3,0, 17,11,5,1, 19,12,7,2, 21,16,10,4, 22,18,13,6, 23,20,14,9, 24],
      [15,9,3,0, 17,10,5,1, 19,12,7,2, 21,16,11,4, 22,18,13,6, 23,20,14,8, 24],
      [15,9,2,0, 17,10,5,1, 19,13,7,3, 21,16,11,4, 22,18,12,6, 23,20,14,8, 24])
   for arr in arrs:
      print(findBest4(arr, swapMIN))
   #133, [0, 27, 106], 31104, [24, 13, 14, 9, 23, 11, 12, 6, 22, 7, 10, 4, 21, 20, 8, 2, 19, 18, 5, 1, 17, 16, 3, 0, 15]
   #133, [0, 27, 106], 62208, [24, 13, 14, 9, 23, 10, 12, 6, 22, 8, 11, 4, 21, 20, 7, 2, 19, 18, 5, 1, 17, 16, 3, 0, 15]
   #133, [0, 27, 106], 62208, [24, 12, 14, 9, 23, 11, 13, 6, 22, 8, 10, 4, 21, 20, 7, 2, 19, 18, 5, 1, 17, 16, 3, 0, 15]
   #133, [0, 27, 106], 124416, [24, 12, 14, 8, 23, 10, 13, 6, 22, 9, 11, 4, 21, 20, 7, 2, 19, 18, 5, 1, 17, 16, 3, 0, 15]
   #134, [0, 28, 106], 124416, [24, 13, 14, 8, 23, 10, 12, 6, 22, 9, 11, 4, 21, 20, 7, 3, 19, 18, 5, 1, 17, 16, 2, 0, 15]

def do105(swapMIN):
   arrs = (
      [15,6,3,0, 17,11,5,1, 19,13,8,2, 21,16,10,4, 22,18,12,7, 23,20,14,9, 24],
      [15,7,3,0, 17,11,5,1, 19,12,8,2, 21,16,10,4, 22,18,13,6, 23,20,14,9, 24],
      [15,7,2,0, 17,11,5,1, 19,13,8,3, 21,16,10,4, 22,18,12,6, 23,20,14,9, 24],
      [15,8,2,0, 17,10,5,1, 19,13,7,3, 21,16,11,4, 22,18,12,6, 23,20,14,9, 24],
      [13,8,3,0, 17,11,5,1, 19,14,7,2, 21,16,10,4, 22,18,12,6, 23,20,15,9, 24],
      [14,9,3,0, 17,10,5,1, 19,13,7,2, 21,15,11,4, 22,18,12,6, 23,20,16,8, 24] )
   for arr in arrs:
      print(findBest4(arr, swapMIN))
   #132, [0, 27, 105], 31104, [24, 13, 14, 9, 23, 11, 12, 7, 22, 6, 10, 4, 21, 20, 8, 2, 19, 18, 5, 1, 17, 16, 3, 0, 15]
   #132, [0, 27, 105], 62208, [24, 12, 14, 9, 23, 11, 13, 6, 22, 7, 10, 4, 21, 20, 8, 2, 19, 18, 5, 1, 17, 16, 3, 0, 15]
   #133, [0, 28, 105], 62208, [24, 13, 14, 9, 23, 11, 12, 6, 22, 7, 10, 4, 21, 20, 8, 3, 19, 18, 5, 1, 17, 16, 2, 0, 15]
   #133, [0, 28, 105], 124416, [24, 13, 14, 9, 23, 10, 12, 6, 22, 8, 11, 4, 21, 20, 7, 3, 19, 18, 5, 1, 17, 16, 2, 0, 15]
   #132, [0, 27, 105], 31104, [24, 14, 15, 9, 23, 11, 12, 6, 22, 8, 10, 4, 21, 20, 7, 2, 19, 18, 5, 1, 17, 16, 3, 0, 13]
   #132, [0, 27, 105], 62208, [24, 13, 16, 8, 23, 10, 12, 6, 22, 9, 11, 4, 21, 20, 7, 2, 19, 18, 5, 1, 17, 15, 3, 0, 14]



#do108(130)
#do107(130)
#do106(130)
#do105(130)

do108(149)
do107(149)
do106(149)
do105(149)

arr = [18, 24, 5, 11, 17, 23, 4, 10, 16, 22, 3, 9, 12, 15, 21, 2, 8, 14, 20, 1, 7, 13, 19, 0, 6]
for h in (13, 4):
   for p in range(0, h):
      _, arr = score.countH(arr, h, p)
   print(h, arr)
print( findBest4(arr, 149) )

arr2 = [15, 21, 2, 8, 14, 20, 1, 7, 13, 19, 0, 6, 12, 18, 24, 5, 11, 17, 23, 4, 10, 16, 22, 3, 9]
print( score.count(arr2.copy()), arr2 )

arr3 = [9, 16, 0, 3, 10, 17, 1, 4, 11, 18, 2, 5, 12, 19, 22, 6, 13, 20, 23, 7, 14, 21, 24, 8, 15]
print( score.count(arr3.copy()), arr3 )

print( [ arr3[4 * i + 1]  for i in range(6) ])
