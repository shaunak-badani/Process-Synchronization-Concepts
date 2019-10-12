#define _POSIX_C_SOURCE 199309L //required for clock
#include <stdio.h> 
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>
#include <pthread.h>

// clock libraries
#include <time.h>


int * shareMem(size_t size){
    key_t mem_key = IPC_PRIVATE;
    int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
    return (int*)shmat(shm_id, NULL, 0);
}

int partition (int arr[], int l, int r) 
{ 
	int pivot = arr[l];  
	int partition_index = r + 1, tmp;  

	for (int j = r; j >= l; j--) 
	{ 
		if (arr[j] > pivot) 
		{ 
			partition_index--;
            tmp = arr[partition_index];
            arr[partition_index] = arr[j];
            arr[j] = tmp;
		} 
	} 
    partition_index--;
    tmp = arr[partition_index];
    arr[partition_index] = arr[l];
    arr[l] = tmp;
	return partition_index; 
}

void normal_quicksort(int arr[], int l, int r) {
     if (l < r)
    {
        int pi = partition(arr, l, r);

        normal_quicksort(arr, l, pi - 1);
        normal_quicksort(arr, pi + 1, r); 
    }
}

// processes quick sort
void quicksort(int arr[], int l, int r) 
{ 
    int temp;
    if(r - l + 1 <= 5){
        for(int i = l ; i < r ; i++)
        {
            for(int j = i + 1; j <= r; j++)
                if(arr[j] < arr[i] && j <= r) 
                {
                    temp = arr[i];
                    arr[i] = arr[j];
                    arr[j] = temp;
                }
        }
        return;
    }

	if (l < r) 
	{ 
		int pi = partition(arr, l, r); 
        int pid_left = fork();

        if(pid_left == 0) {
		    quicksort(arr, l, pi - 1); 
            exit(0);
        }
        else {
            int pid_right = fork();
            if(pid_right == 0) {
		        quicksort(arr, pi + 1, r);
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

typedef struct params {
    int l;
    int r;
    int* arr;
} params;

// threaded quick sort
void* threaded_quicksort(void* inp) {
    params* p = (params*) inp;

    int temp;
    int l = p->l;
    int r = p->r;
    int* arr = p->arr;

    if(r - l + 1 <= 5){
        for(int i = l ; i < r ; i++)
        {
            for(int j = i + 1; j <= r; j++)
                if(arr[j] < arr[i] && j <= r) 
                {
                    temp = arr[i];
                    arr[i] = arr[j];
                    arr[j] = temp;
                }
        }
        return NULL;
    }
    int pi = partition(arr, l, r);

    params left_half;
    left_half.l = l;
    left_half.r = pi - 1;
    left_half.arr = arr;
    pthread_t pid_left;
    pthread_create(&pid_left, NULL, threaded_quicksort, &left_half);

    params right_half;
    right_half.l = pi + 1;
    right_half.r = r;
    right_half.arr = arr;
    pthread_t pid_right;
    pthread_create(&pid_right, NULL, threaded_quicksort, &right_half);

    pthread_join(pid_left, NULL);
    pthread_join(pid_right, NULL);
}

int main() 
{ 
    struct timespec ts;
    long long int n;

    scanf("%lld", &n);
    int *arr = shareMem(sizeof(int)*(n+1));    
    for(int i=0;i<n;i++) scanf("%d", arr+i);


    int arr_merge_sort[n+1], arr_threaded_merge_sort[n+1];
    for(int i=0;i<n;i++) {
        arr_merge_sort[i] = arr[i];
        arr_threaded_merge_sort[i] = arr[i];
    }

    printf("Running concurrent quicksort for n = %lld\n", n);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double st = ts.tv_nsec/(1e9)+ts.tv_sec;

	quicksort(arr, 0, n - 1);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);

    // end concurrent quick sort

    printf("Running normal quicksort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;

    normal_quicksort(arr_merge_sort, 0, n-1);    
    
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);

    // end normal quicksort

    printf("Running threaded quicksort for n = %lld\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec/(1e9)+ts.tv_sec;

    params p1;

    p1.arr = arr_threaded_merge_sort;
    p1.l = 0;
    p1.r = n - 1;
    pthread_t thread_quicksort;
    pthread_create(&thread_quicksort, NULL, threaded_quicksort, &p1);
    pthread_join(thread_quicksort, NULL);    

    for(int i = 0 ; i < n ; i++) printf("%d ", arr_threaded_merge_sort[i]);
    printf("\n");
    
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec/(1e9)+ts.tv_sec;
    printf("time = %Lf\n", en - st);


    printf("\n");
	return 0; 
} 