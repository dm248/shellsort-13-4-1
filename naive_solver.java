import java.io.File;
import java.io.PrintStream;
import java.io.FileNotFoundException;

public class ShellSort 
{	
	/**
	 * 
	 * @param A
	 * @return
	 */
	public static boolean isSorted(int[] A) 
	{
		for (int i = 1; i < A.length; i += 1) 
		{
			if (A[i - 1] > A[i]) 
			{
				return false;
			}
		}
		
		return true;
	}
	
	/**
	 * 
	 * @param A
	 * @return
	 */
	public static void print(int[] A) 
	{
		StringBuilder SB = new StringBuilder();
		
		for (int i = 0; i < A.length; i += 1) 
		{
			SB.append(String.valueOf(A[i]) + ' ');
		}
		
		System.out.println(SB.toString());
	}
	
	/**
	 * 
	 * @param A
	 * @param h
	 * @return
	 */
	public static int sort(int[] A, int[] h) 
	{
		int n = A.length, i, j;
		
		int compares = 0;
		
	    for (int gap : h) 
	    {
		    for (i = 1; i < n; i += 1) 
		    {
		        j = i - 1;

		        while (j >= 0 && j + gap < n) 
		        {
		        	compares += 1;
		        	
		        	if (A[j] > A[j + gap]) 
		        	{
		        		swap(A, j, j + gap);
			            j -= gap;
			            //print(A);
		        	}
		        	else 
		        	{
		        		break;
		        	}
		        }
		    }
	    }
	    
	    return compares;
	}
	
	/**
	 * 
	 * @param A
	 * @param i
	 * @param j
	 */
	private static void swap(int[] A, int i, int j) 
	{
	    int t = A[i];
	    A[i] = A[j];
	    A[j] = t;
	}
	
	/**
	 * 
	 * @param A
	 * @param h
	 * @param threshold
	 */
	private static void test(int[] A, int[] h, double threshold) 
	{
		StringBuilder SB = new StringBuilder();
		
		SB.append("-----------------------------\n");
		SB.append(ShellSort.toString(A) + '\n');
		
		int compares = sort(A, h);
		
		//SB.append(ShellSort.toString(A));
		SB.append("Is sorted? " + String.valueOf(isSorted(A)) + '\n');
		SB.append("#compares = " + String.valueOf(compares) + '\n');
		SB.append("threshold = " + String.valueOf(threshold) + '\n');
		SB.append("Bad case? = " + String.valueOf(compares >= threshold) + '\n');
		
		System.out.println(SB.toString());
	}

	/**
	 * 
	 * @param A
	 * @param l
	 * @param r
	 * @param h
	 * @param threshold
	 */
	public static void testPermutations(int[] A, int l, int r, int[] h, double threshold) 
	{
		int[] C = new int[A.length];
		
		if (l == r) 
		{
			test(A, h, threshold);
		}
		else 
		{
			for (int i = l; i <= r; i += 1) 
			{
				System.arraycopy(A, 0, C, 0, A.length);
				swap(C, l, i);
				testPermutations(C, l + 1, r, h, threshold);
				swap(C, l, i);
			}
		}
	}
	
	/**
	 * 
	 * @param A
	 */
	public static String toString(int[] A) 
	{
		StringBuilder SB = new StringBuilder();
		
		for (int i = 0; i < A.length; i += 1) 
		{
			SB.append(String.valueOf(A[i]) + ' ');
		}
		
		return SB.toString();
	}
	
	/**
	 * 
	 * @param args
	 */
	public static void main(String[] args) throws FileNotFoundException
	{
		PrintStream output = new PrintStream(new File("shellsort_bad_cases.txt"));
		System.setOut(output);
		
		int n = 25;
		double threshold = Math.pow(n,  1.5);
		
		int[] A = new int[n];
		int[] h = {13, 4, 1};
		
		for (int i = 0; i < n; i += 1) 
		{
			A[i] = n - (i + 1);
		}
		
		ShellSort.testPermutations(A, 0, n - 1, h, threshold);
	}
}