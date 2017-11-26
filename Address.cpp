//
//  Address.cpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 11/25/17.
//  Copyright © 2017 Monika Sharma. All rights reserved.
//

#include "Address.h"

/**
 * Constructor
 */
Address::Address() {
    memset(&addr, 0, sizeof(addr));
}

/**
 * Copy constructor
 */
Address::Address(const Address &anotherAddress) {
    // strcpy(addr, anotherAddress.addr);
    memcpy(&addr, &anotherAddress.addr, sizeof(addr));
}

/**
 * Assignment operator overloading
 */
Address& Address::operator =(const Address& anotherAddress) {
    // strcpy(addr, anotherAddress.addr);
    memcpy(&addr, &anotherAddress.addr, sizeof(addr));
    return *this;
}

/**
 * Compare two Address objects
 */
bool Address::operator ==(const Address& anotherAddress) {
    return !memcmp(this->addr, anotherAddress.addr, sizeof(this->addr));
}

Address::Address(string address) {
    size_t pos = address.find(":");
    int id = stoi(address.substr(0, pos));
    short port = (short)stoi(address.substr(pos + 1, address.size()-pos-1));
    memcpy(&addr[0], &id, sizeof(int));
    memcpy(&addr[4], &port, sizeof(short));
}

string Address::getAddress() {
    int id = 0;
    short port;
    memcpy(&id, &addr[0], sizeof(int));
    memcpy(&port, &addr[4], sizeof(short));
    return to_string(id) + ":" + to_string(port);
}


