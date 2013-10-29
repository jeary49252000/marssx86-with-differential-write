#ifndef POWERBUDGET_H
#define POWERBUDGET_H

//PowerBudget.h
//by scyu

#include "SystemConfiguration.h"
#include <cstring>

using namespace std;

namespace DRAMSim
{
class PowerBudget
{
    int32_t* token;    
    
    typedef struct request{
        uint64_t line;
        uint64_t completed_time;
    } Request;
    
    Request* issued_requests;    
public:
    PowerBudget();
    PowerBudget(uint16_t budget);
    void setBudget(uint16_t budget);
    bool issuable(uint64_t line);
    bool consume(uint64_t line, size_t bank_no, uint64_t completed_time);
    void reclaimLine(uint64_t line);
    void reclaim(uint64_t curr_time);
    uint32_t inline countTokens(uint64_t sub_line);
};
}


#endif
