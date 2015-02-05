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








//CommandQueue.cpp
//
//Class file for command queue object
//

#include "CommandQueue.h"
#include "MemoryController.h"
#include <assert.h>


using namespace DRAMSim;

CommandQueue::CommandQueue(vector< vector<BankState> > &states, ostream &dramsim_log_) :
		dramsim_log(dramsim_log_),
		bankStates(states),
		nextBank(0),
		nextRank(0),
		nextBankPRE(0),
		nextRankPRE(0),
		refreshRank(0),
		refreshWaiting(false),
		sendAct(true)
{
	//set here to avoid compile errors
	currentClockCycle = 0;

	//use numBankQueus below to create queue structure
	size_t numBankQueues;
	if (queuingStructure==PerRank)
	{
		numBankQueues = 1;
	}
	else if (queuingStructure==PerRankPerBank)
	{
		numBankQueues = NUM_BANKS;
	}
	else
	{
		ERROR("== Error - Unknown queuing structure");
		exit(0);
	}

	//vector of counters used to ensure rows don't stay open too long
	rowAccessCounters = vector< vector<unsigned> >(NUM_RANKS, vector<unsigned>(NUM_BANKS,0));

	//create queue based on the structure we want
	BusPacket1D actualQueue;
	BusPacket2D perBankQueue = BusPacket2D();
	queues = BusPacket3D();
	for (size_t rank=0; rank<NUM_RANKS; rank++)
	{
		//this loop will run only once for per-rank and NUM_BANKS times for per-rank-per-bank
		for (size_t bank=0; bank<numBankQueues; bank++)
		{
			actualQueue	= BusPacket1D();
			perBankQueue.push_back(actualQueue);
		}
		queues.push_back(perBankQueue);
	}


	//FOUR-bank activation window
	//	this will count the number of activations within a given window
	//	(decrementing counter)
	//
	//countdown vector will have decrementing counters starting at tFAW
	//  when the 0th element reaches 0, remove it
	tFAWCountdown.reserve(NUM_RANKS);
	for (size_t i=0;i<NUM_RANKS;i++)
	{
		//init the empty vectors here so we don't seg fault later
		tFAWCountdown.push_back(vector<unsigned>());
	}

    //initialize ReadReqNum, WriteReqNum --rs
    if(NUM_RANKS > MAX_RANKS || NUM_BANKS > MAX_BANKS){
        ERROR("== Error - Please modify MAX_RANKS, MAX_BANKS");
        exit(0);
    }

    for(int i=0; i<NUM_RANKS; i++){
        for(int j=0; j<NUM_BANKS; j++){
            ReadReqNum[i][j] = 0;
            WriteReqNum[i][j] = 0;
            WriteBurst[i][j] = false;
            WriteInProgressed[i][j] = false;
            WriteBurstStartCycle[i][j] = 0;
            WriteBurstTotalCycle[i][j] = 0;
        }
    }
    num_read_first  = 0;
    num_write_first = 0;
    queuedWriteCnt = 0;
}
CommandQueue::~CommandQueue()
{
	//ERROR("COMMAND QUEUE destructor");
	size_t bankMax = NUM_RANKS;
	if (queuingStructure == PerRank) {
		bankMax = 1; 
	}
	for (size_t r=0; r< NUM_RANKS; r++)
	{
		for (size_t b=0; b<bankMax; b++) 
		{
			for (size_t i=0; i<queues[r][b].size(); i++)
			{
				delete(queues[r][b][i]);
			}
			queues[r][b].clear();
		}
	}
}
//Adds a command to appropriate queue
void CommandQueue::enqueue(BusPacket *newBusPacket)
{
	unsigned rank = newBusPacket->rank;
	unsigned bank = newBusPacket->bank;
	if (queuingStructure==PerRank)
	{
		queues[rank][0].push_back(newBusPacket);
		if (queues[rank][0].size()>CMD_QUEUE_DEPTH)
		{
			ERROR("== Error - Enqueued more than allowed in command queue");
			ERROR("						Need to call .hasRoomFor(int numberToEnqueue, unsigned rank, unsigned bank) first");
			exit(0);
		}
	}
	else if (queuingStructure==PerRankPerBank)
	{
		queues[rank][bank].push_back(newBusPacket);
		if (queues[rank][bank].size()>CMD_QUEUE_DEPTH)
		{
			ERROR("== Error - Enqueued more than allowed in command queue");
			ERROR("						Need to call .hasRoomFor(int numberToEnqueue, unsigned rank, unsigned bank) first");
			exit(0);
		}
	}
	else
	{
		ERROR("== Error - Unknown queuing structure");
		exit(0);
	}
}

