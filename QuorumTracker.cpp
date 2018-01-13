
//
//  QuorumTracker.cpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 1/13/18.
//  Copyright © 2018 Monika Sharma. All rights reserved.
//

#include "QuorumTracker.h"

QuorumTracker::QuorumTracker(
              Params *par,
              Log *log)
{
    this->m_parameters = par;
    this->m_logger = log;
}

QuorumTracker::~QuorumTracker()
{
}

