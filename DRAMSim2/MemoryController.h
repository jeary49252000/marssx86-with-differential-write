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


#ifndef MEMORYCONTROLLER_H
#define MEMORYCONTROLLER_H

//MemoryController.h
//
//Header file for memory controller object
//

#include "SimulatorObject.h"
#include "Transaction.h"
#include "SystemConfiguration.h"
#include "CommandQueue.h"
#include "BusPacket.h"
#include "BankState.h"
#include "Rank.h"
#include "CSVWriter.h"
#include <map>
// scyu: NO_SUB_REQUEST
#define NO_SUB_REQUEST 0

using namespace std;

namespace DRAMSim
{
class MemorySystem;
class MemoryController : public SimulatorObject
{

public:
	//functions
	MemoryController(MemorySystem* ms, CSVWriter &csvOut_, ostream &dramsim_log_);
	virtual ~MemoryController();

    // scyu: add differential write information
    vector<Rank *>* getRank(){return ranks;}
	// laisky: analyze the scheduling problem [why two way is better than one way]
	uint64_t SumMaxToken1stD;
	uint64_t SumMaxToken2ndD;
	uint64_t MaxMaxToken1stD;
	uint64_t MaxMaxToken2ndD;
	// laisky: analyze the token shift
	uint64_t tokenShift;
	// laisky: trace the power utilization
	double rank1_utilization;
	double rank2_utilization;
	double countTimes1;
	double countTimes2;
	double rank1_max_utilization;
	double rank2_max_utilization;
	double rank1_range_utilization[10];
	double rank2_range_utilization[10];


	bool addTransaction(Transaction *trans);
	bool WillAcceptTransaction();
	bool WillAcceptTransaction(unsigned rank, unsigned bank, bool isWrite);
	void returnReadData(const Transaction *trans);
	void receiveFromBus(BusPacket *bpacket);
	void attachRanks(vector<Rank *> *ranks);
	void update();
	void printStats(bool finalStats = false);
	void simulation_start();
	void getDramStats(string &sb);
    void resetStats(); 
    void resetHistory();
	//fields
	vector<Transaction *> transactionQueue;
    unsigned RequestAcceptCnt;
    unsigned RequestRejectCnt;
private:
	ostream &dramsim_log;
	vector< vector <BankState> > bankStates;
	//functions
	void insertHistogram(unsigned latencyValue, unsigned rank, unsigned bank);

	//fields
	MemorySystem *parentMemorySystem;

	CommandQueue commandQueue;

	BusPacket *poppedBusPacket;
	vector<unsigned>refreshCountdown;
	vector<BusPacket *> writeDataToSend;
	vector<unsigned> writeDataCountdown;
	vector<Transaction *> returnTransaction;
	vector<Transaction *> pendingReadTransactions;
	map<unsigned,unsigned> latencies; // latencyValue -> latencyCount
	vector<bool> powerDown;

	vector<Rank *> *ranks;

	//output file
	CSVWriter &csvOut; 

	// these packets are counting down waiting to be transmitted on the "bus"
	BusPacket *outgoingCmdPacket;
	unsigned cmdCyclesLeft;
	BusPacket *outgoingDataPacket;
	unsigned dataCyclesLeft;

	uint64_t totalTransactions;
	vector<uint64_t> grandTotalBankAccesses; 
	vector<uint64_t> totalReadsPerBank;
	vector<uint64_t> totalWritesPerBank;

	vector<uint64_t> totalReadsPerRank;
	vector<uint64_t> totalWritesPerRank;


	vector< uint64_t > totalEpochLatency;

	unsigned channelBitWidth;
	unsigned rankBitWidth;
	unsigned bankBitWidth;
	unsigned rowBitWidth;
	unsigned colBitWidth;
	unsigned byteOffsetWidth;


	unsigned refreshRank;
	
    uint64_t totalWriteLatency;
    uint64_t totalReadLatency;
    uint64_t totalWriteCount;
    uint64_t totalReadCount;
   
    uint64_t totalRefreshCount;
    uint64_t totalRefreshLatency;

    // scyu: add differential write information 
    uint64_t totalBitSetCount;
    uint64_t totalBitResetCount;
    
    uint64_t transactionID;
    

    map<int, int> writeIterDistribution;
    map<int, int> refreshIterDistribution;

    bool detailSim;
    bool writeBarrier;
public:
	// energy values are per rank -- SST uses these directly, so make these public 
	vector< uint64_t > backgroundEnergy;
	vector< uint64_t > burstEnergy;
	vector< uint64_t > actpreEnergy;
	vector< uint64_t > refreshEnergy;

};
}

#endif

