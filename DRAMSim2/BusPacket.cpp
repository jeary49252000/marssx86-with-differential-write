/*********************************************************************************
*  Copyright (c) 2010-2011, Elliott Cooper-Balis
*                             Paul Rosenfeld
*                             Bruce Jacob
*                             University of Maryland 
*                             dramninjas [at] gmail [dot] com
*  All rights reserved.
*  
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*  
*     * Redistributions of source code must retain the above copyright notice,
*        this list of conditions and the following disclaimer.
*  
*     * Redistributions in binary form must reproduce the above copyright notice,
*        this list of conditions and the following disclaimer in the documentation
*        and/or other materials provided with the distribution.
*  
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
*  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************/








//BusPacket.cpp
//
//Class file for bus packet object
//

#include "BusPacket.h"
#include "Iteration.h"

using namespace DRAMSim;
using namespace std;

unsigned getIteration(unsigned retention_level, BusPacketType type){

    int s = rand() % 131072;

    switch(type){
        case WRITE:
        case WRITE_P:
            for(int i=0; i<maxiter; i++){
                if(s < iterCdf_32[retention_level][i]){
                    return iterValue_32[retention_level][i];
                }
            }
            break;
        case REFRESH:
            if(SMART_REFRESH_ENABLE){
                for(int i=0; i<maxiter; i++){
                    if(s < iterCdf_93[retention_level][i]){
                        return iterValue_93[retention_level][i];
                    }
                }
            }else{
                for(int i=0; i<maxiter; i++){
                    if(s < iterCdf_256[retention_level][i]){
                        return (iterValue_256[retention_level][i]) * 4;
                    }
                }
            }
            break;
        default:
            ERROR("getIteration(): unexpected bus packet type");
            exit(0);
            break;
    }

    ERROR("getIteration(): unexpected itertions");
    exit(0);
}

BusPacket::BusPacket(BusPacketType packtype, uint64_t physicalAddr, 
		unsigned col, unsigned rw, unsigned r, unsigned b, void *dat, 
		ostream &dramsim_log_, uint64_t time) :
	dramsim_log(dramsim_log_),
	busPacketType(packtype),
	column(col),
	row(rw),
	bank(b),
	rank(r),
	physicalAddress(physicalAddr),
	data(dat),
    timeAdded(time)
{
    switch(busPacketType){
        case WRITE:
        case WRITE_P:
        case REFRESH:
            //iterations = getIteration(RETENTION_LEVEL, busPacketType);
            iterations = 1;
            break;
    }
	// laisky
	counter = NULL;
	shifted = false;
}

// scyu: add differential write information
BusPacket::BusPacket(BusPacketType packtype, uint64_t physicalAddr, 
		unsigned col, unsigned rw, unsigned r, unsigned b, void *dat, uint64_t allocated_token[], unsigned iter,
		ostream &dramsim_log_, uint64_t time) :
	dramsim_log(dramsim_log_),
	busPacketType(packtype),
	column(col),
	row(rw),
	bank(b),
	rank(r),
	physicalAddress(physicalAddr),
	data(dat),
    timeAdded(time)
{
    switch(busPacketType){
        case WRITE:
        case WRITE_P:
        case REFRESH:
            //iterations = getIteration(RETENTION_LEVEL, busPacketType);
            iterations = 1;
            break;
    }

    // scyu: cpoy allocated_token only when there is a legal diff mask
    if(allocated_token != NULL){
        for(size_t i=0; i<=NUM_CHIPS-1; ++i){
           token[i] = allocated_token[i];
        }
    }
    // scyu: set sub-request ID
    subReqID = iter;
	// laisky
	counter = NULL;
	shifted = false;
}

// scyu: add differential write information
BusPacket::BusPacket(BusPacketType packtype, uint64_t physicalAddr, 
		unsigned col, unsigned rw, unsigned r, unsigned b, void *dat, uint64_t allocated_token[], unsigned iter, uint64_t id,
		ostream &dramsim_log_, uint64_t time) :
	dramsim_log(dramsim_log_),
	busPacketType(packtype),
	column(col),
	row(rw),
	bank(b),
	rank(r),
	physicalAddress(physicalAddr),
	data(dat),
    transID(id),
    timeAdded(time)
{
    switch(busPacketType){
        case WRITE:
        case WRITE_P:
        case REFRESH:
            //iterations = getIteration(RETENTION_LEVEL, busPacketType);
            iterations = 1;
            break;
    }

    // scyu: cpoy allocated_token only when there is a legal diff mask
    if(allocated_token != NULL){
        for(size_t i=0; i<=NUM_CHIPS-1; ++i){
           token[i] = allocated_token[i];
        }
    }
    // scyu: set sub-request ID
    subReqID = iter;
	// laisky
	counter = NULL;
	shifted = false;
}

