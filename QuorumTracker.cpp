//
//  QuorumTracker.cpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 1/13/18.
//  Copyright © 2018 Monika Sharma. All rights reserved.
//

#include "QuorumTracker.h"

QuorumTracker::QuorumTracker(
    Address addr,
    Params *par,
    Log *log)
{
    m_address = addr;
    this->m_parameters = par;
    this->m_logger = log;
}

QuorumTracker::~QuorumTracker()
{
}

void QuorumTracker::updateQuorum(MessageType msgType, string key)
{
    // add quorom counter = 0 for each sent READ message triplet sent above
    this->m_quorum[g_transID].transMsgType = msgType;
    this->m_quorum[g_transID].reqTime = this->m_parameters->getcurrtime();
    this->m_quorum[g_transID].reqKey = key;

    g_transID++;
}

bool QuorumTracker::isTimedout(QuoromDetail& quoromDetail)
{
    return (quoromDetail.replyCounter < 2
            && quoromDetail.reqTime <= (this->m_parameters->getcurrtime() - 5));
}

void QuorumTracker::isQuorumTimedout()
{
    auto record = this->m_quorum.begin();
    while (record != this->m_quorum.end()) {

        if (isTimedout(record->second)) {
            this->m_logger->logReadFail(&(this->m_address), true, record->first, record->second.reqKey);

            // remove failing qurom record
            record = this->m_quorum.erase(record);
            continue;
        }

        // check next
        ++record;
    }
}

bool QuorumTracker::quorumExists(int transID)
{
    return this->m_quorum.end() != this->m_quorum.find(transID);
}

QuoromDetail QuorumTracker::getQuorum(int transID)
{
    auto record = this->m_quorum.find(transID);
    if (record == this->m_quorum.end()) {
        throw new runtime_error("quorum entry not found, call quorumExists before this function");
    }

    return record->second;
}

void QuorumTracker::removeQuorum(int transID)
{
    auto record = this->m_quorum.find(transID);
    if (record != this->m_quorum.end()) {
        this->m_quorum.erase(record);
    }
}

void QuorumTracker::saveQuorum(int transID, QuoromDetail record)
{
    this->m_quorum[transID] = record;
}
