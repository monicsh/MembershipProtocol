//
//  Queue.cpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 11/25/17.
//  Copyright © 2017 Monika Sharma. All rights reserved.
//


#include <stdio.h>
#include "Queue.h"

q_elt::q_elt(void *elt, int size)
	: elt(elt), size(size)
{
}

Queue::Queue() {

}

Queue::~Queue() {

}

bool Queue::enqueue(queue<q_elt> *queue, void *buffer, int size) {
		q_elt element(buffer, size);
		queue->emplace(element);
		return true;

}

