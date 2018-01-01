/**********************************
 * FILE NAME: MP2Node.cpp
 *
 * DESCRIPTION: MP2Node class definition
 **********************************/
#include "KVStoreAlgorithm.h"

ReplicaType KVStoreAlgorithm::ConvertToReplicaType(string replicaTypeString)
{
    if (std::stoi(replicaTypeString) == 1) {
        return SECONDARY;
    } else if (std::stoi(replicaTypeString) == 2) {
        return TERTIARY;
    }

    return PRIMARY;
}

/**
 * constructor
 */
KVStoreAlgorithm::KVStoreAlgorithm(
    Member *memberNode,
    Params *par,
    EmulNet * emulNet,
    Log * log,
    Address * address,
    IMessageQueue * queue )
{
    this->memberNode = memberNode;
    this->par = par;
    this->emulNet = emulNet;
    this->log = log;
    this->m_queue = queue;

    ht = new HashTable();

    this->memberNode->addr = *address;
}

/**
 * Destructor
 */
KVStoreAlgorithm::~KVStoreAlgorithm() {
    delete ht;
    delete memberNode;
}

/**
 * FUNCTION NAME: updateRing
 *
 * DESCRIPTION: This function does the following:
 *                              1) Gets the current membership list from the Membership Protocol (MP1Node)
 *                                 The membership list is returned as a vector of Nodes. See Node class in Node.h
 *                              2) Constructs the ring based on the membership list
 *                              3) Calls the Stabilization Protocol
 */
void KVStoreAlgorithm::updateRing()
{
    /*
     * Step 1. Get the current membership list from Membership Protocol (mp1)
     */
    auto curMemList = getMembershipList();
	

    /*
     * Step 2. Construct the ring. Sort the list based on the hashCode
     */
    sort(curMemList.begin(), curMemList.end());

    // has ring changed?
    size_t length_of_ring = ring.size();
    size_t length_of_memList = curMemList.size();

    bool hasRingChanged = (length_of_ring != length_of_memList);
    if (!hasRingChanged) {
        for (int i = 0; i < length_of_memList; i++) {
			auto hashCurrML = curMemList[i].getHashCode();
			auto hashRing = ring[i].getHashCode();
            if ( hashCurrML != hashRing ) {
                hasRingChanged = true;
                break;
            }
        }
    }

    /*
     * Step 3: Run the stabilization protocol IF REQUIRED
     */
    // Run stabilization protocol if the hash table size is greater than zero and if there
    // has been a changed in the ring
    if (hasRingChanged){
		// update ring
		this->ring = curMemList;
		if (!this->ht->isEmpty()) {
			stabilizationProtocol();
		}
    }
}

/**
 * compare two vectors
 * return : true if  state changed
 *			false on state unchanged
 */
bool KVStoreAlgorithm::isCurrentStateChange(vector<Node> curMemList, vector<Node> ring){

    size_t  length_of_ring = ring.size();
    size_t  length_of_memList = curMemList.size();

    if (length_of_ring != length_of_memList){
        return true;
    } else {
        // if length equal
        for (int i = 0; i< length_of_memList; i++){
            if (curMemList[i].getHashCode() != ring[i].getHashCode()){
                return true;
            }
        }
        return false;

    }

}


/**
 * FUNCTION NAME: getMemberhipList
 *
 * DESCRIPTION: This function goes through the membership list from the Membership protocol/MP1 and
*                               i) generates the hash code for each member
*                               ii) populates the ring member in MP2Node class
*                               It returns a vector of Nodes. Each element in the vector contain the following fields:
*                               a) Address of the node
*                               b) Hash code obtained by consistent hashing of the Address
 */
vector<Node> KVStoreAlgorithm::getMembershipList()
{
    vector<Node> curMemList;

    for (auto i = 0 ; i < this->memberNode->memberList.size(); i++ ) {

        int id = this->memberNode->memberList.at(i).getid();
        short port = this->memberNode->memberList.at(i).getport();

        Address addressOfThisMember;
        memcpy(&addressOfThisMember.m_addr[0], &id, sizeof(int));
        memcpy(&addressOfThisMember.m_addr[4], &port, sizeof(short));

        curMemList.emplace_back(Node(addressOfThisMember));
    }
    return curMemList;
}

/**
 * FUNCTION NAME: hashFunction
 *
 * DESCRIPTION: This functions hashes the key and returns the position on the ring
 *                              HASH FUNCTION USED FOR CONSISTENT HASHING
 *
 * RETURNS:
 * size_t position on the ring
 */
size_t KVStoreAlgorithm::hashFunction(string key) {
    std::hash<string> hashFunc;
    size_t ret = hashFunc(key);
    return ret%RING_SIZE;
}

