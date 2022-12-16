#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

void read_input_file(const char *filename, int& num_rows, vector<int>& key, vector<int>& value);
void read_key_query_file(const char *filename, int& num_ke_query, vector<int>& query_kes);
void read_range_query_file(const char *filename, int& num_range_query, vector<pair<int,int>>& query_pairs);
int compare_result_with_file(const char *filename, vector<int>& result);
void write_result_file(const char *filename, vector<int>& result);
void record_time_used(const char *filename, int time_to_build_index, int time_to_query_key, int time_to_query_range);
#endif