/**********************************
 * FILE NAME: MP1Node.cpp
 *
 * DESCRIPTION: Membership protocol run by this Node.
 * 				Header file of MP1Node class.
 **********************************/

#ifndef _MP1NODE_H_
#define _MP1NODE_H_

#include "stdincludes.h"
#include "Log.h"
#include "Params.h"
#include "Member.h"
#include "EmulNet.h"
#include "Queue.h"


/**
 * Macros
 */
#define TREMOVE 20
#define TFAIL 5

/*
 * Note: You can change/add any functions in MP1Node.{h,cpp}
 */

/**
 * Message Types
 */
enum MsgTypes{
    JOINREQ,
    JOINREP,
    NEWMEMBER,
    LEAVEGROUP,
    REGULAR,
    DUMMYLASTMSGTYPE
};

/**
 * STRUCT NAME: MessageHdr
 *
 * DESCRIPTION: Header and content of a message
 */
typedef struct MessageHdr {
	enum MsgTypes msgType;
}MessageHdr;

/**
 * CLASS NAME: MP1Node
 *
 * DESCRIPTION: Class implementing Membership protocol functionalities for failure detection
 */
class MP1Node {
private:
	EmulNet *emulNet;
	Log *log;
	Params *par;
	Member *memberNode;
	char NULLADDR[6];

public:
	MP1Node(Member *, Params *, EmulNet *, Log *, Address *);
	Member * getMemberNode() {
		return memberNode;
	}
	int recvLoop();
	static int enqueueWrapper(void *env, char *buff, int size);
	void nodeStart(char *servaddrstr, short serverport);
	int initThisNode(Address *joinaddr);
	int introduceSelfToGroup(Address *joinAddress);
	int finishUpThisNode();
	void nodeLoop();
	void checkMessages();
    void sendMemberListEntry(MemberListEntry* entry, Address* toNode, MsgTypes msgType);
    void sendMsgBack(Address* toNode, MsgTypes msgType );
    void updateHeartbeat(int id, short port, long heartbeat);
	bool recvCallBack(void *env, char *data, int size);
    bool deleteLeaveMemberEntry(int addId, short addPort);
    MessageHdr* createRegularMessage(MemberListEntry* entry, MsgTypes msgType, size_t msgsize);
    void propogateRegularMessage();
	void nodeLoopOps();
	int isNullAddress(Address *addr);
	Address getJoinAddress();
	void initMemberListTable(Member *memberNode);
	void printAddress(Address *addr);
    void printMemberList();
    void checkFailure();
	virtual ~MP1Node();
    void sendLeaveMessage(Address* toNode, int addId, short addPort, MsgTypes msgType);
    
    static std::string AddressToString(MemberListEntry& member)
    {
        return to_string(member.getid()) + ":" + to_string(member.getport());
    }

};

#endif /* _MP1NODE_H_ */