void KVStoreAlgorithm::sendMessageToReplicas(vector<Node> replicaNodes, MessageType msgType, string key){
    for (auto it = replicaNodes.begin(); it != replicaNodes.end(); it++){
        Address toaddr = it->nodeAddress;
        Message msg = Message(g_transID, memberNode->addr, msgType, key);

        this->emulNet->ENsend(&memberNode->addr, &toaddr, msg.toString());
    }
}

void KVStoreAlgorithm::sendMessageToReplicas(vector<Node> replicaNodes, MessageType msgType, string key, string value){
    int replica =  0;
    for (auto it = replicaNodes.begin(); it != replicaNodes.end(); it++){
        Address toaddr = it->nodeAddress;
        Message msg = Message(g_transID, memberNode->addr, msgType, key, value, static_cast<ReplicaType>(replica++));

        this->emulNet->ENsend(&memberNode->addr, &toaddr, msg.toString());
    }
}

void KVStoreAlgorithm::updateQuorumRead(MessageType msgType, string key){
    // add quorom counter = 0 for each sent READ message triplet sent above
    this->quorumRead[g_transID].transMsgType = msgType;
    this->quorumRead[g_transID].reqTime = this->par->getcurrtime();
    this->quorumRead[g_transID].reqKey = key;

    g_transID++;
}

void KVStoreAlgorithm::updateQuorum(MessageType msgType, string key){
    this->quorum[g_transID].transMsgType = msgType;
    this->quorum[g_transID].reqTime = this->par->getcurrtime();
    this->quorum[g_transID].reqKey = key;

    g_transID++;
}

void KVStoreAlgorithm::clientCreate(string key, string value) {
    sendMessageToReplicas(findNodes(key), CREATE, key, value);
    updateQuorum(CREATE, key);
}

void KVStoreAlgorithm::clientRead(string key){
    sendMessageToReplicas(findNodes(key), READ, key);
    updateQuorumRead(READ, key);
}

void KVStoreAlgorithm::clientUpdate(string key, string value){
    sendMessageToReplicas(findNodes(key), UPDATE, key, value);
	updateQuorum(UPDATE, key);
}

void KVStoreAlgorithm::clientDelete(string key){
	sendMessageToReplicas(findNodes(key), DELETE, key);
	updateQuorum(DELETE, key);
}


/**
 * coordinator dispatches messages to corresponding nodes
 */
void KVStoreAlgorithm::dispatchMessages(Message message){

}

bool KVStoreAlgorithm::createKeyValue(string key, string value, ReplicaType replica) {
    Entry entry = Entry(value, par->getcurrtime(), replica);
    return this->ht->create(key, entry.convertToString());
}


string KVStoreAlgorithm::readKey(string key) {
	string valueTuple = ht->read(key);
	if (valueTuple.empty()) return "";

	size_t pos = valueTuple.find(":");
	if (pos != std::string::npos) {
		return valueTuple.substr(0, pos);
	}

	return "";
}

bool KVStoreAlgorithm::updateKeyValue(string key, string value, ReplicaType replica) {
    // Update key in local hash table and return true or false
    Entry entry = Entry(value, par->getcurrtime(), replica);
    string keyValue = entry.convertToString();
    return this->ht->update(key, keyValue);

}

bool KVStoreAlgorithm::deletekey(string key) {
    // Delete the key from the local hash table
    return this->ht->deleteKey(key);
}

vector<string> KVStoreAlgorithm::ParseMessageIntoTokens(const string& message, size_t dataSize)
{
	const string delimiter = "::";
	string token;
	size_t pos = 0;
	string messageCopy = message;
	vector<string> messageParts;
	
	//parse the message
	while ((pos = messageCopy.find(delimiter)) != std::string::npos) {
		token = messageCopy.substr(0, pos);
		//std::cout << token << std::endl;
		messageCopy.erase(0, pos + delimiter.length());
		messageParts.push_back(token);
	}
	
	token =  messageCopy.substr(0, dataSize-1);
	messageParts.push_back(token);

	return messageParts;
}

/**
 * FUNCTION NAME: checkMessages
 *
 * DESCRIPTION: This function is the message handler of this node.
 *                              This function does the following:
 *                              1) Pops messages from the queue
 *                              2) Handles the messages according to message types
 */