void BusPacket::copyMask(uint64_t * mask) {
	for (size_t i = 0; i < ((LINE_SIZE>>3)/SUB_REQUEST_COUNT); ++i) {
		sub_mask[i] = mask[i];
	}
}


/*
void BusPacket::copyTokens(uint64_t ** tokens) {
	for (size_t i = 0; i <= SUB_REQUEST_COUNT -1; ++i) {
		for (size_t j = 0; j <=NUM_CHIPS - 1; ++j) {
			this->tokens[i][j] = tokens[i][j];
		}
	}
}*/

void BusPacket::print(uint64_t currentClockCycle, bool dataStart)
{
	if (this == NULL)
	{
		return;
	}

	if (VERIFICATION_OUTPUT)
	{
		switch (busPacketType)
		{
		case READ:
			cmd_verify_out << currentClockCycle << ": read ("<<rank<<","<<bank<<","<<column<<",0);"<<endl;
			break;
		case READ_P:
			cmd_verify_out << currentClockCycle << ": read ("<<rank<<","<<bank<<","<<column<<",1);"<<endl;
			break;
		case WRITE:
			cmd_verify_out << currentClockCycle << ": write ("<<rank<<","<<bank<<","<<column<<",0 , 0, 'h0);"<<endl;
			break;
		case WRITE_P:
			cmd_verify_out << currentClockCycle << ": write ("<<rank<<","<<bank<<","<<column<<",1, 0, 'h0);"<<endl;
			break;
		case ACTIVATE:
			cmd_verify_out << currentClockCycle <<": activate (" << rank << "," << bank << "," << row <<");"<<endl;
			break;
		case PRECHARGE:
			cmd_verify_out << currentClockCycle <<": precharge (" << rank << "," << bank << "," << row <<");"<<endl;
			break;
		case REFRESH:
			cmd_verify_out << currentClockCycle <<": refresh (" << rank << ");"<<endl;
			break;
		case DATA:
			//TODO: data verification?
			break;
		default:
			ERROR("Trying to print unknown kind of bus packet");
			exit(-1);
		}
	}
}
void BusPacket::print()
{
	if (this == NULL) //pointer use makes this a necessary precaution
	{
		return;
	}
	else
	{
		switch (busPacketType)
		{
		case READ:
			PRINT("BP [READ] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"]");
			break;
		case READ_P:
			PRINT("BP [READ_P] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"]");
			break;
		case WRITE:
			PRINT("BP [WRITE] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"]");
			break;
		case WRITE_P:
			PRINT("BP [WRITE_P] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"]");
			break;
		case ACTIVATE:
			PRINT("BP [ACT] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"]");
			break;
		case PRECHARGE:
			PRINT("BP [PRE] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"]");
			break;
		case REFRESH:
			PRINT("BP [REF] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"]");
			break;
		case DATA:
			PRINTN("BP [DATA] pa[0x"<<hex<<physicalAddress<<dec<<"] r["<<rank<<"] b["<<bank<<"] row["<<row<<"] col["<<column<<"] data["<<data<<"]=");
			printData();
			PRINT("");
			break;
		default:
			ERROR("Trying to print unknown kind of bus packet");
			exit(-1);
		}
	}
}

void BusPacket::printData() const 
{
	if (data == NULL)
	{
		PRINTN("NO DATA");
		return;
	}
	PRINTN("'" << hex);
	for (int i=0; i < 4; i++)
	{
		PRINTN(((uint64_t *)data)[i]);
	}
	PRINTN("'" << dec);
}


// laisky: analyze the scheduling problem [why two way is better than one way]
uint64_t BusPacket::getMaxToken() {
	uint64_t max = 0;
	for(size_t i=0; i<=NUM_CHIPS-1; ++i) {
		if (max < token[i])
			max = token[i];
	}
	return max;
}

void BusPacket::copy_token(uint64_t real_token[]) {
	for (size_t i = 0; i <= NUM_CHIPS-1; ++i) {
		this->real_token[i] = real_token[i];
	}
}
