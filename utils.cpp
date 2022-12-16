#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "utils.h"

using namespace std;

void read_input_file(const char *filename, int& num_rows, vector<int>& key, vector<int>& value)
{
    //readfile
    fstream file;
    file.open(filename);
    string line;

    while(getline(file, line, '\n'))
    {
        istringstream templine(line);
        string data;
        getline(templine, data,',');
        key.push_back(atoi(data.c_str()));
        getline(templine, data,',');
        value.push_back(atoi(data.c_str()));
        num_rows += 1;
    }
    file.close();
    cout << "Data file reading complete, " << num_rows << " rows loaded."<< endl;
    return;
}

void read_key_query_file(const char *filename, int& num_key_query, vector<int>& query_keys)
{
    //readfile
    fstream file;
    file.open(filename);
    string line;

    while(getline(file, line, '\n'))
    {
        istringstream templine(line);
        string data;
        getline(templine, data, ',');
        query_keys.push_back(atoi(data.c_str()));
        num_key_query += 1;
    }
    file.close();
    cout << "Key query file reading complete, " << num_key_query << " queries loaded."<< endl;
    return;
}

void read_range_query_file(const char *filename, int& num_range_query, vector<pair<int,int>>& query_pairs)
{
    //readfile
    fstream file;
    file.open(filename);
    string line;

    while(getline(file, line, '\n'))
    {
        istringstream templine(line);
        string data1, data2;
        getline(templine, data1,',');
        getline(templine, data2,',');
        query_pairs.push_back({atoi(data1.c_str()),atoi(data2.c_str())});
        num_range_query += 1;
    }
    file.close();
    cout << "Range query file reading complete, " << num_range_query << " queries loaded."<< endl;
    return;
}

int compare_result_with_file(const char *filename, vector<int>& result){
    fstream file;
    file.open(filename);
    string line;

    vector<int> ans;

    while(getline(file, line, '\n'))
    {
        istringstream templine(line);
        string data;
        getline(templine, data, ',');
        ans.push_back(atoi(data.c_str()));
    }
    file.close();
    for(int i = 0;i < ans.size();i++){
        if(i >= result.size()){
            cout << "Query " << i <<  ": " << ans[i] << " != NULL (your result)" << endl;
            return 0;
        }
        if(ans[i] != result[i]){
            cout << "Query " << i <<  ": " << ans[i] << " != " << result[i] << " (your result)" << endl;
            return 0;
        }
    }

    return 1;
}

void write_result_file(const char *filename, vector<int>& result){
    ofstream file(filename);
    if(file.is_open())
    {
        for(int i = 0;i < result.size();i++){
            file << result[i] << endl;
        }
        file.close();
    }
}

void record_time_used(const char *filename, int time_to_build_index, int time_to_query_key, int time_to_query_range)
{
    ofstream file(filename);
    if(file.is_open())
    {
        file << time_to_build_index << "," << time_to_query_key << "," << time_to_query_range << endl;
        file.close();
    }
}