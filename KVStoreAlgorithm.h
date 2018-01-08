/**********************************
 * FILE NAME: MP2Node.h
 *
 * DESCRIPTION: MP2Node class header file
 **********************************/

#ifndef MP2NODE_H_
#define MP2NODE_H_

#include "stdincludes.h"
#include "EmulNet.h"
#include "Node.h"
#include "HashTable.h"
#include "Log.h"
#include "Params.h"
#include "Message.h"
#include "MessageQueue.h"

// This class encapsulates all the key-value store functionality including:
//  1) Ring
//  2) Stabilization Protocol
//  3) Server side CRUD APIs
//  4) Client side CRUD APIs
class KVStoreAlgorithm
{
private:
    struct QuoromDetail
    {
        QuoromDetail() : replyCounter(0) { }

        MessageType transMsgType;
        int reqTime;
        string reqKey;
        unsigned replyCounter;
    };

    struct ActionOnReplicaNode
    {
        Node node;
        MessageType msgType;
        ReplicaType replicaType;
    };

    struct ViolatedNodeSet
    {
        vector<ActionOnReplicaNode> actionOnReplicaNode;
    };

    // Vector holding the next two neighbors in the ring who have my replicas
    vector<Node> m_hasMyReplicas;

    // Vector holding the previous two neighbors in the ring whose replicas I have
    vector<Node> m_haveReplicasOf;

    vector<Node> m_ring;
    HashTable * m_dataStore;
    Member *m_memberNode;
    Params *m_parameters;
    EmulNet * m_networkEmulator;
    Log * m_logger;
    IMessageQueue * m_queue;

    // container for tracking quorom for READ messages
    std::map<int, struct QuoromDetail> m_quorumRead;

    // container for tracking quorom for DELETE messages
    std::map<int, struct QuoromDetail> m_quorum;

    vector<Node> getMembershipList();
    size_t hashFunction(string key);

    void sendMessageToReplicas(vector<Node> replicaNodes, MessageType msgType, string key);
    void sendMessageToReplicas(vector<Node> replicaNodes, MessageType msgType, string key, string value);
    void updateQuorumRead(MessageType msgType, string key);
    void updateQuorum(MessageType msgType, string key);
    void checkQuoromTimeout();
    void checkReadQuoromTimeout();
    bool isTimedout(QuoromDetail& quoromDetail);


    // Server side  API. The function does the following:
    //  1) read/create/update/delete key value from/into the local hash table
    //  2) Return true or false based on success or failure
    bool createKeyValue(string key, string value, ReplicaType replica);
    string readKey(string key);
    bool updateKeyValue(string key, string value, ReplicaType replica);
    bool deletekey(string key);

    ReplicaType ConvertToReplicaType(string replicaTypeString);

    int myPositionInTheRing();
    int findfirstSuccessorIndex(int myPos);
    int findSecondSuccessorIndex(int myPos);
    int findfirstPredeccesorIndex(int myPos);
    int findSecondPredeccesorIndex(int myPos);
    void setHasMyReplicasIfEmpty(int succ_1, int succ_2);
    void setHaveReplicasOfIfEmpty(int pred_1, int pred_2);
    void sendMessageToUpdateReplicaInfoFromPrimary(const string &key, const string &keyValue, int successorFirstIndex, int successorSecondIndex);
    void sendMessageToUpdateReplicaInfoFromSecondary(const string &key, const string &keyValue, int predeccesorFirstIndex, int successorFirstIndex);
    
    // stabilization protocol - handle multiple failures
    void stabilizationProtocol();

    // compare current state
    bool isCurrentStateChange(vector<Node> curMemList, vector<Node> ring);
    vector<string> ParseMessageIntoTokens(const string& message, size_t dataSize);

    void processReadMessage(const Address &fromaddr, bool isCoordinator, const vector<string> &messageParts, int transID);
    void processCreateMessage(const Address &fromaddr, bool isCoordinator, const vector<string> &messageParts, int transID);
    void processUpdateMessage(const Address &fromaddr, bool isCoordinator, const vector<string> &messageParts, int transID);
    void processDeleteMessage(const Address &fromaddr, bool isCoordinator, const vector<string> &messageParts, int transID);
    void processReplyMessage(bool isCoordinator, const vector<string> &messageParts, int transID);
    void processReadReplyMessage(bool isCoordinator, const vector<string> &messageParts, int transID);
    int findMyPosInReplicaSet(vector<Node> &replicaSet);
    static string parseValue(string valueToParse);
    ReplicaType parseReplicaType(string valueToParse);

public:
    virtual ~KVStoreAlgorithm();
    KVStoreAlgorithm(
        Member *memberNode,
        Params *par,
        EmulNet *emulNet,
        Log *log,
        Address *addressOfMember,
        IMessageQueue* queue);

    Member * getMemberNode() {
        return this->m_memberNode;
    }

    // ring functionalities
    void updateRing();

    // client side CRUD APIs
    void clientCreate(string key, string value);
    void clientRead(string key);
    void clientUpdate(string key, string value);
    void clientDelete(string key);

    // receive messages from Emulnet
    bool recvLoop();

    // handle messages from receiving queue
    void checkMessages();

    // find the addresses of nodes that are responsible for a key
    vector<Node> findNodes(string key);
};

#endif /* MP2NODE_H_ */
