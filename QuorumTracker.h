//
//  QuorumTracker.hpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 1/13/18.
//  Copyright © 2018 Monika Sharma. All rights reserved.
//

#ifndef QuorumTracker_hpp
#define QuorumTracker_hpp

#include "stdincludes.h"
#include "EmulNet.h"
#include "Node.h"
#include "Log.h"
#include "Params.h"
#include "Message.h"

struct QuoromDetail
{
    QuoromDetail()
    : replyCounter(0)
    , reqTime(0)
    {
    }

    MessageType transMsgType;
    int reqTime;
    string reqKey;
    unsigned replyCounter;
};

class QuorumTracker
{
private:
    Params *m_parameters;
    Log * m_logger;

    // TODO: this is only a logger dependency, remove once logger is independent of addres
    Address m_address;

    // container for tracking quorom for messages
    std::map<int, struct QuoromDetail> m_quorum;

    bool isTimedout(QuoromDetail& quoromDetail);

public:
    virtual ~QuorumTracker();
    QuorumTracker(
        Address addr,
        Params *par,
        Log *log);

    void updateQuorum(MessageType msgType, string key);
    void isQuorumTimedout();
    bool isQuorumEntryExists(int transID);
    QuoromDetail GetQuorumDetails(int transID);
    void removeQuorumEntry(int transID);
    void saveQuorum(int transID, QuoromDetail record);
};

#endif /* QuorumTracker_hpp */
