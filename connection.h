#ifndef CONNECTION_H
#define CONNECTION_H

#include <arpa/inet.h>
#include <DataIO.h>
#include <stdio.h>

class BDCapture;

class Connection
{
public:
	Connection(struct in_addr *src, short sport,
	           struct in_addr *dst, short dport);
	~Connection(void);
	void Empty(void);
	bool Match(struct in_addr *src, short sport,
	           struct in_addr *dst, short dport);
	void Push(const u_char *, int, int);
	void ExtractMedia(BDCapture *);
	void SetFinished(void) { finished = true; }
	bool IsFinished(void) { return finished == true; }
	int Length(void) { return buffer ? buffer->BufferLength() : 0; }
	unsigned long Sequence(void) { return sequence; }
	void SetSequence(unsigned long sq) { sequence = sq; }
	Connection *next;
private:
	struct in_addr src;
	struct in_addr dst;
	short sport;
	short dport;
	BMallocIO *buffer;
	bigtime_t time;
	bool finished;
	unsigned long sequence;
	int adjustment;
};

#endif /* CONNECTION_H */