void KVStoreAlgorithm::checkMessages() {
    /*
     * Implement this. Parts of it are already implemented
     */
    char * data;
    int size;

    /*
     * Declare your local variables here
     */

    // dequeue all messages and handle them
    while ( !m_queue->empty() ) {
        /*
         * Pop a message from the queue
         */
        RawMessage item = m_queue->dequeue();
        data = (char *) item.elt;
        size = item.size;
        
        /*
         * Handle the message types here
         */


        const string message(data, data + size);
		const vector<string> messageParts = ParseMessageIntoTokens(message, size);

        const int transID = stoi(messageParts[0]);
        const Address fromaddr = Address(messageParts[1]);
        const int messageType = std::stoi(messageParts[2]);
		
        const bool isCoordinator = (messageParts[1] == memberNode->addr.getAddress());

        switch(messageType) {

        case MessageType::CREATE:
        {
			const string key = messageParts[3];
			const string value = messageParts[4];
            const ReplicaType replicaType = ConvertToReplicaType(messageParts[5]);

            // key - value
            if (createKeyValue(key, value, replicaType)) {
				//REPLY  send acknoledgement to sender
				Message msg = Message(transID, memberNode->addr, MessageType::REPLY , true);
				Address toaddr = fromaddr;
				
				this->log->logCreateSuccess(&memberNode->addr, isCoordinator, transID, key, value);
				this->emulNet->ENsend(&memberNode->addr, &toaddr, msg.toString());
            } else {
				this->log->logCreateFail(&memberNode->addr, isCoordinator, transID, key, value);
            }
            break;
        }
        case MessageType::READ:
        {
			const string key = messageParts[3];
			
			const string value = readKey(key);

            if (!value.empty()) {

                //READREPLY
                Message msg = Message(transID, memberNode->addr, value);
                Address toaddr = fromaddr;
				
                this->log->logReadSuccess(&memberNode->addr, isCoordinator, transID, key, value);
                this->emulNet->ENsend(&memberNode->addr, &toaddr, msg.toString());

            } else{
                this->log->logReadFail(&memberNode->addr, isCoordinator, transID, key);
            }
            break;
        }
        case MessageType::UPDATE:
        {
			const string key = messageParts[3];
			const string value = messageParts[4];
			const ReplicaType replicaType = ConvertToReplicaType(messageParts[5]);

            // key - value
            if (updateKeyValue(key, value, replicaType)) {
                //REPLY  send acknoledgement to sender
                Message msg = Message(transID, memberNode->addr, MessageType::REPLY , true);
                Address toaddr = fromaddr;
				
				this->log->logUpdateSuccess(&memberNode->addr, isCoordinator, transID, key, value);
                this->emulNet->ENsend(&memberNode->addr, &toaddr, msg.toString());
            }
            else{
                Message msg = Message(transID, memberNode->addr, MessageType::REPLY , false);
                Address toaddr = fromaddr;
				
				this->log->logUpdateFail(&memberNode->addr, isCoordinator, transID, key, value);
                this->emulNet->ENsend(&memberNode->addr, &toaddr, msg.toString());
            }

            break;
        }
        case MessageType::DELETE:
        {
			const string key = messageParts[3];

            if (deletekey(key)) {
				//REPLY  send acknoledgement to sender
				Message msg = Message(transID, memberNode->addr, MessageType::REPLY , true);
				Address toaddr = fromaddr;
				
                this->log->logDeleteSuccess(&memberNode->addr, isCoordinator, transID, key);
				this->emulNet->ENsend(&memberNode->addr, &toaddr, msg.toString());
            } else {
				
				//REPLY  send acknoledgement to sender
				Message msg = Message(transID, memberNode->addr, MessageType::REPLY , false);
				Address toaddr = fromaddr;
				
                this->log->logDeleteFail(&memberNode->addr, isCoordinator, transID, key);
				this->emulNet->ENsend(&memberNode->addr, &toaddr, msg.toString());
            }
            break;
        }
        case MessageType::REPLY:
        {
			const bool success("1" == messageParts[3]);
			
			auto record = quorum.find(transID);
			if (record == quorum.end()) {
				break; // not found
			}
			
			if (success){
				record->second.replyCounter++;
				
				if (record->second.replyCounter == 3) {
					// quorom met; remove this triplet
					
					quorum.erase(record);
					
				} else if (record->second.replyCounter == 2) {
					// log as success on basis of message type; but dont remove
					//TODO : log on basis of message type
					if (record->second.transMsgType == CREATE){
						this->log->logCreateSuccess(&(this->memberNode->addr), isCoordinator, transID, record->second.reqKey, "somevalue");
					} else if (record->second.transMsgType == UPDATE){
						this->log->logUpdateSuccess(&(this->memberNode->addr), isCoordinator, transID, record->second.reqKey, "somevalue");
					
					} else if (record->second.transMsgType == DELETE){
						this->log->logDeleteSuccess(&(this->memberNode->addr), isCoordinator, transID, record->second.reqKey);
					}
					
				} else {
					// =1; hold on
				}
				
			}

            break;
        }
        case MessageType::READREPLY:
        {
			const string value = messageParts[3];

			auto record = quorumRead.find(transID);
			if (record == quorumRead.end()) {
				break; // not found
			}
			
			// else if counter is N-1 (=2 here), logReadSuccess(transID), so that any late message for READ doesnt starts from 1
			// else insert new key with transID with counter 1

			record->second.replyCounter++;
			
			if (record->second.replyCounter == 3) {
				// quorom met; remove this triplet
				
				quorumRead.erase(record);
				
			} else if (record->second.replyCounter == 2) {
				// log as success; but dont remove
				
				this->log->logReadSuccess(&(this->memberNode->addr), isCoordinator, transID, record->second.reqKey, value);
			} else {
				// =1; hold on
			}
			
            break;
        }
				
        } // switch end
    }

    /*
     * This function should also ensure all READ and UPDATE operation
     * get QUORUM replies
     */
	//Handle one count CRUD operation
	//iterate quorum one-by-one
	//for (auto & record : this->quorum){
	
	auto record  = this->quorum.begin();
	while (record != this->quorum.end()){
		if (record->second.replyCounter < 2 and record->second.reqTime <= this->par->getcurrtime() - 5){
			//delete the record and log it as failure
			if (record->second.transMsgType == CREATE){
				this->log->logCreateFail(&(this->memberNode->addr), true, record->first, record->second.reqKey, "somevalue");
			} else if (record->second.transMsgType == READ){
				this->log->logReadFail(&(this->memberNode->addr), true, record->first, record->second.reqKey);
			}
			else if (record->second.transMsgType == UPDATE){
	 
				this->log->logUpdateFail(&(this->memberNode->addr), true, record->first, record->second.reqKey, "somevalue");
	 
			} else if (record->second.transMsgType == DELETE){
	 
				this->log->logDeleteFail(&(this->memberNode->addr), true, record->first, record->second.reqKey);
			}
			record = this->quorum.erase(record);
		} else {
			++record;
		}
	}
	
	
	//READQuorum
	auto recordRead  = this->quorumRead.begin();
	while (recordRead != this->quorumRead.end()){
		if (record->second.replyCounter < 2 and recordRead->second.reqTime <= this->par->getcurrtime() - 5){
				this->log->logReadFail(&(this->memberNode->addr), true, recordRead->first, recordRead->second.reqKey);
			
			recordRead = this->quorumRead.erase(recordRead);
		} else {
			++recordRead;
		}
	}

	
}


