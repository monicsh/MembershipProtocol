//
//  EM.hpp
//  MembershipProtocol
//
//  Created by Monika Sharma on 11/26/17.
//  Copyright © 2017 Monika Sharma. All rights reserved.
//

#ifndef EM_hpp
#define EM_hpp

#include <stdio.h>
#include "Address.h"
#endif /* EM_hpp */
#define ENBUFFSIZE 30000

/**
 * Struct Name: en_msg
 */
typedef struct en_msg {
    // Number of bytes after the class
    int size;
    // Source node
    Address from;
    // Destination node
    Address to;
}en_msg;

/**
 * Class Name: EM
 */
class EM {
private:
    int nextid;
    
public: // TODO make this private once emulnet is understood
    en_msg* buff[ENBUFFSIZE];
    int currbuffsize;

public:
    EM& operator = (EM &anotherEM);
    void incrementNextId();
    void resetNextId();
    int getNextId() const;
    int getCurrBuffSize();
    void setNextId(int nextid);
    void settCurrBuffSize(int currbuffsize);
    virtual ~EM();
};
