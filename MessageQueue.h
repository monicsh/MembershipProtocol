//
//  MessageQueue.hpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 11/25/17.
//  Copyright © 2017 Monika Sharma. All rights reserved.
//

#ifndef MessageQueue_hpp
#define MessageQueue_hpp

#include "stdincludes.h"

#include "q_elt.h"
#include "IMessageQueue.h"


class MessageQueue : public IMessageQueue
{
private:
    std::queue<q_elt> m_messages;
    
public:
    MessageQueue();
    virtual ~MessageQueue();
    bool empty() override;
    void enqueue(void *buffer, int size) override;
    q_elt dequeue() override;
};

#endif /* MessageQueue_hpp */
