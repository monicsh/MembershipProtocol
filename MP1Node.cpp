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
		
		// make an own entry into the list
		int myId = atoi(memberNode->addr.getAddress().substr(0,memberNode->addr.getAddress().find(":")).c_str());
		short myPort = atoi(memberNode->addr.getAddress().substr(memberNode->addr.getAddress().find(":")+1).c_str());
		MemberListEntry myEntry =  MemberListEntry(myId, myPort, memberNode->heartbeat, par->getcurrtime());
		memberNode->memberList.push_back(myEntry);
		log->logNodeAdd(&memberNode->addr, &memberNode->addr);
		auto  pos = memberNode->memberList.begin();
		memberNode->myPos = pos;
		
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
		
		// make an own entry into the list
		int myId = atoi(memberNode->addr.getAddress().substr(0,memberNode->addr.getAddress().find(":")).c_str());
		short myPort = atoi(memberNode->addr.getAddress().substr(memberNode->addr.getAddress().find(":")+1).c_str());
		MemberListEntry myEntry =  MemberListEntry(myId, myPort, memberNode->heartbeat, par->getcurrtime());
		memberNode->memberList.push_back(myEntry);
		log->logNodeAdd(&memberNode->addr, &memberNode->addr);
		auto  pos = memberNode->memberList.begin();
		memberNode->myPos = pos;
		
		// send JOINREQ message to introducer member
		emulNet->ENsend(&memberNode->addr, joinaddr, (char *)msg, (int)msgsize);
		//std::cout << "\nDEBUG: ENSend from: " << memberNode->addr.getAddress() << ", to: " << joinaddr->getAddress() << "; msg type: " << msg->msgType << " ;heartbeat: " << memberNode->heartbeat << std::endl;
		
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


/** FUNCTION NAME : sendMsgBack
 *  Introducer response ro JOINREQ with JOINREP
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
	emulNet->ENsend(&memberNode->addr, toNode, (char *)msg, (int)msgsize);
	free(msg);
}

/** FUNCTION NAME : sendMemberListEntry
 *
 *  example- send new member info as NEWMEMBER(msgtype)t o all member
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
	
	emulNet->ENsend(&memberNode->addr, toNode, (char *)msg, (int)msgsize);
	free(msg);
	
}

/** FUNCTION NAME: updateHeartbeat
 *
 * DESCRIPTION: check heartbeat of a received entry in regular message whether it need to be updated in my list
 *              make a new entry in the list if does not find in the list
 */
