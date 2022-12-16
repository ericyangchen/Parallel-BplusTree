#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <mutex>
#include <pthread.h>
#include "utils.h"
#include "btree.h"
#define THREAD 4
#define TASK_INSERT 0
#define TASK_QUERY 1
#define TASK_RANGE_QUERY 2

using namespace std;

node *root = NULL;
int job_idx = 0;
std::mutex job_mutex;
vector<int> key, value, query_keys;
vector<pair<int,int> > query_pairs;
vector<int> key_query_result, range_query_result;

void *worker(void *arg){
    int *type = (int *)arg;
    while(1){
        job_mutex.lock();
        int i = job_idx++;
        job_mutex.unlock();
        if(*type == TASK_INSERT){
            if(i >= key.size()) break;
            root = insert(root, key[i], value[i]);
        }else if(*type == TASK_QUERY){
            if(i >= query_keys.size()) break;
            record *rec;
            rec = find(root, query_keys[i], false, NULL);
            if(rec == NULL){
                key_query_result[i] = -1;
            }else{
                key_query_result[i] = rec->value;
            }
        }else if(*type == TASK_RANGE_QUERY){
            if(i >= query_pairs.size()) break;
            int num = find_range_maxvalue(root, query_pairs[i].first, query_pairs[i].second, false);
		    range_query_result[i] = num;
        }
        
        // if(i % 1000 == 0)cout << "insert count = " << i << endl;
        // cout << "insert(" << key[i] << ", " << value[i] << ")" << endl;
    }
    return NULL;
}

int main()
{
    int num_rows = 0;
    int num_key_query = 0;
    int num_range_query = 0;
    read_input_file("data.txt", num_rows, key, value);
    read_key_query_file("key_query.txt", num_key_query, query_keys);
    read_range_query_file("range_query.txt", num_range_query, query_pairs);
    
    pthread_t tid[THREAD];

    //Uncomment to test your index

    chrono::steady_clock::time_point start = chrono::steady_clock::now();
    //Build index when index constructor is called
    job_idx = 0;
    for(int i = 0;i < 1;i++){
        int type = TASK_INSERT;
        if(pthread_create(&tid[i], NULL, worker, (void *) &type)){
            cout << "[ERROR] Create thread " << i << " failed." << endl;
        }
    }
    for(int i = 0;i < 1;i++){
        pthread_join(tid[i], NULL);
    }

    chrono::steady_clock::time_point built_index = chrono::steady_clock::now();
    //Query by key
    job_idx = 0;
    key_query_result.resize(query_keys.size());
    for(int i = 0;i < THREAD;i++){
        int type = TASK_QUERY;
        if(pthread_create(&tid[i], NULL, worker, (void *) &type)){
            cout << "[ERROR] Create thread " << i << " failed." << endl;
        }
    }
    for(int i = 0;i < THREAD;i++){
        pthread_join(tid[i], NULL);
    }
    chrono::steady_clock::time_point key_query = chrono::steady_clock::now();
    //Query by range of key
    job_idx = 0;
    range_query_result.resize(query_pairs.size());
    for(int i = 0;i < THREAD;i++){
        int type = TASK_RANGE_QUERY;
        if(pthread_create(&tid[i], NULL, worker, (void *) &type)){
            cout << "[ERROR] Create thread " << i << " failed." << endl;
        }
    }
    for(int i = 0;i < THREAD;i++){
        pthread_join(tid[i], NULL);
    }

    chrono::steady_clock::time_point range_query = chrono::steady_clock::now();
    //Free memory

    cout << "------------------------------" << endl;
    auto time_to_build_index = chrono::duration_cast<chrono::microseconds>(built_index - start).count();
    auto time_to_query_key = chrono::duration_cast<chrono::microseconds>(key_query - built_index).count();
    auto time_to_query_range = chrono::duration_cast<chrono::microseconds>(range_query - key_query).count();
    cout << "     Insert Time: " << int(time_to_build_index) << endl;
    cout << "      Query Time: " << int(time_to_query_key) << endl;
    cout << "Range Query Time: " << int(time_to_query_range) << endl;
    
    cout << "------------------------------" << endl;
    if(compare_result_with_file("key_query_ans.txt", key_query_result)){
        cout << "  Key Query: AC" << endl;
    }else{
        cout << "  Key Query: WA" << endl;
    }
    if(compare_result_with_file("range_query_ans.txt", range_query_result)){
        cout << "Range Query: AC" << endl;
    }else{
        cout << "Range Query: WA" << endl;
    }
    cout << "Writing \"key_query_out.txt\" ... ";
    write_result_file("key_query_out.txt", key_query_result);
    cout << "Done." << endl;
    cout << "Writing \"range_query_out.txt\" ... ";
    write_result_file("range_query_out.txt", range_query_result);
    cout << "Done." << endl;

    record_time_used("time_used.txt", int(time_to_build_index), int(time_to_query_key), int(time_to_query_range));
    return 0;
}
