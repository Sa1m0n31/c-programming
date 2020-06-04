#include<stdio.h>
#include<stdlib.h>

/* 
	Example showing how pointers in C works
*/

int main(void)
{
	int i, a, n;
	int *array; /* Array is a pointer */
	
	printf("Number of elements in array: ");
	scanf("%i", &n);
	
	/* Dynamically allocation for array */
	array = (int*) malloc(n * sizeof(int));
	
	/* Input */
	for(i=0; i<n; i++)
	{
		scanf("%i", &a);
		*(array+i) = a;
	}
	
	/* Output */
	printf("Your dynamically allocated array:\n");
	for(i=0; i<n; i++)
	{
		printf("%i ", *(array+i));
	}
	
	free(array);
	
	return 0;
}