void MP1Node::updateHeartbeat(int id, short port, long heartbeat){
	bool found = false;
	for(auto  it = memberNode->memberList.begin(); it != memberNode->memberList.end(); it++){
		if (it->getid() == id and it->getport() == port){
			if (it->getheartbeat() < heartbeat) {
				it->setheartbeat(heartbeat);
				it->settimestamp(par->getcurrtime());
			}
			
			found = true;
			break;
		}
	}
	
	// new entry
	if (!found){
		MemberListEntry newEntry = MemberListEntry(id, port, heartbeat, par->getcurrtime());
		memberNode->memberList.push_back(newEntry);
		
		Address addToLog = Address(to_string(id) + ":" + to_string(port));
		log->logNodeAdd(&memberNode->addr, &addToLog);
	}
	
	return;
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
	
	// JOINREQ : response the request with JOINREP, send new member's info to others, update own memberlist
	if (msg->msgType == JOINREQ){
		
		// 1. acknoledge this new node with JOINREP message
		sendMsgBack(address, JOINREP);
		
		// 2. Add new entry in own memberList
		MemberListEntry newEntry = MemberListEntry(addId, addPort, heartbeat, par->getcurrtime());
		member->memberList.push_back(newEntry);
		
		Address addToLog = Address(to_string(addId) + ":" + to_string(addPort));
		log->logNodeAdd(&memberNode->addr, &addToLog);
		
		// 3. send info about new join member to all other memberss (NEWMEMBER)
		for (auto  it = member->memberList.begin(); it != member->memberList.end(); it++){
			string address = to_string(it->getid()) + ":" + to_string(it->getport());
			Address toNode = Address(address);
			if (memberNode->memberList[0].getid() != it->getid()){
				sendMemberListEntry(&newEntry, &toNode, NEWMEMBER);
			}
		}
		
	} else if (msg->msgType == JOINREP){
		
		memberNode->inGroup = true;
		memberNode->inited = true;
		
		//make own MemberListEntry in memberList
		MemberListEntry coordinatorEntry = MemberListEntry(addId, addPort, heartbeat, par->getcurrtime());
		
		bool isDuplicate = false;
		for (auto  it = member->memberList.begin(); it != member->memberList.end(); it++){
			if ( it->getid() == addId){
				// duplicate message
				isDuplicate = true;
			}
			
		}
		if (!isDuplicate){
			memberNode->memberList.push_back(coordinatorEntry);
			
			Address addToLog = Address(to_string(addId)+":"+to_string(addPort));
			log->logNodeAdd(&memberNode->addr, &addToLog);
		}
		
	} else if (msg->msgType == NEWMEMBER){
		//std::cout<<"I am: " <<memberNode->addr.getAddress()<<"new member is: "<< address->getAddress()<<endl;
		if (memberNode->memberList[0].getid() != addId and memberNode->memberList[0].getid() != 1){
			MemberListEntry newEntry = MemberListEntry(addId, addPort, heartbeat, par->getcurrtime());
			
			bool isDuplicate = false;
			for (auto  it = member->memberList.begin(); it != member->memberList.end(); it++){
				if ( it->getid() == addId){
					// duplicate message
					isDuplicate = true;
				}
				
			}
			
			if (!isDuplicate){
				member->memberList.push_back(newEntry);
				
				Address addToLog = Address(to_string(addId)+":"+to_string(addPort));
				log->logNodeAdd(&memberNode->addr, &addToLog);
			}
		}
		
	} else if (msg->msgType == REGULAR){
		//update memberList for the member if the incoming heartbeat > heartbeat
		//std::cout<<"REGULAR message: from: "<<addId<<endl;
		if (!memberNode->memberList.empty() && memberNode->memberList[0].getid() != addId){
			updateHeartbeat(addId, addPort,heartbeat);
		}
		
	} else if (msg->msgType == LEAVEGROUP){
		// whichone?
		//addId and addPort
	}
	
	/*
	 else if (msg->msgType == LEAVEGROUP){
	 //delete the member from the list
	 if (deleteLeaveMemberEntry(addId, addPort)) {
	 
	 //Address nodeToRemove(to_string(addId) + ":" + to_string(addPort));
	 //log->logNodeRemove(&memberNode->addr, &nodeToRemove);
	 
	 //propogate the message to all members
	 for (auto it = memberNode->memberList.begin() + 1; it != memberNode->memberList.end(); it++) {
	 Address toNode(to_string(it->getid()) + ":" + to_string(it->getport()));
	 sendLeaveMessage(&toNode , addId, addPort, LEAVEGROUP);
	 }
	 
	 }
	 
	 }
	 */
	
	return true;
}


/**
 * FUNCTION NAME: sendLeaveMessage
 */
void MP1Node::sendLeaveMessage(Address* toNode, int addId, short addPort, MsgTypes msgType) {
	size_t msgsize = sizeof(MessageHdr) + sizeof(toNode->addr) + sizeof(long) + 1;
	MessageHdr* msg = (MessageHdr *) malloc(msgsize * sizeof(char));
	
	Address leaveMemberAddress(to_string(addId) + ":" + to_string(addPort));
	
	// create LEAVEGROUP message: format of data is {struct Address myaddr}
	msg->msgType = msgType;
	memcpy((char *)(msg+1), leaveMemberAddress.addr, sizeof(Address));
	memcpy((char *)(msg+1) + 1 + sizeof(Address), &memberNode->heartbeat, sizeof(long));
	
	// send LEAVEGROUP message to new member
	emulNet->ENsend(&memberNode->addr, toNode, (char *)msg, (int)msgsize);
	free(msg);
}


/**
 * FUNCTION NAME: deleteLeaveMemberEntry
 *
 * DESCRIPTION: delete the leaving member from the member list
 **//*
	 bool MP1Node::deleteLeaveMemberEntry(int addId, short addPort)
	 {
	 //bool mismatchedItr = (!memberNode->memberList.empty() && memberNode->memberList[0].getid() != memberNode->myPos->getid());
	 // bool mismatchedIndex = (!memberNode->memberList.empty() && memberNode->memberList[0].getid() != memberNode->memberList.begin()->getid());
	 
	 for(auto it = memberNode->memberList.begin() + 1; it != memberNode->memberList.end(); it++) {
	 
	 if (it->getid() == addId and it->getport() == addPort) {
	 
	 //Address nodeToRemove(AddressToString(*it));
	 //log->logNodeRemove(&memberNode->addr, &nodeToRemove);
	 
	 it = memberNode->memberList.erase(it);
	 it--;
	 return true;
	 }
	 }
	 
	 
	 return false;
	 }
	 */

/**
 * FUNCTION NAME: createRegularMessage
 *
 */