/**
 * FUNCTION NAME: findNodes
 *
 * DESCRIPTION: Find the replicas of the given keyfunction
 *                              This function is responsible for finding the replicas of a key
 */
vector<Node> KVStoreAlgorithm::findNodes(string key) {
    size_t pos = hashFunction(key);
    vector<Node> addr_vec;
    if (ring.size() >= 3) {
        // if pos <= min || pos > max, the leader is the min
        if (pos <= ring.at(0).getHashCode() || pos > ring.at(ring.size()-1).getHashCode()) {
            addr_vec.emplace_back(ring.at(0));
            addr_vec.emplace_back(ring.at(1));
            addr_vec.emplace_back(ring.at(2));
        }
        else {
            // go through the ring until pos <= node
            for (int i=1; i<ring.size(); i++){
                Node addr = ring.at(i);
                if (pos <= addr.getHashCode()) {
                    addr_vec.emplace_back(addr);
                    addr_vec.emplace_back(ring.at((i+1)%ring.size()));
                    addr_vec.emplace_back(ring.at((i+2)%ring.size()));
                    break;
                }
            }
        }
    }
    return addr_vec;
}

/**
 * FUNCTION NAME: recvLoop
 *
 * DESCRIPTION: Receive messages from EmulNet and push into the queue (mp2q)
 */
bool KVStoreAlgorithm::recvLoop() {
    if ( memberNode->bFailed ) {
        return false;
    }
    else {
        bool flag;
        flag =  emulNet->ENrecv(&(memberNode->addr), this->m_queue, NULL, 1);
        return flag;
    }
}

/**
 * FUNCTION NAME: stabilizationProtocol
 *
 * DESCRIPTION: This runs the stabilization protocol in case of Node joins and leaves
 *                              It ensures that there always 3 copies of all keys in the DHT at all times
 *                              The function does the following:
 *				1) Ensures that there are three "CORRECT" replicas of all the keys in spite of failures and joins
 *				Note:- "CORRECT" replicas implies that every key is replicated in its two neighboring nodes in the ring
 */
