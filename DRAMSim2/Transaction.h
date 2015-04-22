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

#ifndef TRANSACTION_H
#define TRANSACTION_H

//Transaction.h
//
//Header file for transaction object

#include "SystemConfiguration.h"
#include "BusPacket.h"

using std::ostream; 

namespace DRAMSim
{
enum TransactionType
{
	DATA_READ,
	DATA_WRITE,
	RETURN_DATA
};

class Transaction
{
	Transaction();
public:
	//fields
	TransactionType transactionType;
	uint64_t address;
	void *data;
	// scyu: add differential write information
    bool isSubReq;
    uint8_t iteration;
    uint64_t token[NUM_CHIPS]; 
    uint64_t diffMask[(LINE_SIZE)>>3]; 
    uint64_t transID;
    uint64_t timeAdded;
	uint64_t timeReturned;

	// laisky: counter for commit
	uint64_t * counter;
	// laisky: trace the power utilization for BaseLine
	uint64_t real_token[NUM_CHIPS];
	// laisky: use same bits shifting
	uint64_t sub_mask[(LINE_SIZE>>3)/SUB_REQUEST_COUNT];

	void copyMask(uint64_t *mask);
	friend ostream &operator<<(ostream &os, const Transaction &t);
	//functions
	Transaction(TransactionType transType, uint64_t addr, void *data);
	Transaction(TransactionType transType, uint64_t addr, void *data, uint64_t diff_mask[]);
	Transaction(TransactionType transType, uint64_t addr, void *data, uint64_t allocated_token[], bool is_sub_request, uint8_t iter);
	Transaction(const Transaction &t);

	BusPacketType getBusPacketType()
	{
		switch (transactionType)
		{
			case DATA_READ:
			if (rowBufferPolicy == ClosePage)
			{
				return READ_P;
			}
			else if (rowBufferPolicy == OpenPage)
			{
				return READ; 
			}
			else
			{
				ERROR("Unknown row buffer policy");
				abort();
			}
			break;
		case DATA_WRITE:
			if (rowBufferPolicy == ClosePage)
			{
				return WRITE_P;
			}
			else if (rowBufferPolicy == OpenPage)
			{
				return WRITE; 
			}
			else
			{
				ERROR("Unknown row buffer policy");
				abort();
			}
			break;
		default:
			ERROR("This transaction type doesn't have a corresponding bus packet type");
			abort();
		}
	}
};

}

#endif

