#ifndef POWERBUDGET_H
#define POWERBUDGET_H

//PowerBudget.h
//by scyu

#include "SystemConfiguration.h"
#include "BusPacket.h"
#include <cstring>
#include <sstream>

using namespace std;

namespace DRAMSim
{
class PowerBudget
{
    uint16_t chip_budget;
    int32_t* token;
    int16_t line_num;
   
    typedef struct request{
        uint64_t* token;
        uint64_t completed_time;
    } Request;
    
    Request* issued_requests;    
public:
    PowerBudget();
    PowerBudget(uint16_t budget);
    void setBudget(uint16_t budget);
    bool issuable(uint64_t* line);
    bool issuableAfterShifting(BusPacket* req, vector<BusPacket *> &victims);
    bool issuableFWC(uint64_t* allocated_token, bool* need_more_iter);
    void doFWC(uint64_t* allocated_token, bool* need_more_iter);
    bool shiftSubReq(BusPacket** req, vector<BusPacket *> &queue, bool do_shift);
    bool consume(uint64_t* line, size_t bank_no, uint64_t completed_time);
    void reclaimLine(uint64_t* line);
    void reclaim(uint64_t curr_time);
    uint32_t inline countTokens(uint64_t sub_line);
    string dumpBudgetStatus(uint64_t* allocated_token);
    string dumpRequestStatus(uint64_t* line);
    string dumpBalanceStatus(void);
    void mappingFunction(uint64_t* line, uint64_t allocated_token[]);
    void mappingFunction(uint64_t* line, uint64_t** allocated_token, uint8_t iter);
    float countPriority(uint64_t* line);
    uint8_t getHotChips(uint64_t* line, bool* hot_chips);
    uint8_t getIterations(uint64_t* line);
};
}


#endif
