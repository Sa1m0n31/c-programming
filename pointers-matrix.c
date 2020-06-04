#include<stdio.h>
#include<stdlib.h>

/*
	Example showing how to use pointers as two-dimensional array in C
*/

int main(void)
{
	int **matrix;
	int i, j, m, n, a;
	
	/* Dimensions of matrix */
	scanf("%i", &m);
	scanf("%i", &n);
	
	/* Allocation for matrix */
	matrix = (int**) malloc(m * sizeof(int*));
	
	/* Allocation for each row in matrix */
	for(i=0; i<m; i++)
	{
		*(matrix+i) = (int*) malloc(n * sizeof(int)); 
	}
	
	/* Each row is an array, matrix is an array of arrays */
	for(i=0; i<m; i++)
	{
		for(j=0; j<n; j++)
		{
			scanf(" %i", &a);
			*(*(matrix+i)+j) = a;
		}
	}
	
	/* Output */
	printf("Your matrix:\n");
	for(i=0; i<m; i++)
	{
		for(j=0; j<n; j++)
		{
			printf("%-5i", *(*(matrix+i)+j));
		}
		printf("\n");
	}
	
	free(matrix);
	return 0;
}
