/**********************************
 * FILE NAME: MP1Node.cpp
 *
 * DESCRIPTION: Membership protocol run by this Node.
 * 				Definition of MP1Node class functions.
 **********************************/

#include "MP1Node.h"

/*
 * Note: You can change/add any functions in MP1Node.{h,cpp}
 */

/**
 * Overloaded Constructor of the MP1Node class
 * You can add new members to the class if you think it
 * is necessary for your logic to work
 */
MP1Node::MP1Node(Member *member, Params *params, EmulNet *emul, Log *log, Address *address) {
	for( int i = 0; i < 6; i++ ) {
		NULLADDR[i] = 0;
	}
	this->memberNode = member;
	this->emulNet = emul;
	this->log = log;
	this->par = params;
	this->memberNode->addr = *address;
}

/**
 * Destructor of the MP1Node class
 */
MP1Node::~MP1Node() {}

/**
 * FUNCTION NAME: recvLoop
 *
 * DESCRIPTION: This function receives message from the network and pushes into the queue
 * 				This function is called by a node to receive messages currently waiting for it
 */
int MP1Node::recvLoop() {
    if ( memberNode->bFailed ) {
    	return false;
    }
    else {
    	return emulNet->ENrecv(&(memberNode->addr), enqueueWrapper, NULL, 1, &(memberNode->mp1q));
    }
}

/**
 * FUNCTION NAME: enqueueWrapper
 *
 * DESCRIPTION: Enqueue the message from Emulnet into the queue
 */
int MP1Node::enqueueWrapper(void *env, char *buff, int size) {
	Queue q;
	return q.enqueue((queue<q_elt> *)env, (void *)buff, size);
}

/**
 * FUNCTION NAME: nodeStart
 *
 * DESCRIPTION: This function bootstraps the node
 * 				All initializations routines for a member.
 * 				Called by the application layer.
 */
void MP1Node::nodeStart(char *servaddrstr, short servport) {
    Address joinaddr;
    joinaddr = getJoinAddress();

    // Self booting routines
    if( initThisNode(&joinaddr) == -1 ) {
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "init_thisnode failed. Exit.");
#endif
        exit(1);
    }

    if( !introduceSelfToGroup(&joinaddr) ) {
        finishUpThisNode();
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Unable to join self to group. Exiting.");
#endif
        exit(1);
    }

    return;
}

/**
 * FUNCTION NAME: initThisNode
 *
 * DESCRIPTION: Find out who I am and start up
 */
int MP1Node::initThisNode(Address *joinaddr) {
	/*
	 * This function is partially implemented and may require changes
	 */
	int id = *(int*)(&memberNode->addr.addr);
	int port = *(short*)(&memberNode->addr.addr[4]);

	memberNode->bFailed = false;
	memberNode->inited = true;
	memberNode->inGroup = false;
    // node is up!
	memberNode->nnb = 0;
	memberNode->heartbeat = 0;
	memberNode->pingCounter = TFAIL;
	memberNode->timeOutCounter = -1;
    initMemberListTable(memberNode);

    return 0;
}

/**
 * FUNCTION NAME: introduceSelfToGroup
 *
 * DESCRIPTION: Join the distributed system
 */
int MP1Node::introduceSelfToGroup(Address *joinaddr) {
	MessageHdr *msg;
#ifdef DEBUGLOG
    static char s[1024];
#endif

    if ( 0 == memcmp((char *)&(memberNode->addr.addr), (char *)&(joinaddr->addr), sizeof(memberNode->addr.addr))) {
        // I am the group booter (first process to join the group). Boot up the group
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Starting up group...");
#endif
        memberNode->inGroup = true;
    }
    else {
        size_t msgsize = sizeof(MessageHdr) + sizeof(joinaddr->addr) + sizeof(long) + 1;
        msg = (MessageHdr *) malloc(msgsize * sizeof(char));
        // create JOINREQ message: format of data is {struct Address myaddr}
        msg->msgType = JOINREQ;
        memcpy((char *)(msg+1), &memberNode->addr.addr, sizeof(memberNode->addr.addr));
        memcpy((char *)(msg+1) + 1 + sizeof(memberNode->addr.addr), &memberNode->heartbeat, sizeof(long));

#ifdef DEBUGLOG
        sprintf(s, "Trying to join...");
        log->LOG(&memberNode->addr, s);
#endif

        // send JOINREQ message to introducer member
        emulNet->ENsend(&memberNode->addr, joinaddr, (char *)msg, msgsize);
        std::cout << "\nDEBUG: ENSend from: " << memberNode->addr.getAddress() << ", to: " << joinaddr->getAddress() << "; msg type: " << msg->msgType << " ;heartbeat: " << memberNode->heartbeat << std::endl;

        free(msg);
    }

    return 1;

}

