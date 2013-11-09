#include "PowerBudget.h"
//#include <bitset>

using namespace DRAMSim;

#define SHARED_BUDGET 1

PowerBudget::PowerBudget(){
    token = (int32_t*) calloc(NUM_CHIPS,sizeof(int32_t));
    issued_requests = (Request*) calloc(NUM_BANKS,sizeof(Request));
    for(size_t i=0; i<=NUM_BANKS-1; ++i)
        issued_requests[i].line = (uint64_t*) calloc(LINE_SIZE>>3,sizeof(uint64_t));
}

PowerBudget::PowerBudget(uint16_t budget){
    line_num = LINE_SIZE>>3;
    token = (int32_t*) malloc(NUM_CHIPS*sizeof(int32_t));
    issued_requests = (Request*) calloc(NUM_BANKS,sizeof(Request));
    for(size_t i=0; i<=NUM_BANKS-1; ++i)
        issued_requests[i].line = (uint64_t*) calloc(LINE_SIZE>>3,sizeof(uint64_t));
    setBudget(budget);
#if SHARED_BUDGET
    cerr << "SHARED POWER BUDGET CONFIGURATION" << endl;
#endif
}

void PowerBudget::setBudget(uint16_t budget){
    for(size_t i=0; i<=NUM_CHIPS-1; ++i)
        token[i] = budget;
}

bool PowerBudget::consume(uint64_t* line, size_t bank_no, uint64_t completed_time){
    //uint8_t size_row_per_chip = SIZE_ROW/NUM_CHIPS;
    size_t line_per_chip = line_num/NUM_CHIPS;
    bool    out_of_token = false;

    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        //token[i] -= countTokens((line[j] << size_row_per_chip*i) >> size_row_per_chip*(NUM_CHIPS-1)); 
        for(size_t j=0; j<= line_per_chip-1; ++j){
            token[i] -= countTokens(line[i*line_per_chip+j]); 
        }
        out_of_token |= (token[i] < 0);
    }
#if SHARED_BUDGET
    int32_t remain_token = 0;
    for(size_t i=0; i<=NUM_CHIPS-1; ++i)
        remain_token += token[i];
    out_of_token = remain_token < 0;
#endif
    if(out_of_token){
        reclaimLine(line);
        return false;
    }
    for(size_t i=0; i<=line_num-1; ++i)
        issued_requests[bank_no].line[i] = line[i];     
    issued_requests[bank_no].completed_time = completed_time;     
    return true;
}

uint32_t inline PowerBudget::countTokens(uint64_t sub_line){
    uint32_t counter;
    for(counter = 0; sub_line; counter++)  
        sub_line &= sub_line - 1; 
    return counter;
}

bool PowerBudget::issuable(uint64_t* line){
    //uint8_t size_row_per_chip = SIZE_ROW/NUM_CHIPS;
    size_t line_per_chip = line_num/NUM_CHIPS;
    size_t allocated_token = 0;
    bool   out_of_token = false;
    
    for(size_t i=0; i<=NUM_CHIPS-1; ++i, allocated_token=0){
        //out_of_token |= (token[i] < (int64_t) countTokens((line << size_row_per_chip*i) >> size_row_per_chip*(NUM_CHIPS-1)));
        for(size_t j=0; j<= line_per_chip-1; ++j){
            allocated_token += countTokens(line[i*line_per_chip+j]); 
            //cout << std::bitset<64>(line)[i*line_per_chip+j] << endl;
        }
        //cout << token[i] << " " << allocated_token << endl;
        out_of_token |= (allocated_token > token[i]);
    }
#if SHARED_BUDGET
    int32_t remain_token = 0;
    for(size_t i=0; i<=NUM_CHIPS-1; ++i)
        remain_token += token[i];
    out_of_token = remain_token < 0;
#endif
    return !out_of_token;
}

void PowerBudget::reclaimLine(uint64_t* line){
    //uint8_t size_row_per_chip = SIZE_ROW/NUM_CHIPS;
    size_t line_per_chip = line_num/NUM_CHIPS;
    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        //token[i] += countTokens((line << size_row_per_chip*i) >> size_row_per_chip*(NUM_CHIPS-1)); 
        for(size_t j=0; j<= line_per_chip-1; ++j){
            token[i] += countTokens(line[i*line_per_chip+j]); 
        }
    }
}

void PowerBudget::reclaim(uint64_t current_time){
    for(size_t i=0; i<=NUM_BANKS-1; ++i){
        if((current_time >= issued_requests[i].completed_time) 
                && (issued_requests[i].completed_time > 0)){
            reclaimLine(issued_requests[i].line);
            for(size_t j=0; j<=line_num-1; ++j)
                issued_requests[i].line[j] = 0;     
            issued_requests[i].completed_time = 0;
        }
    }
}

