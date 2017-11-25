/**********************************
 * FILE NAME: Queue.h
 *
 * DESCRIPTION: Header file for std::<queue> related functions
 **********************************/

#ifndef QUEUE_H_
#define QUEUE_H_

#include "stdincludes.h"

/**
* CLASS NAME: q_elt
*
* DESCRIPTION: Entry in the queue
*/
class q_elt {
public:
	void *elt;
	int size;
	q_elt(void *elt, int size);
};

//Interface Q
class QueueInterface {
public:
	virtual bool enqueue()=0;
	virtual q_elt dequeue()=0;

};

/**
 * Class name: Queue
 *
 * Description: This function wraps std::queue related functions
 */
class Queue : QueueInterface {
public:
	Queue();
	virtual ~Queue();
	static bool enqueue(queue<q_elt> *queue, void *buffer, int size);
	bool enqueue() override {

	}

	q_elt dequeue() override {
	}

};



#endif /* QUEUE_H_ */