MessageHdr* MP1Node::createRegularMessage(MemberListEntry* entry, MsgTypes msgType, size_t msgsize){
	//size_t msgsize = sizeof(MessageHdr) + (sizeof(Address)+1) + sizeof(long) +  1;
	MessageHdr* msg = (MessageHdr *) malloc(msgsize * sizeof(char));
	
	// create REGULAR message
	string mleAddressString = to_string(entry->getid()) + ":" + to_string(entry->getport());
	Address mleAddress = Address(mleAddressString);
	long mleHeartbeat = entry->getheartbeat();
	
	msg->msgType = msgType;
	memcpy((char *)(msg+1), (char *)(mleAddress.addr), sizeof(mleAddress.addr));
	memcpy((char *)(msg+1)+1 + sizeof(mleAddress.addr), &mleHeartbeat ,sizeof(long));
	
	return msg;
}

/**
 * FUNCTION NAME: propogateRegularMessage
 *
 * DESCRIPTION : propogate REGULAR message to all members in the list
 */
void MP1Node::propogateRegularMessage(){
	MemberListEntry* entry;
	size_t msgsize = sizeof(MessageHdr) + (sizeof(Address)+1) + sizeof(long) +  1;
	
	for(auto  it = memberNode->memberList.begin(); it != memberNode->memberList.end(); it++){
		
		if (it->getid() == memberNode->memberList[0].getid()){
			//cout<<"this is my entry and here my info : "<<it->getid()<<"HB : "<<it->getheartbeat()<<endl;
			memberNode->memberList[0].setheartbeat(memberNode->memberList[0].getheartbeat()+1);
		}
		entry = &*it;
		MessageHdr* msg = createRegularMessage(entry, REGULAR, msgsize);
		
		//disseminateRegularMessage(MessageHdr* msg, msgsize );
		for(auto  toNodeIt = memberNode->memberList.begin() + 1; toNodeIt != memberNode->memberList.end(); toNodeIt++){
			//if (toNodeIt != memberNode->memberList[0]){
			string toNodeAddressString = to_string(toNodeIt->getid()) + ":" + to_string(toNodeIt->getport());
			Address toNodeAddress = Address(toNodeAddressString);
			
			emulNet->ENsend(&memberNode->addr, &toNodeAddress, (char *)msg, (int)msgsize);
			//}
		}
	}
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
	
	checkFailure();
	
	//1. check all membership enties in MEmberList whether && 2. if the entry got timeout-- delete it
	/*
	 
	 for(auto  it = memberNode->memberList.begin(); it != memberNode->memberList.end(); it++){
	 if (it->getid() == memberNode->myPos->getid()){
	 //cout<<"ooOO it's me"<<it->getid()<<endl;
	 //memberNode->myPos->setheartbeat(memberNode->myPos->getheartbeat()+1);
	 //memberNode->myPos->settimestamp(par->getcurrtime());
	 
	 } else if ((par->getcurrtime() - it->gettimestamp()) > 4*(memberNode->pingCounter)){
	 //cout<<"time: "<<par->getcurrtime()<<"ts: "<<it->gettimestamp()<<"id: "<<it->getid()<<"port: "<<it->getport()<<endl;
	 //cout<<"member being erased : "<<it->getid()<< "heartbeat : "<<it->getheartbeat()<<endl;
	 memberNode->memberList.erase(it);
	 }
	 }
	 */
	;
	//3. Propogate MemberList with REGULAR msg
	propogateRegularMessage();
	
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


/**
 *
 */
void MP1Node::printMemberList()
{
	std::cout << "=== [id:" << memberNode->myPos->getid() << "] ";
	
	printAddress(&memberNode->addr);
	
	for (auto& mbr : memberNode->memberList)
	{
		std::cout << mbr.getid() << " "
		<< mbr.getport() << " "
		<< mbr.getheartbeat() << " "
		<< mbr.gettimestamp() << endl;
	}
	
	std::cout << endl;
}

/* FUNCTION NAME: checkFailure
 *
 * DESCRIPTION: check all membership enties in MEmberList whether && if the entry got timeout-delete it
 */
void MP1Node::checkFailure()
{
	
	// bool mismatchedItr = (!memberNode->memberList.empty() && memberNode->memberList[0].getid() != memberNode->myPos->getid());
	// bool mismatchedIndex = (!memberNode->memberList.empty() && memberNode->memberList[0].getid() != memberNode->memberList.begin()->getid());
	
	/*if (mismatchedItr) {
	 printMemberList();
	 }*/
	
	for(auto it = memberNode->memberList.begin() + 1; it != memberNode->memberList.end(); it++) {
		// bool mismatchedItr = (it->getid() != memberNode->myPos->getid());
		//115 - 50
		if (((par->getcurrtime() - it->gettimestamp()) >= (3 * memberNode->pingCounter))) {
			
			Address nodeToRemove(AddressToString(*it));
			log->logNodeRemove(&memberNode->addr, &nodeToRemove);
			
			it = memberNode->memberList.erase(it);
			it--;
		}
	}
	
}
