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

class QuorumTracker
{
private:
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

    Params *m_parameters;
    Log * m_logger;

public:
    virtual ~QuorumTracker();
    QuorumTracker(
        Params *par,
        Log *log);

};
#endif /* QuorumTracker_hpp */
