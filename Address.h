//
//  Address.hpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 11/25/17.
//  Copyright © 2017 Monika Sharma. All rights reserved.
//

#ifndef Address_hpp
#define Address_hpp

#include "stdincludes.h"

/**
 * CLASS NAME: Address
 *
 * DESCRIPTION: Class representing the address of a single node
 */
class Address {
public:
    char addr[6];
    Address() {}
    // Copy constructor
    Address(const Address &anotherAddress);
    // Overloaded = operator
    Address& operator =(const Address &anotherAddress);
    bool operator ==(const Address &anotherAddress);
    Address(string address) {
        size_t pos = address.find(":");
        int id = stoi(address.substr(0, pos));
        short port = (short)stoi(address.substr(pos + 1, address.size()-pos-1));
        memcpy(&addr[0], &id, sizeof(int));
        memcpy(&addr[4], &port, sizeof(short));
    }
    string getAddress() {
        int id = 0;
        short port;
        memcpy(&id, &addr[0], sizeof(int));
        memcpy(&port, &addr[4], sizeof(short));
        return to_string(id) + ":" + to_string(port);
    }
    void init() {
        memset(&addr, 0, sizeof(addr));
    }
};

#endif /* Address_hpp */
