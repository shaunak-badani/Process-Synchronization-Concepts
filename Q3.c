#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define WAIT_STATE 0
#define ON_RIDE_POOL_ONE 1
#define ON_RIDE_POOL_FULL 2
#define ON_RIDE_PREMIER 3
#define POOL 5
#define PREMIER 6

typedef struct driver_info {
    int state;
} d_info;

d_info cab[10000];

typedef struct pay_slip {
    int inactive;
    int rider_no;
    int cab_no;
} p_slip;

long long int no_of_riders;
long long int no_of_drivers;
long long int no_of_payment_servers;
long long int wait_time_rider = 1;
p_slip payment_servers[10000];

pthread_mutex_t rider_mutex, payment_serve;

typedef struct params {
    int id;
    int cabType;
    int maxWaitTime;
    int rideTime;
} params;

typedef struct s{
    int id;
} s;

int rand_range(int low, int high) {
    if(low == high) return low;
    return low + (rand()) % (high - low + 1);
}

char* print_type(int cabType) {
    return (cabType == 5) ? "Pool" : "Premier";
}

void* check_payment(void* arg) {

    s* inp = (s*) arg;
    int i = inp->id;
    
    while(no_of_riders) {
        if(payment_servers[i].inactive == 1){
            printf("Processing payment for rider %d and cab %d on server : %d\n", payment_servers[i].rider_no, payment_servers[i].cab_no, i);
            sleep(2);
            payment_servers[i].inactive = 0;
        }
        else
            pthread_mutex_unlock(&payment_serve);
    }
}

// Payment servers
void make_payment(int cab_no, int rider_no) {

    int flag = 1;
    while(flag) {
        for(int i = 0 ; i < no_of_payment_servers ; i++) {
            pthread_mutex_lock(&payment_serve);
            if(payment_servers[i].inactive == 0){
                payment_servers[i].cab_no = cab_no;
                payment_servers[i].rider_no = rider_no;
                payment_servers[i].inactive = 1;
                pthread_mutex_unlock(&payment_serve);
                while(payment_servers[i].inactive != 0) {}
                flag = 0;
                break;
            }
            else
                pthread_mutex_unlock(&payment_serve);
        }
        if(!flag)
            pthread_mutex_unlock(&payment_serve);
    }
    
}

void* bookcab(void* arg) {
    params* inp = (params*) arg;
    struct timespec ts;
    int st, en;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_sec;

    printf("Rider %d looking for cab of type %s \n", inp->id, print_type(inp->cabType));
    int flag = 1;
    int i;
    while(flag) {
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        en = ts.tv_sec;
        if(en - st >= inp->maxWaitTime) {
            printf("Rider %d Timed Out\n", inp->id);
            no_of_riders--;
            return NULL;
        }

        for(i = 0 ; i < no_of_drivers ; i++) {
            pthread_mutex_lock(&rider_mutex);
            if(inp->cabType == POOL) {
                if(cab[i].state == ON_RIDE_POOL_ONE) cab[i].state = ON_RIDE_POOL_FULL;
                else if(cab[i].state == WAIT_STATE) cab[i].state = ON_RIDE_POOL_ONE;
                else {
                    pthread_mutex_unlock(&rider_mutex);
                    continue;
                }
            }
            else if(inp->cabType == PREMIER) {
                if(cab[i].state == WAIT_STATE) cab[i].state = ON_RIDE_PREMIER;
                else {
                    pthread_mutex_unlock(&rider_mutex);
                    continue;
                }
            }
            printf("Rider %d sat in cab %d\n", inp->id, i);
            flag = 0;
            pthread_mutex_unlock(&rider_mutex);
            break;
        }
    }

    // wait for cab ride to complete
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_sec;
    en = st;
    while(en - st <= inp->rideTime) {
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        en = ts.tv_sec;
    }

    pthread_mutex_lock(&rider_mutex);
    if(cab[i].state == ON_RIDE_POOL_ONE) cab[i].state = WAIT_STATE;
    if(cab[i].state == ON_RIDE_POOL_FULL) cab[i].state = ON_RIDE_POOL_ONE;
    if(cab[i].state == ON_RIDE_PREMIER) cab[i].state = ON_RIDE_POOL_ONE;
    pthread_mutex_unlock(&rider_mutex);

    make_payment(i, inp->id);

    printf("Rider %d done with ride \n", inp->id);
    no_of_riders--;
}

int main() {
    no_of_riders = 17;
    no_of_drivers = 8;
    no_of_payment_servers = 4;

    pthread_t rider_threads[no_of_riders];
    pthread_t payment_threads[no_of_payment_servers];

    pthread_mutex_init(&rider_mutex, NULL);
    pthread_mutex_init(&payment_serve, NULL);

    for(int i = 0 ; i < no_of_payment_servers ; i++) {
        s* inp = (s*)malloc(sizeof(s));
        inp->id = i;
        pthread_create(&payment_threads[i], NULL, check_payment, inp);
    }

    for(int i = 0 ; i < no_of_riders ; i++) {
        params* inp = (params*)malloc(sizeof(params));
        inp->cabType = rand_range(5, 6);
        inp->maxWaitTime = rand_range(5, 10);
        inp->rideTime = rand_range(10, 20);
        inp->id = i;
        pthread_create(&rider_threads[i], NULL, bookcab, (void*)inp);
        sleep(wait_time_rider);
    }

    for(int i = 0 ; i < no_of_riders ; i++) pthread_join(rider_threads[i], NULL);
    for(int i = 0 ; i < no_of_payment_servers ; i++) pthread_join(payment_threads[i], NULL);
    printf("Simulation Over\n");
}