#include <string.h>
#include <kernel/OS.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include "bdcapture.h"
sem_id sem_of_death = -1;

static int get_link_level_hdr_length(int type)
{
    switch (type) {
        case DLT_EN10MB:
            return 14;

        case DLT_SLIP:
            return 16;

        case DLT_SLIP_BSDOS:
            return 24;

        case DLT_NULL:
#ifdef DLT_LOOP
        case DLT_LOOP:
#endif
            return 4;

        case DLT_PPP:
#ifdef DLT_C_HDLC
        case DLT_C_HDLC:
#endif
#ifdef DLT_PPP_SERIAL
        case DLT_PPP_SERIAL:
#endif
            return 4;

        case DLT_PPP_BSDOS:
            return 24;

        case DLT_FDDI:
            return 21;

        case DLT_IEEE802:
            return 22;

        case DLT_ATM_RFC1483:
            return 8;

        case DLT_RAW:
            return 0;

#ifdef DLT_ATM_CLIP
        case DLT_ATM_CLIP:	/* Linux ATM defines this */
            return 8;
#endif

#ifdef DLT_LINUX_SLL
        case DLT_LINUX_SLL:	/* fake header for Linux cooked socket */
            return 16;
#endif

        default:;
    }
    fprintf(stderr, "BeDrift: unknown data link type %d", type);
	return 0;
}

void process_packet(u_char *user, const struct pcap_pkthdr *hdr,
                    const u_char *pkt)
{
	BDCapture *pthis = (BDCapture *)user;
	struct ip theip;
	struct tcphdr tcp;
	struct in_addr src, dst;
	int off, len;
	Connection *c = NULL;
	int pkt_offset = pthis->PktOffset();

	memcpy(&theip, pkt + pkt_offset, sizeof(theip));
	
	memcpy(&src, &theip.ip_src, sizeof(theip.ip_src));
	memcpy(&dst, &theip.ip_dst, sizeof(theip.ip_dst));
	
	memcpy(&tcp, pkt + pkt_offset + (theip.ip_hl << 2), sizeof(tcp));
	off = pkt_offset + (theip.ip_hl << 2) + (tcp.th_off << 2);
	len = hdr->caplen - off;

	tcp.th_sport = ntohs(tcp.th_sport);
	tcp.th_dport = ntohs(tcp.th_dport);
	tcp.th_seq = ntohl(tcp.th_seq);
	
/*
printf("%08lx.%d > %08lx.%d: %c%c%c%c %lu:%lu(%d)\n",
       theip.ip_src, tcp.th_sport,
       theip.ip_dst, tcp.th_dport,
       tcp.th_flags & TH_SYN ? 'S' : ' ',
       tcp.th_flags & TH_FIN ? 'F' : ' ',
       tcp.th_flags & TH_ACK ? 'A' : ' ',
       tcp.th_flags & TH_RST ? 'R' : ' ',
       tcp.th_seq, tcp.th_seq + len, len);
*/
	c = pthis->MatchConnection(&src, tcp.th_sport, &dst, tcp.th_dport);
	if (!c) {
		if ((tcp.th_flags & TH_SYN)) {
			c = pthis->AddConnection(&src, tcp.th_sport, &dst, tcp.th_dport);
			c->SetSequence(tcp.th_seq);
		}

		return;
	}

	if ((tcp.th_flags & TH_RST)) {
		pthis->RemoveConnection(c);
		c = NULL;
		return;
	}
	
	if (len > 0) {
		unsigned long offset = tcp.th_seq;

		if (offset < c->Sequence())
			offset = 0xffffffff - (c->Sequence() - offset);
		else
			offset -= c->Sequence();

		if (offset <= (c->Length() + WRAPLEN))
			c->Push(pkt + off, offset, len);
	}
	if ((tcp.th_flags & TH_FIN))
		c->ExtractMedia(pthis);

	pthis->SweepConnections();
}

int32 pcap_process(void *_this)
{
	BDCapture *pthis = (BDCapture *)_this;
	pcap_t *pc = pthis->Pcap();
	while (acquire_sem_etc(sem_of_death, 1, B_TIMEOUT, 0) != 0) {
		pcap_dispatch(pc, -1, process_packet, (u_char*)pthis);
	}
	return 0;
}

BDCapture::BDCapture(BDView *v)
{
	promisc = 1;
	pcap_thread = -1;
	conns = NULL;
	view = v;
	lock = new BLocker("BFInterface");
	
	if (sem_of_death == -1)
		sem_of_death = create_sem(0, "pcap_sem_of_death");

	memset(pcap_errbuf, 0, sizeof(pcap_errbuf));
}


BDCapture::~BDCapture(void)
{
printf("BDCapture::~BDCapture()\n");	
	if (pcap_thread >= 0) {
		status_t ev = -1;
		release_sem(sem_of_death);
		wait_for_thread(pcap_thread, &ev);
		pcap_thread = -1;
	}
	Connection *c = conns;
	while (c) {
		conns = c->next;
		delete c;
		c = conns;
	}	
	
	if (pcap)
		pcap_close(pcap);
}

bool BDCapture::SetInterface(const char *path, bool set_promisc)
{
	char *interface = (char *)path;

	promisc = set_promisc;
		
	if ((!interface || interface[0] == '\0') && 
	    !(interface = pcap_lookupdev(pcap_errbuf))) {
	    printf("interface = %s\n", interface);
		return false;
	}
	pcap = pcap_open_live(interface, SNAPLEN, promisc, 1000, pcap_errbuf);

	if (!pcap) {
		printf("Failed to open pcap for %s, %s\n", interface, pcap_errbuf);
		return false;
	}
	if (pcap) {
		device = strdup(interface);
		pkt_offset = get_link_level_hdr_length(pcap_datalink(pcap));
		return StartThread();
	}
	return true;
}

bool BDCapture::StartThread()
{
	pcap_thread = spawn_thread(pcap_process, "pcap_process", B_NORMAL_PRIORITY, this);
	if (pcap_thread >= 0) {
		if (resume_thread(pcap_thread) == B_OK)
			return true;
	}
	return false;
}

bool BDCapture::StopThread()
{
	if (pcap_thread >= 0) {
		if (kill_thread(pcap_thread) == B_NO_ERROR)
			return true;
	}
	return false;
}

Connection *BDCapture::MatchConnection(struct in_addr *src, short sport,
                                       struct in_addr *dst, short dport)
{
	lock->Lock();
	Connection *c = conns;
	
	while (c) {
		if (c->Match(src, sport, dst, dport)) {
			lock->Unlock();
			return c;
		}
		c = c->next;
	}
	lock->Unlock();
	return NULL;
}

Connection *BDCapture::AddConnection(struct in_addr *src, short sport,
                                       struct in_addr *dst, short dport)
{
	Connection *c = new Connection(src, sport, dst, dport);
	lock->Lock();	
	c->next = conns;
	conns = c;
	lock->Unlock();
	return c;
}

void BDCapture::RemoveConnection(Connection *c)
{
	lock->Lock();
	Connection *d = conns;
	
	if (d != c) {
		while (d && d->next != c)
			d = d->next;
		if (d)
			d->next = c->next;
	} else
		conns = c->next;
	c->Empty();
	delete c;
	lock->Unlock();
}

void BDCapture::SweepConnections(void)
{
	lock->Lock();
	Connection *c = conns, *d = NULL;
	while (c) {
		if (c->IsFinished()) {
			if (d)
				d->next = c->next;
			else
				conns = c->next;
			d = c->next;
			delete c;
			c = d;
			continue;
		}
		d = c;
		c = c->next;
	}
	lock->Unlock();
}
