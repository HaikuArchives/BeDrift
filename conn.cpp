#include "connection.h"
#include <stdio.h>
#include <TranslationUtils.h>
#include <string.h>
#include <unistd.h>
#include "bedrift.h"
#include <Application.h>
#include <Bitmap.h>

#include "bd.h"
#include "bdcapture.h"

Connection::Connection(struct in_addr *_src, short _sport,
	           struct in_addr *_dst, short _dport)
{
	src = *_src;
	sport = _sport;
	dst = *_dst;
	dport = _dport;
	buffer = NULL;
	sequence = 0;
	finished = false;
	adjustment = 0;
}

void Connection::Empty(void)
{
	if (buffer)
		delete buffer;
	buffer = NULL;
}

Connection::~Connection(void)
{
	Empty();
}

bool Connection::Match(struct in_addr *_src, short _sport,
	                   struct in_addr *_dst, short _dport)
{

	if (sport == _sport && dport == _dport &&
	    memcmp(&src, _src, sizeof(src)) == 0 &&
	    memcmp(&dst, _dst, sizeof(dst)) == 0) {
		return true;
	}
	return false;
}

void Connection::Push(const u_char *data, int offset, int len)
{
	char *wrptr = (char*)data;
	if (!buffer)
		buffer = new BMallocIO();
	char *st = strstr((char*)data, "\r\n\r\n");
	if (st && st <= wrptr + len) {
		adjustment = st - wrptr + 4;
		len -= adjustment;
		wrptr += adjustment;
		if (offset != 0 && offset < adjustment) {
			adjustment += offset;
			offset = 0;
		}
	}
	if (adjustment > 0 && offset >= adjustment)
			offset -= adjustment;
		
	buffer->Seek(offset, SEEK_SET);
	buffer->Write(wrptr, len);
//printf("%p: pushed %d bytes @ %d, buffer is %d bytes\n", this, len, offset, buffer->BufferLength());
}

void Connection::ExtractMedia(BDCapture *cap)
{
	if (!buffer || buffer->BufferLength() == 0)
		return;
//	printf("%p: extractmedia() for %ld bytes\n", this, buffer->BufferLength());
	char *buff = (char*)buffer->Buffer();
	buff[buffer->BufferLength()] = '\0';
	BBitmap *bb = BTranslationUtils::GetBitmap(buffer);
	if (bb) {
		if (bb->IsValid())
			cap->View()->AddBitmap(bb);
		else
			delete bb;
	}
	delete buffer;
	buffer = NULL;
	SetFinished();
}