//Removes the next item from the command queue based on the system's
//command scheduling policy
//
//scyu: get instance of memory controller here to check issuable or not
bool CommandQueue::pop(BusPacket **busPacket, vector<Rank *>* ranks)
{
    //scyu: change the instance of global variable _ranks for checking issuable 
    _ranks = ranks;

	//this can be done here because pop() is called every clock cycle by the parent MemoryController
	//	figures out the sliding window requirement for tFAW
	//
	//deal with tFAW book-keeping
	//	each rank has it's own counter since the restriction is on a device level
	for (size_t i=0;i<NUM_RANKS;i++) 
	{
		//decrement all the counters we have going
		for (size_t j=0;j<tFAWCountdown[i].size();j++)
		{
			tFAWCountdown[i][j]--;
		
		}

		//the head will always be the smallest counter, so check if it has reached 0
		if (tFAWCountdown[i].size()>0 && tFAWCountdown[i][0]==0)

		{
			tFAWCountdown[i].erase(tFAWCountdown[i].begin());
		}
	}

	/* Now we need to find a packet to issue. When the code picks a packet, it will set
		 *busPacket = [some eligible packet]
		 
		 First the code looks if any refreshes need to go
		 Then it looks for data packets
		 Otherwise, it starts looking for rows to close (in open page)
	*/

    //scyu: only modify ClosePage now 
    if (rowBufferPolicy==ClosePage)
	{
		bool sendingREF = false;
		//if the memory controller set the flags signaling that we need to issue a refresh
		if (refreshWaiting)
		{
			bool foundActiveOrTooEarly = false;
			//look for an open bank
			for (size_t b=0;b<NUM_BANKS;b++)
			{
				vector<BusPacket *> &queue = getCommandQueue(refreshRank,b);
				//checks to make sure that all banks are idle
				if (bankStates[refreshRank][b].currentBankState == RowActive)
				{
					foundActiveOrTooEarly = true;
					//if the bank is open, make sure there is nothing else
					// going there before we close it
					for (size_t j=0;j<queue.size();j++)
					{
						BusPacket *packet = queue[j];
						if (packet->row == bankStates[refreshRank][b].openRowAddress &&
								packet->bank == b)

						{
							if (packet->busPacketType != ACTIVATE && isIssuable(packet))

							{
								// scyu: FIXME, might be an issue
								*busPacket = packet;
								queue.erase(queue.begin() + j);
								sendingREF = true;
							}
							break;
						}
					}

					break;

				}
				//	NOTE: checks nextActivate time for each bank to make sure tRP is being
				//				satisfied.	the next ACT and next REF can be issued at the same
				//				point in the future, so just use nextActivate field instead of
				//				creating a nextRefresh field
				else if (bankStates[refreshRank][b].nextActivate > currentClockCycle)

				{
					foundActiveOrTooEarly = true;
					break;
				}

			}

			//if there are no open banks and timing has been met, send out the refresh
			//	reset flags and rank pointer
			if (!foundActiveOrTooEarly && bankStates[refreshRank][0].currentBankState != PowerDown)
			{
				*busPacket = new BusPacket(REFRESH, 0, 0, 0, refreshRank, 0, 0, dramsim_log, currentClockCycle);
				refreshRank = -1;
				refreshWaiting = false;
				sendingREF = true;
			}

		} // refreshWaiting

		//if we're not sending a REF, proceed as normal
		if (!sendingREF)
		{
			//scyu: reordering buffer to schedule write requests
			vector<BusPacket *> reordering_buffer;
			vector<size_t> reordering_buffer_index;

			bool foundIssuable = false;
			unsigned startingRank = nextRank;
			unsigned startingBank = nextBank;
			do
			{
				vector<BusPacket *> &queue = getCommandQueue(nextRank, nextBank);
				//make sure there is something in this queue first
				//	also make sure a rank isn't waiting for a refresh
				//	if a rank is waiting for a refesh, don't issue anything to it until the
				//		refresh logic above has sent one out (ie, letting banks close)
				if (!queue.empty() && !((nextRank == refreshRank) && refreshWaiting))
				{
					bool read_first;

					int read_count=0, write_count=0, activate_count=0;
					for (size_t i=0;i<queue.size();i++){
						read_count += (int) (queue[i]->busPacketType == READ_P);
						write_count += (int) (queue[i]->busPacketType == WRITE_P);
						activate_count += (int) (queue[i]->busPacketType == ACTIVATE);
					}

					if(WriteBurst[nextRank][nextBank] == true){
						if(WriteReqNum[nextRank][nextBank] > 0){ //write queue is still not empty
							read_first = false;
						}else{ // write queue are drained to be empty
							WriteBurst[nextRank][nextBank] = false;  //finish write burst mode
							WriteBurstTotalCycle[nextRank][nextBank] += currentClockCycle - WriteBurstStartCycle[nextRank][nextBank];
							PRINT( " - Bursting Time (Rank " << nextRank << " ,Bank " << nextBank << "): " << (float) WriteBurstTotalCycle[nextRank][nextBank]/currentClockCycle << " ( " << WriteBurstTotalCycle[nextRank][nextBank] << " )");
							read_first = true;
						}
					}else{
						if(WriteReqNum[nextRank][nextBank] >= W_QUEUE_DEPTH){ // write queue full
							WriteBurst[nextRank][nextBank] = true;  //enter write burst mode
							WriteBurstStartCycle[nextRank][nextBank] = currentClockCycle;
							read_first = false;
						}else{
							read_first = true;
						}
					}
					//read_first = (ReadReqNum[nextRank][nextBank] > 0 && WriteReqNum[nextRank][nextBank] <= W_QUEUE_DEPTH * 8 / 10 );

					// needs to find active or not
					bool find_activate = (bankStates[nextRank][nextBank].currentBankState != RowActive); 
					// round 0 for finding issuable requests
					// round 1 for finding un-issuable but could be issuable after dynamic division
					int  round = 0;

					do
					{
						//search from beginning to find first issuable bus packet
						for (size_t i=0;i<queue.size();i++) {
							if(find_activate){
								if(queue[i]->busPacketType == ACTIVATE){
									if(read_first){
										if(queue[i+1]->busPacketType == WRITE_P) continue;
									}else{
										if(queue[i+1]->busPacketType == READ_P) continue;
									}
								}else{
									continue;
								}
							}else{
								if(queue[i]->busPacketType == ACTIVATE){
									continue;
								}
							}

							if (isIssuable(queue[i])){
								//check to make sure we aren't removing a read/write that is paired with an activate
								if (i>0 && queue[i-1]->busPacketType==ACTIVATE &&
										queue[i-1]->physicalAddress == queue[i]->physicalAddress)
									continue;

								if(queue[i]->busPacketType==ACTIVATE){
									//check for dependencies
									bool dependencyFound = false;
									for (size_t j=0;j<i;j++){
										BusPacket *prevPacket = queue[j];
										if (prevPacket->busPacketType == ACTIVATE &&
												prevPacket->bank == queue[i]->bank &&
												prevPacket->row == queue[i]->row){
											dependencyFound = true;
											break;
										}
									}
									if (dependencyFound) continue;
								}

								if(!BUDGET_AWARE_SCHEDULE){
									// for baseline architecture, do not iterrupt write iterations
									if(queue[i]->busPacketType == ACTIVATE && i<(queue.size()-1) 
											&& queue[i+1]->busPacketType == WRITE_P && WriteInProgressed[nextRank][nextBank]){
										// check transaction ID
										// XXX
										//if(queue[i+1]->transID != TransInProgressed[nextRank][nextBank]){
										Rank* rank = _ranks->at(nextRank);
										if(queue[i+1]->transID != TransInProgressed[nextRank][nextBank] 
												|| (POWER_BUDGETING && !rank->budget->issuable(queue[i+1]->token))){
											//cout << queue[i+1]->transID <<  " waiting " << TransInProgressed[nextRank][nextBank] << endl;
											continue;
										}
										// no need to check subReqID, since sub-requests are issued in ordered
									}
								}
									// no sub-request, only dynamic division
#if NO_SUB_REQUEST
								else{
									// check sub-requests is issuable or not
									Rank* rank = _ranks->at(nextRank);
									// for baseline architecture, do not iterrupt write iterations
									if(queue[i]->busPacketType == ACTIVATE && i<(queue.size()-1) 
											&& queue[i+1]->busPacketType == WRITE_P && WriteInProgressed[nextRank][nextBank]){

										// not to push un-issuable packet at round 0
										// not to push un-issuable but issuable by shifting packet at round 1
										if((round == 0 && !rank->budget->issuable(queue[i+1]->token)) 
												|| (round > 0 && !rank->budget->issuableAfterShifting(queue[i+1], queue))){
											continue;
										}
										// check transaction ID
										if((queue[i+1]->transID != TransInProgressed[nextRank][nextBank])){
											//cout << queue[i+1]->transID <<  " waiting " << TransInProgressed[nextRank][nextBank] << endl;
											//cout << rank->budget->dumpBudgetStatus(queue[i+1]->token) << endl;
											continue;
										}
										// no need to check subReqID, since sub-requests are issued in ordered
									}
									else if(queue[i]->busPacketType == WRITE_P){
										// not to push un-issuable packet at round 0
										// not to push un-issuable but issuable by shifting packet at round 1
										if((round == 0 && !rank->budget->issuable(queue[i]->token)) 
												|| (round > 0 && !rank->budget->issuableAfterShifting(queue[i], queue))){
											continue;
										}
										if(WriteInProgressed[nextRank][nextBank] && (queue[i]->transID != TransInProgressed[nextRank][nextBank])){
											continue;
										}
									}
								}
#else
								else{
									// check sub-requests is issuable or not
									Rank* rank = _ranks->at(nextRank);
									if(queue[i]->busPacketType == ACTIVATE && i<(queue.size()-1) && queue[i+1]->busPacketType == WRITE_P){ 
										// not to push un-issuable packet at round 0
										// not to push un-issuable but issuable by shifting packet at round 1
										if((round == 0 && !rank->budget->issuable(queue[i+1]->token)) 
												|| (round > 0 && !rank->budget->issuableAfterShifting(queue[i+1], queue))){
											continue;
										}
									}
									else if(queue[i]->busPacketType == WRITE_P){
										// not to push un-issuable packet at round 0
										// not to push un-issuable but issuable by shifting packet at round 1
										if((round == 0 && !rank->budget->issuable(queue[i]->token)) 
												|| (round > 0 && !rank->budget->issuableAfterShifting(queue[i], queue))){
											continue;
										}
									}

								}
#endif
								// scyu: found issuable, push the packet into reordering buffer
								reordering_buffer.push_back(queue[i]);
								reordering_buffer_index.push_back(i);
								foundIssuable = true;
#if 0
								*busPacket = queue[i];
								queue.erase(queue.begin()+i);
								foundIssuable = true;
								break;
#endif
							}
						}

						// make sure read woundn't interleave an in-progressed write
						if(!read_first && WriteInProgressed[nextRank][nextBank] && !BUDGET_AWARE_SCHEDULE)
							break;
#if NO_SUB_REQUEST
						if(!read_first && WriteInProgressed[nextRank][nextBank] && BUDGET_AWARE_SCHEDULE)
							break;
#endif
						// XXX
						if(!read_first && WriteInProgressed[nextRank][nextBank] && BUDGET_AWARE_SCHEDULE)
							continue;
						read_first = !read_first;
					} while(!foundIssuable && (++round) < 2);
				}

				//if we found something, break out of do-while
				//scyu: keep search issuable writes unless the bank is in bursting mode
				//if (foundIssuable && WriteBurst[nextRank][nextBank]) break;
				if (foundIssuable) break;

				//rank round robin
				if (queuingStructure == PerRank)
				{
					nextRank = (nextRank + 1) % NUM_RANKS;
					if (startingRank == nextRank)
					{
						break;
					}
				}
				else 
				{
					nextRankAndBank(nextRank, nextBank);
					if (startingRank == nextRank && startingBank == nextBank)
					{
						break;
					}
				}
			} while (true);

				// scyu: find the best request to issue
			if(foundIssuable){
				size_t request_to_be_issued = 0;
#if 1
				// #candidates > 1 && write first, traverse the reodering buffer
				if(BUDGET_AWARE_SCHEDULE && reordering_buffer.size() > 1 && reordering_buffer[0]->busPacketType == WRITE_P){
					//cout << "write first, size of reordering buffer " << reordering_buffer.size()<< endl;
					vector<BusPacket *> &queue = getCommandQueue(reordering_buffer[0]->rank, reordering_buffer[0]->bank); 
					Rank* rank = _ranks->at(reordering_buffer[0]->rank);
					float score = 0.0f;
					float max_score = 0.0f;
					for(size_t i=0; i<=reordering_buffer.size()-1; ++i){
						if(reordering_buffer[i]->busPacketType == WRITE_P && rank->budget->issuableAfterShifting(reordering_buffer[i], queue)){
							score = rank->budget->countPriority(reordering_buffer[i]->token);
							if(score > max_score){
								max_score = score;
								request_to_be_issued = i;
							}
						}
					}
				}else if(BUDGET_AWARE_SCHEDULE && reordering_buffer.size() > 1 && reordering_buffer[0]->busPacketType == ACTIVATE){
					vector<BusPacket *> &queue = getCommandQueue(reordering_buffer[0]->rank, reordering_buffer[0]->bank); 
					// only reorder for write first
					if(queue[reordering_buffer_index[0]+1]->busPacketType == WRITE_P){
						//cout << "act first, size of reordering buffer " << reordering_buffer.size()<< endl;
						Rank* rank = _ranks->at(reordering_buffer[0]->rank);
						float max_score = 0.0f;
						float score = 0.0f;
						for(size_t i=0; i<=reordering_buffer.size()-1; ++i){
							if(reordering_buffer[i]->busPacketType == ACTIVATE){
								if(reordering_buffer_index[i]+1 < queue.size() && queue[reordering_buffer_index[i]+1]->busPacketType == WRITE_P 
										//&& rank->budget->issuable(queue[reordering_buffer_index[i]+1]->token)){
										//&& isIssuable(queue[reordering_buffer_index[i]+1]) && rank->budget->issuableAfterShifting(queue[reordering_buffer_index[i]+1], queue)){
									&& rank->budget->issuableAfterShifting(queue[reordering_buffer_index[i]+1], queue)){
										score = rank->budget->countPriority(queue[reordering_buffer_index[i]+1]->token);
										if(score > max_score){
											max_score = score;
											request_to_be_issued = i;
										}
								}
							}
						}
					}
				}
#endif           
				//if(request_to_be_issued != 0)
				//    cout << "reordered" << endl;

				// issue the request
				vector<BusPacket *> &queue = getCommandQueue(reordering_buffer[request_to_be_issued]->rank, reordering_buffer[request_to_be_issued]->bank); 
				*busPacket = queue[reordering_buffer_index[request_to_be_issued]];

				if((*busPacket)->busPacketType == WRITE_P && !BUDGET_AWARE_SCHEDULE){
					Rank* r = _ranks->at((*busPacket)->rank);
					bool need_more_iter = false;
					if(POWER_BUDGETING && !FLEXIBLE_WRITE_CONFIG && !r->budget->issuable((*busPacket)->token)){
						// not issuable 
						busPacket = NULL;
						return false;
					}
					if(POWER_BUDGETING && FLEXIBLE_WRITE_CONFIG && !r->budget->issuableFWC((*busPacket)->token, &need_more_iter)){
						// not issuable 
						busPacket = NULL;
						return false;
					}
					if(FLEXIBLE_WRITE_CONFIG){
						//scyu: issuable for write configuration with more divisions
						bool need_more_iter = false;
						r->budget->doFWC((*busPacket)->token, &need_more_iter);
						(*busPacket)->iterations = (need_more_iter)? 2 : 1;
					}
					if((*busPacket)->subReqID == SUB_REQUEST_COUNT-1){
						// write transaction end
						WriteInProgressed[(*busPacket)->rank][(*busPacket)->bank] = false;
					}else if((*busPacket)->subReqID == 0){
						// write transaction start
						WriteInProgressed[(*busPacket)->rank][(*busPacket)->bank] = true;
						TransInProgressed[(*busPacket)->rank][(*busPacket)->bank] = (*busPacket)->transID;
					}
					//cout << (*busPacket)->transID << " " << (*busPacket)->subReqID << endl;
				}else if((*busPacket)->busPacketType == WRITE_P && BUDGET_AWARE_SCHEDULE){
					Rank* r = _ranks->at((*busPacket)->rank);
					// check could be shifted or not
					if(r->budget->issuableAfterShifting(*busPacket, queue)){
						// shift it 
						if (!r->budget->issuable((*busPacket)->token)) {
							if (r->budget->shiftSubReq(busPacket, queue, false)) {
								(*busPacket)->shifted = true;
							}
						}
						if(!r->budget->shiftSubReq(busPacket, queue, true) && !r->budget->issuable((*busPacket)->token)){
							// failed to shift and is not issuable => no issuable packet
							busPacket = NULL;
							return false;
						}
					}else{
						// if it can not be shifted and is not issuable => no issuable packet!
						busPacket = NULL;
						return false;
					}
					// no sub-request, only dynamic division
#if NO_SUB_REQUEST
					//cout << "rank " << (*busPacket)->rank << " bank " << (*busPacket)->bank << endl;
					//cout << "issue " << (*busPacket)->transID << "-" << (*busPacket)->subReqID << endl; 
					if((*busPacket)->subReqID == SUB_REQUEST_COUNT-1){
						// write transaction end
						WriteInProgressed[(*busPacket)->rank][(*busPacket)->bank] = false;
						//cout << "tran " << TransInProgressed[(*busPacket)->rank][(*busPacket)->bank] << " end" << endl;
					}else if((*busPacket)->subReqID == 0){
						// write transaction start
						WriteInProgressed[(*busPacket)->rank][(*busPacket)->bank] = true;
						TransInProgressed[(*busPacket)->rank][(*busPacket)->bank] = (*busPacket)->transID;
						//cout << "tran " << (*busPacket)->transID << " start" << endl;
					}
#endif
				}
				//if((*busPacket)->busPacketType == WRITE_P){
				//    Rank* r = _ranks->at(reordering_buffer[request_to_be_issued]->rank);
				//    cout << r->budget->dumpRequestStatus(queue[reordering_buffer_index[request_to_be_issued]]->token) << endl;
				//}
#if 0
				// scyu: skip division w/ zero modified bits
				bool zero_modified_bits = true;
				for(size_t i=0; i<=NUM_CHIPS-1; ++i){
					if((*busPacket)->token[i] != 0){
						zero_modified_bits &= false;
					}
				}
				if(!POWER_BUDGETING){
					zero_modified_bits = false;
				}
				(*busPacket)->iterations = (zero_modified_bits)? 0 : (*busPacket)->iterations;
#endif
				queue.erase(queue.begin()+reordering_buffer_index[request_to_be_issued]);
			}

			//if we couldn't find anything to send, return false
			if (!foundIssuable){
				if (POWER_BUDGETING) {
					// laisky
					// TODO: Change to count by PCM bank self
					// A counter in the Bank, increase it by cycle
					// 1. First check if target bank queue is not empty
					// 2. For not empty case, increase the counter in the bank
					unsigned startingRank = 0;
					unsigned startingBank = 0;
					unsigned rankNum = 0;
					unsigned bankNum = 0;
					do
					{
						vector<BusPacket *> &queue = getCommandQueue(rankNum, bankNum);
						Rank* rank = _ranks->at(rankNum);
						// Check if queue is empty
						if (queue.size() > 0) {
							vector<Bank> &banks = rank->banks;
							banks[bankNum].addBlockCycles();
						}
						//rank round robin
						if (queuingStructure == PerRank)
						{
							rankNum = (rankNum + 1) % NUM_RANKS;
							if (startingRank == rankNum)
							{
								break;
							}
						}
						else 
						{
							nextRankAndBank(rankNum, bankNum);
							if (startingRank == rankNum && startingBank == bankNum)
							{
								break;
							}
						}
					} while (true);
				}
				return false;
			}
		}
    }
    else if (rowBufferPolicy==OpenPage)
    {
		bool sendingREForPRE = false;
		if (refreshWaiting)
		{
			bool sendREF = true;
			//make sure all banks idle and timing met for a REF
			for (size_t b=0;b<NUM_BANKS;b++)
			{
				//if a bank is active we can't send a REF yet
				if (bankStates[refreshRank][b].currentBankState == RowActive)
				{
					sendREF = false;
					bool closeRow = true;
					//search for commands going to an open row
					vector <BusPacket *> &refreshQueue = getCommandQueue(refreshRank,b);

					for (size_t j=0;j<refreshQueue.size();j++)
					{
						BusPacket *packet = refreshQueue[j];
						//if a command in the queue is going to the same row . . .
						if (bankStates[refreshRank][b].openRowAddress == packet->row &&
								b == packet->bank)
						{
							// . . . and is not an activate . . .
							if (packet->busPacketType != ACTIVATE)
							{
								closeRow = false;
								// . . . and can be issued . . .
								if (isIssuable(packet))
								{
									//send it out
									*busPacket = packet;
									refreshQueue.erase(refreshQueue.begin()+j);
									sendingREForPRE = true;
								}
								break;
							}
							else //command is an activate
							{
								//if we've encountered another act, no other command will be of interest
								break;
							}
						}
					}

					//if the bank is open and we are allowed to close it, then send a PRE
					if (closeRow && currentClockCycle >= bankStates[refreshRank][b].nextPrecharge)
					{
						rowAccessCounters[refreshRank][b]=0;
						*busPacket = new BusPacket(PRECHARGE, 0, 0, 0, refreshRank, b, 0, dramsim_log, currentClockCycle);
						sendingREForPRE = true;
					}
					break;
				}
				//	NOTE: the next ACT and next REF can be issued at the same
				//				point in the future, so just use nextActivate field instead of
				//				creating a nextRefresh field
				else if (bankStates[refreshRank][b].nextActivate > currentClockCycle) //and this bank doesn't have an open row
				{
					sendREF = false;
					break;
				}
			}

			//if there are no open banks and timing has been met, send out the refresh
			//	reset flags and rank pointer
			if (sendREF && bankStates[refreshRank][0].currentBankState != PowerDown)
			{
				*busPacket = new BusPacket(REFRESH, 0, 0, 0, refreshRank, 0, 0, dramsim_log, currentClockCycle);
				refreshRank = -1;
				refreshWaiting = false;
				sendingREForPRE = true;
			}
		}

		if (!sendingREForPRE)
		{
			unsigned startingRank = nextRank;
			unsigned startingBank = nextBank;
			bool foundIssuable = false;
			do // round robin over queues
			{
				vector<BusPacket *> &queue = getCommandQueue(nextRank,nextBank);
				//make sure there is something there first
				if (!queue.empty() && !((nextRank == refreshRank) && refreshWaiting))
				{

					bool read_first = (ReadReqNum[nextRank][nextBank] > 0 && WriteReqNum[nextRank][nextBank] <= W_QUEUE_DEPTH * 8 / 10 );
					// 2-pass search for prioritizing reads or writes
					for(int pass = 0; pass<2; pass++){
						//search from the beginning to find first issuable bus packet
						for (size_t i=0;i<queue.size();i++)
						{
							BusPacket *packet = queue[i];

							if (read_first){
								if(packet->busPacketType == WRITE)  continue;
								if(packet->busPacketType == ACTIVATE && queue[i+1]->busPacketType == WRITE) continue; 
							}else{
								if(packet->busPacketType == READ)  continue;
								if(packet->busPacketType == ACTIVATE && queue[i+1]->busPacketType == READ) continue; 
							}

							if (isIssuable(packet))
							{
								//check for dependencies
								bool dependencyFound = false;
								for (size_t j=0;j<i;j++)
								{
									BusPacket *prevPacket = queue[j];
									if (prevPacket->busPacketType != ACTIVATE &&
											prevPacket->bank == packet->bank &&
											prevPacket->row == packet->row)
									{
										dependencyFound = true;
										break;
									}
								}
								if (dependencyFound) continue;

								*busPacket = packet;

								//if the bus packet before is an activate, that is the act that was
								//	paired with the column access we are removing, so we have to remove
								//	that activate as well (check i>0 because if i==0 then theres nothing before it)
								if (i>0 && queue[i-1]->busPacketType == ACTIVATE)
								{
									rowAccessCounters[(*busPacket)->rank][(*busPacket)->bank]++;
									// i is being returned, but i-1 is being thrown away, so must delete it here 
									delete (queue[i-1]);

									// remove both i-1 (the activate) and i (we've saved the pointer in *busPacket)
									queue.erase(queue.begin()+i-1,queue.begin()+i+1);
								}
								else // there's no activate before this packet
								{
									//or just remove the one bus packet
									queue.erase(queue.begin()+i);
								}

								foundIssuable = true;
								break;
							}
						}

						if(foundIssuable == true){
							if(pass == 0){
								if((*busPacket)->busPacketType == READ){
									num_read_first  ++;
								}else if((*busPacket)->busPacketType == WRITE){
									num_write_first ++;
								}
							}
							break;
						}else{
							read_first = !read_first;
						}
					}  
				}

				//if we found something, break out of do-while
				if (foundIssuable) break;

				//rank round robin
				if (queuingStructure == PerRank)
				{
					nextRank = (nextRank + 1) % NUM_RANKS;
					if (startingRank == nextRank)
					{
						break;
					}
				}
				else 
				{
					nextRankAndBank(nextRank, nextBank); 
					if (startingRank == nextRank && startingBank == nextBank)
					{
						break;
					}
				}
			}
			while (true);

			//if nothing was issuable, see if we can issue a PRE to an open bank
			//	that has no other commands waiting
			if (!foundIssuable)
			{
				//search for banks to close
				bool sendingPRE = false;
				unsigned startingRank = nextRankPRE;
				unsigned startingBank = nextBankPRE;

				do // round robin over all ranks and banks
				{
					vector <BusPacket *> &queue = getCommandQueue(nextRankPRE, nextBankPRE);
					bool found = false;
					//check if bank is open
					if (bankStates[nextRankPRE][nextBankPRE].currentBankState == RowActive)
					{
						for (size_t i=0;i<queue.size();i++)
						{
							//if there is something going to that bank and row, then we don't want to send a PRE
							if (queue[i]->bank == nextBankPRE &&
									queue[i]->row == bankStates[nextRankPRE][nextBankPRE].openRowAddress)
							{
								found = true;
								break;
							}
						}

						//if nothing found going to that bank and row or too many accesses have happend, close it
						if (!found || rowAccessCounters[nextRankPRE][nextBankPRE]==TOTAL_ROW_ACCESSES)
						{
							if (currentClockCycle >= bankStates[nextRankPRE][nextBankPRE].nextPrecharge)
							{
								sendingPRE = true;
								rowAccessCounters[nextRankPRE][nextBankPRE] = 0;
								*busPacket = new BusPacket(PRECHARGE, 0, 0, 0, nextRankPRE, nextBankPRE, 0, dramsim_log, currentClockCycle);
								break;
							}
						}
					}
					nextRankAndBank(nextRankPRE, nextBankPRE);
				}
				while (!(startingRank == nextRankPRE && startingBank == nextBankPRE));

				//if no PREs could be sent, just return false
				if (!sendingPRE) return false;
			}
		}
    }

    //sendAct is flag used for posted-cas
	//  posted-cas is enabled when AL>0
	//  when sendAct is true, when don't want to increment our indexes
	//  so we send the column access that is paid with this act
    if (AL>0 && sendAct)
	{
		sendAct = false;
	}
	else
	{
		sendAct = true;
		nextRankAndBank(nextRank, nextBank);
	}

	//if its an activate, add a tfaw counter
	if ((*busPacket)->busPacketType==ACTIVATE)
	{
		tFAWCountdown[(*busPacket)->rank].push_back(tFAW);
	}


	return true;
}

