//
//  MessageQueue.cpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 11/25/17.
//  Copyright © 2017 Monika Sharma. All rights reserved.
//

#include "MessageQueue.h"

MessageQueue::MessageQueue()
{
}

MessageQueue::~MessageQueue()
{
}

bool MessageQueue::empty()
{
    return m_messages.empty();
}

void MessageQueue::enqueue(void *buffer, int size)
{
    q_elt item(buffer, size);
    m_messages.emplace(item);
}

q_elt MessageQueue::dequeue()
{
    q_elt item = m_messages.front();
    m_messages.pop();
    return item;
}
