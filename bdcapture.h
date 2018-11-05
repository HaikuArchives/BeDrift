#ifndef BFINTERFACE_H
#define BFINTERFACE_H

extern "C" { 
#include <pcap.h>
}

#include <kernel/OS.h>
#include <Locker.h>
#include "connection.h"

#define SNAPLEN    262144
#define WRAPLEN    262144

class BDView;

class BDCapture {
public:
	BDCapture(BDView *);
	~BDCapture();

	bool SetInterface(const char *, bool);
	bool IsValid(void) { return !(pcap == NULL); };
	bool StartThread();
	bool StopThread();

	Connection *AddConnection(struct in_addr *, short,
	                          struct in_addr *, short);
	Connection *MatchConnection(struct in_addr *, short,
	                            struct in_addr *, short);
	void RemoveConnection(Connection *);
	void SweepConnections(void);
	int PktOffset(void) { return pkt_offset; }
	pcap_t *Pcap(void) { return pcap; }
	BDView *View() { return view; };
	
private:
	BDView *view;
	Connection *conns;
	BLocker *lock;
	char *device;
	int promisc;
	pcap_t *pcap;
	thread_id pcap_thread;
	char pcap_errbuf[PCAP_ERRBUF_SIZE];	
	int pkt_offset;
};

#endif /* BFINTERFACE_H */