void KVStoreAlgorithm::stabilizationProtocol()
{
	
	// 1. find out my postition in the ring
	size_t myPos = -1;
	int i;
	for (i = 0; i < this->ring.size(); i++) {
		if (ring[i].nodeAddress == this->memberNode->addr){
			myPos = i;
			break;
		}
		
	}
	
	// set successors and predeccesors index
	size_t succ_1 = (i+1)%(this->ring.size());
	size_t succ_2 = (i+2)%(this->ring.size());
	size_t pred_1 = (i-1 + this->ring.size())%(this->ring.size());
	size_t pred_2 = (i-2 + this->ring.size())%(this->ring.size());
	
	// Initialize successors and predeccesors
	if (this->hasMyReplicas.empty() && this->haveReplicasOf.empty()){
		
		//set hasMyReplicas and haveReplicasOf
		this->hasMyReplicas.push_back(ring[succ_1]);
		this->hasMyReplicas.push_back(ring[succ_2]);
		this->haveReplicasOf.push_back(ring[pred_1]);
		this->haveReplicasOf.push_back(ring[pred_2]);
		
	}
	
	// iterate key-value hash table
	for (auto it = this->ht->hashTable.begin(); it != this->ht->hashTable.end(); it++) {
		string key = it->first;
		string value = it->second;
		//extract replicaType from value string
		Entry * entry = new Entry(value);
		ReplicaType replicaType = entry->replica;
		string keyValue = entry->value;
		
		vector<Node> replicaSet = findNodes(key);
		
		if (replicaSet.size() < 3){ return; }
		
		//check this node exists in replicaSet
		int r;
		int myPosInReplica = -1;
		for (r = 0; r < replicaSet.size(); r++){
			if (replicaSet[r].nodeAddress == this->memberNode->addr){
				myPosInReplica = r;
				break;
			}
		}
		
		// TODO : delete record if does not exist in replica set
		
		// node exists in the replicaSet
		if (myPosInReplica >= 0){
			//check my position changed?
			if ((int)(replicaType) != myPosInReplica){
				// position changed
				// update replicatype
				entry->replica = (ReplicaType)(myPosInReplica);
			}
			
			//check others positions
			switch (myPosInReplica){
			case 0:
					if (ring[succ_1].getAddress() != hasMyReplicas[0].getAddress() and
						ring[succ_1].getAddress() != hasMyReplicas[1].getAddress()){
						//new neighbor encounter
						// Send Create message to new node for key, value, SECONDARY
						
						Address toaddr = ring[succ_1].nodeAddress;
						Message msg = Message(g_transID, memberNode->addr, CREATE, key, keyValue, SECONDARY);
						this->emulNet->ENsend(&memberNode->addr, &toaddr, msg.toString());
						
					}
					
					if (ring[succ_2].getAddress() != hasMyReplicas[0].getAddress() and
						ring[succ_2].getAddress() != hasMyReplicas[1].getAddress()){
						//new neighbor encounter
						// Send Create message to new node for key, value, SECONDARY
						
						Address toaddr = ring[succ_2].nodeAddress;
						Message msg = Message(g_transID, memberNode->addr, CREATE, key, keyValue, TERTIARY);
						this->emulNet->ENsend(&memberNode->addr, &toaddr, msg.toString());
						
					}
					g_transID++;
				break;
			case 1:
					if (ring[succ_1].getAddress() != hasMyReplicas[0].getAddress()){
						//new neighbor encounter
						// Send Create message to new node for key, value, SECONDARY
						if (ring[pred_1].getAddress() != haveReplicasOf[0].getAddress()) {
							Address toaddrSucc = ring[succ_1].nodeAddress;
							Address toaddrPred = ring[pred_1].nodeAddress;
							Message msgToSucc = Message(g_transID, memberNode->addr, CREATE, key, keyValue, TERTIARY);
							Message msgToPred= Message(g_transID, memberNode->addr, CREATE, key, keyValue, PRIMARY);
							this->emulNet->ENsend(&memberNode->addr, &toaddrSucc, msgToSucc.toString());
							this->emulNet->ENsend(&memberNode->addr, &toaddrPred, msgToPred.toString());
							g_transID++;
						}
						
					}
				break;
			case 2:
				break;
			}
			
			
			hasMyReplicas.clear();
			haveReplicasOf.clear();
			hasMyReplicas.push_back(ring[succ_1]);
			hasMyReplicas.push_back(ring[succ_2]);
			haveReplicasOf.push_back(ring[pred_1]);
			haveReplicasOf.push_back(ring[pred_2]);
			
			
		}
	}
	
}