/**
 * FUNCTION NAME: finishUpThisNode
 *
 * DESCRIPTION: Wind up this node and clean up state
 */
int MP1Node::finishUpThisNode() {
   /*
    * Your code goes here
    */
    return 0;
}

/**
 * FUNCTION NAME: nodeLoop
 *
 * DESCRIPTION: Executed periodically at each member
 * 				Check your messages in queue and perform membership protocol duties
 */
void MP1Node::nodeLoop() {
    if (memberNode->bFailed) {
    	return;
    }

    // Check my messages
    checkMessages();

    // Wait until you're in the group...
    if( !memberNode->inGroup ) {
    	return;
    }

    // ...then jump in and share your responsibilites!
    nodeLoopOps();

    return;
}

/**
 * FUNCTION NAME: checkMessages
 *
 * DESCRIPTION: Check messages in the queue and call the respective message handler
 */
void MP1Node::checkMessages() {
    void *ptr;
    int size;

    // Pop waiting messages from memberNode's mp1q
    while ( !memberNode->mp1q.empty() ) {
    	ptr = memberNode->mp1q.front().elt;
    	size = memberNode->mp1q.front().size;
    	memberNode->mp1q.pop();
    	recvCallBack((void *)memberNode, (char *)ptr, size);
    }
    return;
}


/** FUNCTION NAME :
 *
 *
 */
void MP1Node::sendMsgBack(Address* toNode, MsgTypes msgType) {
    size_t msgsize = sizeof(MessageHdr) + sizeof(toNode->addr) + sizeof(long) + 1;
    MessageHdr* msg = (MessageHdr *) malloc(msgsize * sizeof(char));

    // create JOINREP message: format of data is {struct Address myaddr}
    msg->msgType = msgType;
    memcpy((char *)(msg+1), &memberNode->addr.addr, sizeof(memberNode->addr.addr));
    memcpy((char *)(msg+1) + 1 + sizeof(memberNode->addr.addr), &memberNode->heartbeat, sizeof(long));

    // send JOINREP message to new member
    emulNet->ENsend(&memberNode->addr, toNode, (char *)msg, msgsize);
    free(msg);
}

/** FUNCTION NAME : sendMemberListEntry
 *
 *
 */
void MP1Node::sendMemberListEntry(MemberListEntry* entry, Address* toNode, MsgTypes msgType) {
    size_t msgsize = sizeof(MessageHdr) + (sizeof(Address)+1) + sizeof(long) +  1;
    MessageHdr* msg = (MessageHdr *) malloc(msgsize * sizeof(char));

    // create NEWMEMBER message
    string newMemberAddressString = to_string(entry->getid()) + ":" + to_string(entry->getport());
    Address newMemberAddress = Address(newMemberAddressString);
    long newMemberHeartbeat = entry->getheartbeat();

    msg->msgType = msgType;
    memcpy((char *)(msg+1), (char *)(newMemberAddress.addr), sizeof(newMemberAddress.addr));
    memcpy((char *)(msg+1)+1 + sizeof(newMemberAddress.addr), &newMemberHeartbeat ,sizeof(long));

    emulNet->ENsend(&memberNode->addr, toNode, (char *)msg, msgsize);
    free(msg);



    //create memeberlistentry package
    //memcpy((char *)(msg+1), (char *)(&mle), sizeof(mle));



    //memcpy((char *)(msg+1), &memberNode->addr.addr, sizeof(memberNode->addr.addr));

    //memcpy((char *)(msg+1), (char *)(entry->getid()),sizeof(int));
    //memcpy((char *)(msg+1) + sizeof(entry->getid()), (char *)(entry->getport()),sizeof(short));
    //memcpy((char *)(msg+1) + sizeof(entry->getid()) + sizeof(entry->getport()), (char *)(entry->getheartbeat()),sizeof(long));

    //memcpy((char *)(msg) + 1 + sizeof(&memberNode->addr.addr), entry->getheartbeat(), sizeof(long));

    // send NEWMEMBER message
    //emulNet->ENsend(&memberNode->addr, toNode, (char *)msg, msgsize);

    //free(msg);

}


