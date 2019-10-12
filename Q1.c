#include <stdio.h> 
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>

int * shareMem(size_t size){
    key_t mem_key = IPC_PRIVATE;
    int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
    return (int*)shmat(shm_id, NULL, 0);
}

void swap(int* a, int* b) 
{ 
	int t = *a; 
	*a = *b; 
	*b = t; 
} 

int partition (int arr[], int low, int high) 
{ 
	int pivot = arr[high];  
	int i = (low - 1);  

	for (int j = low; j <= high - 1; j++) 
	{ 
		if (arr[j] < pivot) 
		{ 
			i++;
			swap(&arr[i], &arr[j]); 
		} 
	} 
	swap(&arr[i + 1], &arr[high]); 
	return (i + 1); 
} 

void quicksort(int arr[], int low, int high) 
{ 
    if(high - low + 1 <= 5){
        for(int i = low ; i < high ; i++)
        {
            for(int j = i + 1; j <= high; j++)
                if(arr[j] < arr[i] && j <= high) 
                {
                    int temp = arr[i];
                    arr[i] = arr[j];
                    arr[j] = temp;
                }
        }
        return;
    }

	if (low < high) 
	{ 
		int pi = partition(arr, low, high); 
        int pid_left = fork();

        if(pid_left == 0) {
		    quicksort(arr, low, pi - 1); 
            exit(0);
        }
        else {
            int pid_right = fork();
            if(pid_right == 0) {
		        quicksort(arr, pi + 1, high);
                exit(0);
            }
            else {
                int status;
                waitpid(pid_left, &status, 0);
                waitpid(pid_right, &status, 0);
            }
        }
	} 
} 

int main() 
{ 
    int n;
    scanf("%d", &n);
    int *arr = shareMem(sizeof(int)*(n+1));    
    for(int i=0;i<n;i++) scanf("%d", arr+i);

	quicksort(arr, 0, n - 1);
    for(int i = 0 ; i < n ; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
	return 0; 
} 