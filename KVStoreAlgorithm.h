/**********************************
 * FILE NAME: MP2Node.h
 *
 * DESCRIPTION: MP2Node class header file
 **********************************/

#ifndef MP2NODE_H_
#define MP2NODE_H_

/**
 * Header files
 */
#include "stdincludes.h"
#include "EmulNet.h"
#include "Node.h"
#include "HashTable.h"
#include "Log.h"
#include "Params.h"
#include "Message.h"

/**
 * CLASS NAME: MP2Node
 *
 * DESCRIPTION: This class encapsulates all the key-value store functionality
 * 				including:
 * 				1) Ring
 * 				2) Stabilization Protocol
 * 				3) Server side CRUD APIs
 * 				4) Client side CRUD APIs
 */
class KVStoreAlgorithm {
private:
	// Vector holding the next two neighbors in the ring who have my replicas
	vector<Node> hasMyReplicas;
	// Vector holding the previous two neighbors in the ring whose replicas I have
	vector<Node> haveReplicasOf;
	// Ring
	vector<Node> ring;
	// Hash Table
	HashTable * ht;
	// Member representing this member
	Member *memberNode;
	// Params object
	Params *par;
	// Object of EmulNet
	EmulNet * emulNet;
	// Object of Log
	Log * log;
	
	vector<bool> neighbourStateCheckFlag;
	
	struct QuoromDetail
	{
		QuoromDetail() : replyCounter(0)
		{ }
		
		MessageType transMsgType;
		int reqTime;
		string reqKey;
		unsigned replyCounter;
	};
	
	// container for tracking quorom for READ messages
	std::map<int, struct QuoromDetail> quorumRead;
	
	// container for tracking quorom for DELETE messages
	std::map<int, struct QuoromDetail> quorum;
	

	struct ActionOnReplicaNode
	{
		
		Node node;
		MessageType msgType;
		ReplicaType replicaType;
	};
	
	struct ViolatedNodeSet
	{
		ViolatedNodeSet() {};
		vector<ActionOnReplicaNode> actionOnReplicaNode;
	};


public:
	KVStoreAlgorithm(Member *memberNode, Params *par, EmulNet *emulNet, Log *log, Address *addressOfMember);
	Member * getMemberNode() {
		return this->memberNode;
	}

	// ring functionalities
	void updateRing();
	vector<Node> getMembershipList();
	size_t hashFunction(string key);


	// client side CRUD APIs
	void clientCreate(string key, string value);
	void clientRead(string key);
	void clientUpdate(string key, string value);
	void clientDelete(string key);

	// receive messages from Emulnet
	bool recvLoop();
	static int enqueueWrapper(void *env, char *buff, int size);

	// handle messages from receiving queue
	void checkMessages();

	// coordinator dispatches messages to corresponding nodes
	void dispatchMessages(Message message);

	// find the addresses of nodes that are responsible for a key
	vector<Node> findNodes(string key);

	// server
	bool createKeyValue(string key, string value, ReplicaType replica);
	string readKey(string key);
	bool updateKeyValue(string key, string value, ReplicaType replica);
	bool deletekey(string key);

        ReplicaType ConvertToReplicaType(string replicaTypeString);
    
	// stabilization protocol - handle multiple failures
	void stabilizationProtocol();
	
	// compare current state
	bool isCurrentStateChange(vector<Node> curMemList, vector<Node> ring);
	vector<string> ParseMessageIntoTokens(const string& message, size_t dataSize);

	~KVStoreAlgorithm();
};

#endif /* MP2NODE_H_ */
