#include "PowerBudget.h"
//#include <bitset>

using namespace DRAMSim;


PowerBudget::PowerBudget(){
    token = (int32_t*) calloc(NUM_CHIPS,sizeof(int32_t));
    issued_requests = (Request*) calloc(NUM_BANKS,sizeof(Request));
}

PowerBudget::PowerBudget(uint16_t budget){
    token = (int32_t*) malloc(NUM_CHIPS*sizeof(int32_t));
    issued_requests = (Request*) calloc(NUM_BANKS,sizeof(Request));
    setBudget(budget);
}

void PowerBudget::setBudget(uint16_t budget){
    for(size_t i=0; i<=NUM_CHIPS-1; ++i)
        token[i] = budget;
}

bool PowerBudget::consume(uint64_t line, size_t bank_no, uint64_t completed_time){
    uint8_t size_row_per_chip = SIZE_ROW/NUM_CHIPS;
    bool    out_of_token = false;

    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        token[i] -= countTokens((line << size_row_per_chip*i) >> size_row_per_chip*(NUM_CHIPS-1)); 
        out_of_token |= token[i] < 0;
    }
    if(out_of_token){
        reclaimLine(line);
        return false;
    }
    issued_requests[bank_no].line = line;     
    issued_requests[bank_no].completed_time = completed_time;     
    return true;
}

uint32_t inline PowerBudget::countTokens(uint64_t sub_line){
    uint32_t counter;
    for(counter = 0; sub_line; counter++)  
        sub_line &= sub_line - 1; 
    return counter;
}

bool PowerBudget::issuable(uint64_t line){
    uint8_t size_row_per_chip = SIZE_ROW/NUM_CHIPS;
    bool    out_of_token = false;
    
    //cerr << std::bitset<64>(line) << endl;
    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        //cerr << token[i] << " " << countTokens((line << size_row_per_chip*i) >> size_row_per_chip*(NUM_CHIPS-1)) << endl;
        out_of_token |= (token[i] < (int64_t) countTokens((line << size_row_per_chip*i) >> size_row_per_chip*(NUM_CHIPS-1)));
    }
    return !out_of_token;
}

void PowerBudget::reclaimLine(uint64_t line){
    uint8_t size_row_per_chip = SIZE_ROW/NUM_CHIPS;
    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        token[i] += countTokens((line << size_row_per_chip*i) >> size_row_per_chip*(NUM_CHIPS-1)); 
    }
}

void PowerBudget::reclaim(uint64_t current_time){
    for(size_t i=0; i<=NUM_BANKS-1; ++i){
        if(current_time >= issued_requests[i].completed_time 
                && issued_requests[i].completed_time > 0){
            reclaimLine(issued_requests[i].line);
            issued_requests[i].line = 0;
            issued_requests[i].completed_time = 0;
            //cerr << "reclaim some line" << endl;
        }
    }
}

