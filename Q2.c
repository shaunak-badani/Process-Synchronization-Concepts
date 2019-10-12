#include <stdio.h>
#include<unistd.h>
#include <pthread.h>
#include <stdlib.h>


typedef struct tab {
    int no_of_slots;
    int p;
} tab_info;

typedef struct s{
    int id;
} s;

tab_info tables[100000];
pthread_mutex_t robot_mutex, serving_table_mutex, student_mutex;
long long int robot_biryani_vessels[10000];
long long int no_of_students;
long long int no_of_tables;
long long int no_of_chefs;
long long int time_consume_biryani = 15;  

void biryani_ready(void* arg, int no_of_vessels_prepared);

int rand_range(int low, int high) {
    if(low == high) return low;
    return low + (rand()) % (high - low);
}

/* Student functions */

void student_in_slot(int student_id, int table) {
    if(no_of_students <= 0) return; 
    printf("Student %d eating on table %d\n", student_id, table);
    sleep(time_consume_biryani);
    no_of_students--;
    printf("Student %d has finished eating. \n", student_id);
    tables[table].no_of_slots++;
    tables[table].p--;
}

void* wait_for_slot(void* arg) {
    s* inp = (s*) arg;
    int i;
    int flag = 1;
    while(flag) {
        for(i = 0 ; i < no_of_tables ; i++) {
            pthread_mutex_lock(&student_mutex);
            // printf("Locked %d\n", inp->id);
            // printf("tables[%d].p: %d\n", i, tables[i].p);
            if(tables[i].p > 0 && tables[i].no_of_slots > 0) {
                // printf("before decrement tables[%d] no of slots %d\n", i, tables[i].no_of_slots);                
                tables[i].no_of_slots--;
                // printf("tables[%d] no of slots %d\n", i, tables[i].no_of_slots);
                pthread_mutex_unlock(&student_mutex);
                student_in_slot(inp->id, i);
                flag = 0;
                break;
            }
            else{
                // printf("tables[%d].p: %d\n", i, tables[i].p);
                // sleep(3);
                // printf("unlocked %d\n", inp->id);
                pthread_mutex_unlock(&student_mutex);
            }
        }
        if(!flag) {
            // printf("unlocked !flag %d\n", inp->id);
            pthread_mutex_unlock(&student_mutex);
        }
    }
}

/*
    Biryani vessel functions
*/

void ready_to_serve_table(int id) {
    int no_of_slots = rand_range(1, tables[id].p);
    if(no_of_students == 0) return;
    printf("%d slots emptied on table %d\n", no_of_slots, id);
    sleep(2);
    tables[id].no_of_slots += no_of_slots;
}

void* wait_for_biryani(void *arg) {
    s* inp = (s*) arg;

    // wait for biryani vessels to be prepared
    int flag = 1, i;
    while(flag && no_of_students > 0){
        for(i = 0 ; i < no_of_chefs ; i++) {
            pthread_mutex_lock(&serving_table_mutex);
            if(robot_biryani_vessels[i] > 0) {
                robot_biryani_vessels[i]--;
                flag = 0;
                break;
            }
            else 
                pthread_mutex_unlock(&serving_table_mutex);
        }
        if(!flag) pthread_mutex_unlock(&serving_table_mutex);
    }

    int p = rand_range(2, 5);
    tables[inp->id].p = p;
    if(no_of_students <= 0) return NULL; 
    printf("Loading biryani vessel of chef %d into table %d with feeding capacity %d\n", i, inp->id, p);
    sleep(3);
    ready_to_serve_table(inp->id);
}


/*
    End biryani vessel functions
*/


/*
    Robot chef functions
*/

void* make_biryani(void* arg) {
    s* inp = (s*) arg;
    int robot_id = inp->id;
    while(no_of_students > 0) {
        while(robot_biryani_vessels[robot_id] > 0) {
            if(no_of_students == 0) return NULL; 
            // printf("inside\n");
        }
        int time_taken = rand_range(2, 5);
        int rest_time = 1;
        sleep(rest_time);
        printf("Robot chef %d taking %d secs to make biryani.\n", inp->id, time_taken);
        sleep(time_taken);

        // no of biryani vessels
        // int r = rand_range(1, 10); 
        int r = rand_range(1, 3); 
        printf("Robot chef %d ready with %d biryani vessels.\n", inp->id, r);
        biryani_ready(arg, r);   

    }
    return NULL;
}

void biryani_ready(void* arg, int no_of_vessels_prepared) {
    s* inp = (s*) arg;
    int robot_id = inp->id;
    robot_biryani_vessels[robot_id] += no_of_vessels_prepared;

    return;
}

/*
test
*/

void* test_function(void *arg) {
    while(1) {
        for(int i = 0 ; i < no_of_chefs ; i++) {
            printf("table[%d] : %lld\n", i, robot_biryani_vessels[i]);
        }
        sleep(5);
    }
}

/*
    End robot chef functions
*/

void initializePipeline(int M, int ST, int N) {
    pthread_t robot_chef_ids[M];
    pthread_t serving_table_ids[ST];
    pthread_t student_ids[N];

    int wait_time_student = 1;

    for(int i = 0 ; i < M ; i++) {
        s* inp = (s*)malloc(sizeof(s));
        inp->id = i;
        pthread_create(&robot_chef_ids[i], NULL, make_biryani, (void*)inp);
    }
    for(int i = 0 ; i < ST ; i++) {
        s* inp = (s*)malloc(sizeof(s));
        inp->id = i;
        pthread_create(&serving_table_ids[i], NULL, wait_for_biryani, (void*)inp);
    }
    
    pthread_t test;
    s* inp = (s*)malloc(sizeof(s));
    // pthread_create(&test, NULL, test_function, (void*) inp);

    for(int i = 0 ; i < N ; i++) {
        s* inp = (s*)malloc(sizeof(s));
        inp->id = i;
        sleep(wait_time_student);
        pthread_create(&student_ids[i], NULL, wait_for_slot, (void*)inp);
        printf("Student %d has entered the line. \n", i);
    }

    for(int i = 0 ; i < M ; i++) {
        pthread_join(robot_chef_ids[i], NULL);
    }
    for(int i = 0 ; i < ST ; i++) {
        pthread_join(serving_table_ids[i], NULL);
    }
    for(int i = 0 ; i < N ; i++) {
        pthread_join(student_ids[i], NULL);
    }
    printf("Simulation Over\n");

}

int main() {
    no_of_tables = 6;
    no_of_students = 10;
    no_of_chefs = 3;
    initializePipeline(no_of_chefs,  no_of_tables, no_of_students);
}