//check if a rank/bank queue has room for a certain number of bus packets
bool CommandQueue::hasRoomFor(unsigned numberToEnqueue, unsigned rank, unsigned bank)
{
	vector<BusPacket *> &queue = getCommandQueue(rank, bank); 
	return (CMD_QUEUE_DEPTH - queue.size() >= numberToEnqueue);
}

//prints the contents of the command queue
void CommandQueue::print()
{
	if (queuingStructure==PerRank)
	{
		PRINT(endl << "== Printing Per Rank Queue" );
		for (size_t i=0;i<NUM_RANKS;i++)
		{
			PRINT(" = Rank " << i << "  size : " << queues[i][0].size() );
			for (size_t j=0;j<queues[i][0].size();j++)
			{
				PRINTN("    "<< j << "]");
				queues[i][0][j]->print();
			}
		}
	}
	else if (queuingStructure==PerRankPerBank)
	{
		PRINT("\n== Printing Per Rank, Per Bank Queue" );

		for (size_t i=0;i<NUM_RANKS;i++)
		{
			PRINT(" = Rank " << i );
			for (size_t j=0;j<NUM_BANKS;j++)
			{
				PRINT("    Bank "<< j << "   size : " << queues[i][j].size() );

				for (size_t k=0;k<queues[i][j].size();k++)
				{
					PRINTN("       " << k << "]");
					queues[i][j][k]->print();
				}
			}
		}
	}
}