/**
 * FUNCTION NAME: recvCallBack
 *
 * DESCRIPTION: Message handler for different message types
 */
bool MP1Node::recvCallBack(void *env, char *data, int size ) {

    Member* member = (Member*)env;
    MessageHdr* msg = (MessageHdr*)data;
    data += sizeof(MessageHdr);
    Address* address = (Address*)data;
    data += 7;
    long heartbeat = (long)(*data);

    // Extract id and port from Address
    int addId = atoi(address->getAddress().substr(0,address->getAddress().find(":")).c_str());
    short addPort = atoi(address->getAddress().substr(address->getAddress().find(":")+1).c_str());

    std::cout << "msgType: " << msg->msgType
    << " ; address: " << address->getAddress()
    << " ; heartbeat: " << heartbeat
    << "\n";

    
    // add in your member list
    if (msg->msgType == JOINREQ)
    {
        MemberListEntry newEntry = MemberListEntry(addId, addPort, heartbeat, par->getcurrtime());

        // 1. acknoledge this new node with JOINREP message
        sendMsgBack(address, JOINREP);

        // 2. send info about new join member to all other memberss (NEWMEMBER)
        for (std::vector<MemberListEntry>::iterator it = member->memberList.begin(); it != member->memberList.end(); ++it){
            //std::cout<< "Member::  id: "<< it->getid()<<"port: "<<it->getport() <<"heartbeat: "<<it->getheartbeat() << endl;
            string address = to_string(it->getid()) + ":" + to_string(it->getport());
            Address toNode = Address(address);
            std::cout<<"toNode : "<<toNode.getAddress();
            sendMemberListEntry(&newEntry, &toNode, NEWMEMBER);
        }
        // 3. add this node to yout memmeberlist
        member->memberList.push_back(newEntry);

    }
    else if (msg->msgType == JOINREP)
    {
        // turn on member-> inGroup
        memberNode->inGroup = true;
        memberNode->inited = true;


    }
    else if (msg->msgType == NEWMEMBER)
    {
        std::cout<<"new member is: "<< address->getAddress()
        <<"I am: " <<memberNode->addr.getAddress()<<endl;


    }

    if (this->memberNode->addr.getAddress() == address->getAddress())
    {
        // update your own location in memberlist
    }

    
    return true;
}

/**
 * FUNCTION NAME: nodeLoopOps
 *
 * DESCRIPTION: Check if any node hasn't responded within a timeout period and then delete
 * 				the nodes
 * 				Propagate your membership list
 */
void MP1Node::nodeLoopOps() {

	/*
	 * Your code goes here
	 */

    return;
}

/**
 * FUNCTION NAME: isNullAddress
 *
 * DESCRIPTION: Function checks if the address is NULL
 */
int MP1Node::isNullAddress(Address *addr) {
	return (memcmp(addr->addr, NULLADDR, 6) == 0 ? 1 : 0);
}

/**
 * FUNCTION NAME: getJoinAddress
 *
 * DESCRIPTION: Returns the Address of the coordinator
 */
Address MP1Node::getJoinAddress() {
    Address joinaddr;

    memset(&joinaddr, 0, sizeof(Address));
    *(int *)(&joinaddr.addr) = 1;
    *(short *)(&joinaddr.addr[4]) = 0;

    return joinaddr;
}

/**
 * FUNCTION NAME: initMemberListTable
 *
 * DESCRIPTION: Initialize the membership list
 */
void MP1Node::initMemberListTable(Member *memberNode) {
	memberNode->memberList.clear();
}

/**
 * FUNCTION NAME: printAddress
 *
 * DESCRIPTION: Print the Address
 */
void MP1Node::printAddress(Address *addr)
{
    printf("%d.%d.%d.%d:%d \n",  addr->addr[0],addr->addr[1],addr->addr[2],
                                                       addr->addr[3], *(short*)&addr->addr[4]) ;    
}
