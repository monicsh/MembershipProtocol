#**********************
#*
#* Progam Name: MP1. Membership Protocol.
#*
#* Current file: Makefile
#* About this file: Build Script.
#* 
#***********************

CFLAGS =  -g -std=c++11

all: Application

Application: MembershipProtocol.o EmulNet.o Application.o Log.o Params.o Member.o MessageQueue.o Trace.o KVStoreAlgorithm.o Node.o HashTable.o Entry.o Message.o RawMessage.o
	g++ -o Application MembershipProtocol.o EmulNet.o Application.o Log.o Params.o MessageQueue.o Member.o Trace.o KVStoreAlgorithm.o Node.o HashTable.o Entry.o Message.o RawMessage.o ${CFLAGS}

MembershipProtocol.o: MembershipProtocol.cpp MembershipProtocol.h Log.h Params.h Member.h EmulNet.h MessageQueue.h
	g++ -c MembershipProtocol.cpp ${CFLAGS}

EmulNet.o: EmulNet.cpp EmulNet.h Params.h Member.h
	g++ -c EmulNet.cpp ${CFLAGS}

Application.o: Application.cpp Application.h Member.h Log.h Params.h Member.h EmulNet.h MessageQueue.o
	g++ -c Application.cpp ${CFLAGS}

Log.o: Log.cpp Log.h Params.h Member.h
	g++ -c Log.cpp ${CFLAGS}

Params.o: Params.cpp Params.h 
	g++ -c Params.cpp ${CFLAGS}

Member.o: Member.cpp Member.h
	g++ -c Member.cpp ${CFLAGS}

Trace.o: Trace.cpp Trace.h
	g++ -c Trace.cpp ${CFLAGS}

KVStoreAlgorithm.o: KVStoreAlgorithm.cpp KVStoreAlgorithm.h EmulNet.h Params.h Member.h Trace.h Node.h HashTable.h Log.h Params.h Message.h
	g++ -c KVStoreAlgorithm.cpp ${CFLAGS}

MessageQueue.o: IMessageQueue.h MessageQueue.cpp MessageQueue.h
	g++ -c MessageQueue.cpp ${CFLAGS}

RawMessage.o: RawMessage.h RawMessage.cpp
	g++ -c RawMessage.cpp ${CFLAGS}

Node.o: Node.cpp Node.h Member.h
	g++ -c Node.cpp ${CFLAGS}

HashTable.o: HashTable.cpp HashTable.h common.h Entry.h
	g++ -c HashTable.cpp ${CFLAGS}

Entry.o: Entry.cpp Entry.h Message.h
	g++ -c Entry.cpp ${CFLAGS}

Message.o: Message.cpp Message.h Member.h common.h
	g++ -c Message.cpp ${CFLAGS}

clean:
	rm -rf *.o Application dbg.log msgcount.log stats.log machine.log
