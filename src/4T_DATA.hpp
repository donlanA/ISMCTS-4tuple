#ifndef DATA_HPP
#define DATA_HPP

#include "4T_header.h"

class GST;

class DATA {
public:
    unsigned long long LUTw_E[TUPLE_NUM * FEATURE_NUM + 1] = {0}, LUTv_E[TUPLE_NUM * FEATURE_NUM + 1] = {0};
    unsigned long long LUTw_U[TUPLE_NUM * FEATURE_NUM + 1] = {0}, LUTv_U[TUPLE_NUM * FEATURE_NUM + 1] = {0};

    // R1、B1 variable
    unsigned long long LUTw_E_R1[TUPLE_NUM * FEATURE_NUM + 1] = {0}, LUTv_E_R1[TUPLE_NUM * FEATURE_NUM + 1] = {0};
    unsigned long long LUTw_U_R1[TUPLE_NUM * FEATURE_NUM + 1] = {0}, LUTv_U_R1[TUPLE_NUM * FEATURE_NUM + 1] = {0};
    unsigned long long LUTw_E_B1[TUPLE_NUM * FEATURE_NUM + 1] = {0}, LUTv_E_B1[TUPLE_NUM * FEATURE_NUM + 1] = {0};
    unsigned long long LUTw_U_B1[TUPLE_NUM * FEATURE_NUM + 1] = {0}, LUTv_U_B1[TUPLE_NUM * FEATURE_NUM + 1] = {0};

    int trans[POS_NUM + 1] = {0};      //transfer now_board to index

    void init_data();
    int LUT_idx(int location, int feature){
        return (location - 1) * FEATURE_NUM + feature;
    }
    void read_data_file(int num);
    void write_data_file();
    void write_data_file_run(int run);

    // R1、B1 function
    void read_data_file_R1(int num);
    void write_data_file_run_R1(int run);
    void read_data_file_B1(int num);
    void write_data_file_run_B1(int run);
};

#endif