/** 
 * return a reference to the queue for a given rank, bank. Since we
 * don't always have a per bank queuing structure, sometimes the bank
 * argument is ignored (and the 0th index is returned 
 */
vector<BusPacket *> &CommandQueue::getCommandQueue(unsigned rank, unsigned bank)
{
	if (queuingStructure == PerRankPerBank)
	{
		return queues[rank][bank];
	}
	else if (queuingStructure == PerRank)
	{
		return queues[rank][0];
	}
	else
	{
		ERROR("Unknown queue structure");
		abort(); 
	}

}

//checks if busPacket is allowed to be issued
bool CommandQueue::isIssuable(BusPacket *busPacket)
{
	switch (busPacket->busPacketType)
	{
		case REFRESH:

			break;
		case ACTIVATE:
			if ((bankStates[busPacket->rank][busPacket->bank].currentBankState == Idle ||
						bankStates[busPacket->rank][busPacket->bank].currentBankState == Refreshing) &&
					currentClockCycle >= bankStates[busPacket->rank][busPacket->bank].nextActivate &&
					tFAWCountdown[busPacket->rank].size() < 4)
			{
				if(POWER_BUDGETING){
					//  scyu: add differential write information 
					//  issueable if consumed power < power budget
					Rank* r = _ranks->at(busPacket->rank);
					r->budget->reclaim(currentClockCycle);
				}
				return true;
			}
			else
			{
				return false;
			}
			break;
		case WRITE:
		case WRITE_P:
			if (bankStates[busPacket->rank][busPacket->bank].currentBankState == RowActive &&
					currentClockCycle >= bankStates[busPacket->rank][busPacket->bank].nextWrite &&
					busPacket->row == bankStates[busPacket->rank][busPacket->bank].openRowAddress &&
					rowAccessCounters[busPacket->rank][busPacket->bank] < TOTAL_ROW_ACCESSES)
			{
				//  scyu: toggle the power budget constraint 
				if(POWER_BUDGETING){
					//  scyu: add differential write information 
					//  issueable if consumed power < power budget
					Rank* r = _ranks->at(busPacket->rank);
					r->budget->reclaim(currentClockCycle);
					if(!FLEXIBLE_WRITE_CONFIG){
						// for our mechanism, not check issuable here since there is chance to split it
						return (BUDGET_AWARE_SCHEDULE) || r->budget->issuable(busPacket->token); 
					}else{
						// for flexible write configuration
						bool need_more_iter;
						return r->budget->issuableFWC(busPacket->token, &need_more_iter);
					}
				}else{
					return true;
				}
			}
			else
			{
				return false;
			}
			break;
		case READ_P:
		case READ:
			if (bankStates[busPacket->rank][busPacket->bank].currentBankState == RowActive &&
					currentClockCycle >= bankStates[busPacket->rank][busPacket->bank].nextRead &&
					busPacket->row == bankStates[busPacket->rank][busPacket->bank].openRowAddress &&
					rowAccessCounters[busPacket->rank][busPacket->bank] < TOTAL_ROW_ACCESSES)
			{
				return true;
			}
			else
			{
				return false;
			}
			break;
		case PRECHARGE:
			if (bankStates[busPacket->rank][busPacket->bank].currentBankState == RowActive &&
					currentClockCycle >= bankStates[busPacket->rank][busPacket->bank].nextPrecharge)
			{
				return true;
			}
			else
			{
				return false;
			}
			break;
		default:
			ERROR("== Error - Trying to issue a crazy bus packet type : ");
			busPacket->print();
			exit(0);
	}
	return false;
}

