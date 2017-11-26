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

string Address::getAddressLogFormatted() {
    // %d.%d.%d.%d:%d
    char formatstring[30]={0};
    sprintf(formatstring, "%d.%d.%d.%d:%d", addr[0], addr[1],addr[2],addr[3], (short*)addr[4]);

    return formatstring;
}

string Address::getAddress() {
    int id = 0;
    short port;
    memcpy(&id, &addr[0], sizeof(int));
    memcpy(&port, &addr[4], sizeof(short));
    return to_string(id) + ":" + to_string(port);
}

char Address::getAddrByte(int pos) {
    if (pos >= 0 && pos < MAX_BYTES) {
        return addr[pos];
    }

    throw new runtime_error("bad byte requested");
}


