#include "PowerBudget.h"
//#include <bitset>

using namespace DRAMSim;

#define DEBUG 0

#define SHARED_BUDGET 0

#define BYTE_INTERLEAVING 0

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
    chip_budget = budget;
    for(size_t i=0; i<=NUM_CHIPS-1; ++i)
        token[i] = budget;
}

bool PowerBudget::consume(uint64_t* line, size_t bank_no, uint64_t completed_time){
    uint64_t allocated_token[NUM_CHIPS];
    bool    out_of_token = false;

    mappingFunction(line, allocated_token);

#if SHARED_BUDGET
    uint32_t remain_token = 0;
    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        token[i] -= allocated_token[i];
        remain_token += token[i];
    }
    out_of_token = remain_token < 0;
#else
    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        token[i] -= allocated_token[i];
        if(token[i] <= 0)
            out_of_token = true;
    }
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
    uint64_t allocated_token[NUM_CHIPS];
    bool   out_of_token = false;

    mappingFunction(line, allocated_token);

#if SHARED_BUDGET
    uint32_t total_allocated = 0;
    uint32_t remain_token = 0;
    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        total_allocated += allocated_token[i];
        remain_token += token[i];
    }
    out_of_token = total_allocated > remain_token;
#else
    for(size_t i=0; i<=NUM_CHIPS-1; ++i)
        out_of_token |= (allocated_token[i] > token[i]);
#endif

#if DEBUG
    if(out_of_token){
        cout << "out of budget:\t" << dumpBudgetStatus(allocated_token) << endl;
        //PRINT("out of budget:\t" << dumpBudgetStatus() << endl);
    }
#endif
    return !out_of_token;
}

void PowerBudget::reclaimLine(uint64_t* line){
    uint64_t allocated_token[NUM_CHIPS];

    mappingFunction(line, allocated_token);
    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        token[i] += allocated_token[i]; 
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


string PowerBudget::dumpBudgetStatus(uint64_t* allocated_token){
    stringstream ret;
    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        ret << token[i] << "(" << allocated_token[i] << ") ";  
    }
    return ret.str();
}

string PowerBudget::dumpRequestStatus(uint64_t* line){
    uint64_t allocated_token[NUM_CHIPS];
    stringstream ret;

    uint64_t max = 0;
    size_t ind = 0;

    mappingFunction(line, allocated_token);
    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        //ret << token[i] << "(" << allocated_token[i] << ") ";  
        ret << allocated_token[i] << "\t";  
        if(allocated_token[i] > max){
            max = allocated_token[i];
            ind = i;
        }
    }
    ret << ind;
    return ret.str();
}
string PowerBudget::dumpBalanceStatus(void){
    stringstream ret;

    int32_t sum = 0;
    int32_t max = 0;
    int32_t loading = 0;
    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        loading = (chip_budget - token[i]);
        if(loading > max)
            max = loading;
        sum += loading;
    }
    if(max > 0){
        ret << (((float) sum / NUM_CHIPS) / (float) max);
    }else{
        ret << "1";
    }
    return ret.str();
}

void PowerBudget::mappingFunction(uint64_t* line, uint64_t allocated_token[]){
    for(size_t i=0; i<=NUM_CHIPS-1; ++i)
        allocated_token[i] = 0;

#if BYTE_INTERLEAVING
    for(size_t i=0; i<=line_num-1; ++i){
        uint64_t buf = line[i];
        for(size_t j=0; j<=7; ++j){
            for(size_t k=0; k<=7; ++k, buf = buf >> 1){
                allocated_token[j % NUM_CHIPS] += buf & 0x1;
            }
        }
    }
#else
    size_t line_per_chip = line_num/NUM_CHIPS;
    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        for(size_t j=0; j<= line_per_chip-1; ++j){
            allocated_token[i] += countTokens(line[i*line_per_chip+j]); 
        }
    }
#endif
}

float PowerBudget::countPriority(uint64_t* line){
    uint64_t allocated_token[NUM_CHIPS];
    
    mappingFunction(line, allocated_token);

    // count balance metric: AVERAGE(loading)/MAX(loading)
    // count utilization: SUM(allocated_token)
    int32_t sum = 0;
    int32_t max = 0;
    int32_t loading = 0;
    int32_t utilization = 0;
    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        loading = (chip_budget - token[i]) + allocated_token[i];
        if(loading > max)
            max = loading;
        sum += loading;
        utilization += allocated_token[i];
    }

    float balance_metric = ((float) sum / NUM_CHIPS) / (float) max;
    float balance_utilization_product = balance_metric * utilization / (chip_budget *NUM_CHIPS);

    //return max - sum / NUM_CHIPS;
    return balance_utilization_product;
    //return balance_metric;
}

uint8_t PowerBudget::getHotChips(uint64_t* line, bool* hot_chips){
    uint8_t  counter = 0;
    uint64_t allocated_token[NUM_CHIPS];
    mappingFunction(line, allocated_token);

    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        hot_chips[i] = false;
        if(allocated_token[i] > token[i]){
            hot_chips[i] = true;
            counter++;
        }
    }

    return counter;
}