//figures out if a rank's queue is empty
bool CommandQueue::isEmpty(unsigned rank)
{
	if (queuingStructure == PerRank)
	{
		return queues[rank][0].empty();
	}
	else if (queuingStructure == PerRankPerBank)
	{
		for (size_t i=0;i<NUM_BANKS;i++)
		{
			if (!queues[rank][i].empty()) return false;
		}
		return true;
	}
	else
	{
		DEBUG("Invalid Queueing Stucture");
		abort();
	}
}

//tells the command queue that a particular rank is in need of a refresh
void CommandQueue::needRefresh(unsigned rank)
{
	refreshWaiting = true;
	refreshRank = rank;
}

void CommandQueue::nextRankAndBank(unsigned &rank, unsigned &bank)
{
	if (schedulingPolicy == RankThenBankRoundRobin)
	{
		rank++;
		if (rank == NUM_RANKS)
		{
			rank = 0;
			bank++;
			if (bank == NUM_BANKS)
			{
				bank = 0;
			}
		}
	}
	//bank-then-rank round robin
	else if (schedulingPolicy == BankThenRankRoundRobin)
	{
		bank++;
		if (bank == NUM_BANKS)
		{
			bank = 0;
			rank++;
			if (rank == NUM_RANKS)
			{
				rank = 0;
			}
		}
	}
	else
	{
		ERROR("== Error - Unknown scheduling policy");
		exit(0);
	}

}

void CommandQueue::update()
{
	//do nothing since pop() is effectively update(),
	//needed for SimulatorObject
	//TODO: make CommandQueue not a SimulatorObject
}
