#include "PowerBudget.h"
//#include <bitset>

using namespace DRAMSim;

#define DEBUG 0
#define MAX_BITS_TO_SHIFT 16
#define DYNAMIC_DIVISION 1
#define ISSUE_LESS 1
#define ISSUE_MORE 1
#define SHARED_BUDGET 0
#define BYTE_INTERLEAVING 0

PowerBudget::PowerBudget(){
    line_num = (LINE_SIZE>>3)/SUB_REQUEST_COUNT;
    token = (int32_t*) calloc(NUM_CHIPS,sizeof(int32_t));
    issued_requests = (Request*) calloc(NUM_BANKS,sizeof(Request));
    for(size_t i=0; i<=NUM_BANKS-1; ++i)
        issued_requests[i].token = (uint64_t*) calloc(line_num,sizeof(uint64_t));
}

PowerBudget::PowerBudget(uint16_t budget){
    line_num = (LINE_SIZE>>3)/SUB_REQUEST_COUNT;
    token = (int32_t*) malloc(NUM_CHIPS*sizeof(int32_t));
    issued_requests = (Request*) calloc(NUM_BANKS,sizeof(Request));
    for(size_t i=0; i<=NUM_BANKS-1; ++i)
        issued_requests[i].token = (uint64_t*) calloc(line_num,sizeof(uint64_t));
    setBudget(budget);

    if(POWER_BUDGETING)
        cout << "POWER BUDGETING" << endl;
    if(BUDGET_AWARE_SCHEDULE){
        cout << "POWER BUDGET AWARE SCHEDULING ON" << endl;
    }
    if(FLEXIBLE_WRITE_CONFIG){
        cout << "FLEXIBLE_WRITE_CONFIGURATION ON" << endl;
    }
#if SHARED_BUDGET
    cout << "SHARED POWER BUDGET CONFIGURATION" << endl;
#endif

#if DYNAMIC_DIVISION
    cout << "DYNAMIC CDIVISION" << endl;
#else
    cout << "STATIC DIVISION" << endl;
#endif

#if ISSUE_MORE
    cout << "ISSUE MORE" << endl;
#endif

#if ISSUE_LESS
    cout << "ISSUE LESS" << endl;
#endif
}

void PowerBudget::setBudget(uint16_t budget){
    chip_budget = budget;
    for(size_t i=0; i<=NUM_CHIPS-1; ++i)
        token[i] = budget;
}

bool PowerBudget::consume(uint64_t* allocated_token, size_t bank_no, uint64_t completed_time){
    bool    out_of_token = false;

#if DEBUG
    cout << "consume:\t" << dumpBudgetStatus(allocated_token) << endl;
#endif

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
        reclaimLine(allocated_token);
        return false;
    }
    for(size_t i=0; i<=line_num-1; ++i)
        issued_requests[bank_no].token[i] = allocated_token[i];     
    issued_requests[bank_no].completed_time = completed_time;     
    return true;
}

uint32_t inline PowerBudget::countTokens(uint64_t sub_line){
    uint32_t counter;
    for(counter = 0; sub_line; counter++)  
        sub_line &= sub_line - 1; 
    return counter;
}

bool PowerBudget::issuable(uint64_t* allocated_token){
    bool   out_of_token = false;

#if SHARED_BUDGET
    uint32_t total_allocated = 0;
    uint32_t remain_token = 0;
    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        total_allocated += allocated_token[i];
        remain_token += token[i];
    }
    out_of_token = total_allocated > remain_token;
#else
    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        out_of_token |= (allocated_token[i] > token[i]);
        //cout << i << ": " << token[i] << "(" << allocated_token[i] << ") " << endl; 
    }
#endif

#if DEBUG
    if(out_of_token){
        //cout << "out of budget:\t" << dumpBudgetStatus(allocated_token) << endl;
        //PRINT("out of budget:\t" << dumpBudgetStatus() << endl);
    }
#endif
    return !out_of_token;
}

bool PowerBudget::issuableFWC(uint64_t* allocated_token, bool* need_more_iter){
    uint64_t two_iter[NUM_CHIPS]; 
    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        two_iter[i] = allocated_token[i]/2; // assume equally distributed into 2 iterations
    }
    //return (issuable(allocated_token));
    if(issuable(allocated_token)){
        *need_more_iter = false;
        return true;
    }else if(issuable(two_iter)){
        *need_more_iter = true;
        cout << "issuable via two_iter" << endl;
        return true;
    }
    return false;
}
 
void PowerBudget::doFWC(uint64_t* allocated_token, bool* need_more_iter){
    uint64_t two_iter[NUM_CHIPS]; 
    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        two_iter[i] = allocated_token[i]/2; // assume equally distributed into 2 iterations
    }
    if(issuable(allocated_token)){
        *need_more_iter = false;
    }else if(issuable(two_iter)){
        *need_more_iter = true;
        for(size_t i=0; i<=NUM_CHIPS-1; ++i){
            allocated_token[i] = two_iter[i]; 
        }
    }
}

void PowerBudget::reclaimLine(uint64_t* allocated_token){
#if DEBUG
    cout << "reclaim:\t" << dumpBudgetStatus(allocated_token) << endl; 
#endif
    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        token[i] += allocated_token[i]; 
    }
}

void PowerBudget::reclaim(uint64_t current_time){
    for(size_t i=0; i<=NUM_BANKS-1; ++i){
        if((current_time >= issued_requests[i].completed_time) 
                && (issued_requests[i].completed_time > 0)){
            reclaimLine(issued_requests[i].token);
            for(size_t j=0; j<=line_num-1; ++j)
                issued_requests[i].token[j] = 0;     
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

string PowerBudget::dumpRequestStatus(uint64_t* allocated_token){
    stringstream ret;

    uint64_t max = 0;
    size_t ind = 0;

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

bool PowerBudget::issuableAfterShifting(BusPacket* req, vector<BusPacket *> &queue){
    
#if DYNAMIC_DIVISION 
    return (issuable(req->token) || shiftSubReq(&req, queue, false));
#else
    return issuable(req->token);
#endif
}

// do_shift = false: try issuable after shifting or not, not do the shifting immediately  
bool PowerBudget::shiftSubReq(BusPacket** req, vector<BusPacket *> &queue, bool do_shift){
    vector<size_t> victim_index;
    uint64_t loan[NUM_CHIPS];
    bool need_shift = false;
    const static uint8_t lower_bound_to_issue_any_other_request = 24;
    //const static uint8_t lower_bound_to_issue_any_other_request = 48;

    // determine whether there is a need to shift
    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        loan[i] = ((*req)->token[i] > token[i])? (*req)->token[i] - token[i] : 0;
#if DYNAMIC_DIVISION
        need_shift |= loan[i] > 0;
#endif
    }

    // no need to shift
    if(!need_shift){    
        // "issue more" case
        // find victims
        for(size_t i=0; i<=queue.size()-1; ++i){
            if((*req) == queue[i]){

            }else if(queue[i]->busPacketType == WRITE_P && queue[i]->transID == (*req)->transID){
                victim_index.push_back(i); 
            }
        }
        if(victim_index.size() == 0){
            return true;
        }
        // try loaning to the other
        // FIXME: only try the last one now
        bool could_shift = false;
        for(size_t i=0; i<=NUM_CHIPS-1; ++i){
            if(token[i] > lower_bound_to_issue_any_other_request){
                // preserve those budget for other requests
                loan[i] = 0;
                continue;
            }
            loan[i] = (token[i] > (*req)->token[i])? token[i] - (*req)->token[i] : 0;
            loan[i] = (loan[i] > queue[victim_index.back()]->token[i])? queue[victim_index.back()]->token[i] : loan[i];
            // check upper bound
            loan[i] = (loan[i] > MAX_BITS_TO_SHIFT)? MAX_BITS_TO_SHIFT : loan[i];
#if DYNAMIC_DIVISION && ISSUE_MORE 
            could_shift |= loan[i] > 0;
#endif
        }
        if(could_shift && do_shift){ 
            // do shifting
            for(size_t i=0; i<=NUM_CHIPS-1; ++i){
                //loan[i] = loan[i]/2; // tunable 
                (*req)->token[i] += loan[i];
                queue[victim_index.back()]->token[i] -= loan[i];
            }   
            //cout << "issue more" << endl;
        }
    }else{  
        // "issue less" case
        // find victims
        for(size_t i=0; i<=queue.size()-1; ++i){
            if((*req) == queue[i]){

            }else if(queue[i]->busPacketType == WRITE_P && queue[i]->transID == (*req)->transID){
                victim_index.push_back(i); 
            }
        }

#if DYNAMIC_DIVISION && !ISSUE_LESS
    return false;
#endif

        if(victim_index.size() == 0){
            // fail to find any victim
            return false;
        }

        // check if any victim would be assigned too many #token after shift
        bool victim_out_of_budget = false;
        for(size_t i=0; i<=victim_index.size()-1; ++i){
            for(size_t j=0; j<=NUM_CHIPS-1; ++j){
                victim_out_of_budget |= ((queue[victim_index[i]]->token[j] + loan[j]/victim_index.size()) > chip_budget);
            }
        }
        if(victim_out_of_budget){
            // need shift but fail to shift
            return false;
        }else if(do_shift){
                // check upper bound
            for(size_t j=0; j<=NUM_CHIPS-1; ++j){
                if(loan[j] > MAX_BITS_TO_SHIFT){
                    return false;
                }
            }
            // do shift
            for(size_t j=0; j<=NUM_CHIPS-1; ++j){
                (*req)->token[j] -= loan[j]; 
            }
            for(size_t i=0; i<=victim_index.size()-1; ++i){
                for(size_t j=0; j<=NUM_CHIPS-1; ++j){
                    //cout << queue[victim_index[i]]->token[j] << " + " << loan[j] / victim_index.size() << " = " ; 
                    queue[victim_index[i]]->token[j] += loan[j] / victim_index.size();
                    //cout << queue[victim_index[i]]->token[j] << endl;
                }
            }
        }
        // let last one take over the remainder tokens
        /*
           for(size_t j=0; j<=NUM_CHIPS-1; ++j){
           queue[victim_index.back()]->token[j] += loan[j] % victim_index.size();
           }
           */
        //cout << "issue less" << endl;
    }
    return true;
}

float PowerBudget::countPriority(uint64_t* allocated_token){

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
    float balance_utilization_product = (balance_metric * utilization) / (chip_budget *NUM_CHIPS);

    //return max - sum / NUM_CHIPS;
    //return balance_utilization_product;
    return balance_utilization_product + (issuable(allocated_token)? 1:0);
    //return 1;
    //return balance_metric
}

uint8_t PowerBudget::getHotChips(uint64_t* allocated_token, bool* hot_chips){
    uint8_t  counter = 0;

    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        hot_chips[i] = false;
        if(allocated_token[i] > token[i]){
            hot_chips[i] = true;
            counter++;
        }
    }
    return counter;
}

uint8_t PowerBudget::getIterations(uint64_t* allocated_token){
    uint8_t iter = 0;
    uint8_t max_iter = 0;

    for(size_t i=0; i<=NUM_CHIPS-1; ++i){
        if((iter = allocated_token[i]/chip_budget) > max_iter){
            max_iter = iter;
        }    
    }
    return iter;
}
