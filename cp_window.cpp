#include <Box.h>
#include <View.h>
#include <stdio.h>
#include <sys/ioctl.h>
extern "C" {
#include <sys/socket.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <stdlib.h>
#include <string.h>
}

#include "bedrift.h"
#include "cp.h"

CPWindow::CPWindow() : BWindow(BRect(0, 0, 0, 0), 
                                "BeDrift Control Panel", 
                                B_TITLED_WINDOW, 
                                B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE)
{
	float vert = 0.0;
	views = NULL;
	quitb = NULL;
	viewsx = viewsy = 0;
	
	bg = new BView(Bounds(), "background", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	AddChild(bg);
	bg->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
	BStringView *title = new BStringView(BRect(0,0,0,0), "title", "BeDrift",
	                                     B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	BStringView *vers = new BStringView(BRect(0,0,0,0), "vers", "0.0.1 alpha",
	                                    B_FOLLOW_ALL_SIDES, B_WILL_DRAW);

	title->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	title->SetFont(be_bold_font);
	title->SetFontSize(14);
	title->ResizeToPreferred();

	vers->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	vers->ResizeToPreferred();

	quitb = new BButton(BRect(0,0,0,0), "quit", "Exit", 
	                     new BMessage(DN_EXIT));
	quitb->ResizeToPreferred();
	quitb->SetEnabled(true);

	BuildIFList();
	
	vert = title->Bounds().Height() + vers->Bounds().Height();
	vert += quitb->Bounds().Height() + 10;

	if ((int)title->Bounds().Width() > viewsx)
		ResizeTo(title->Bounds().Width() + 2, viewsy + vert + 1);
	else
		ResizeTo(viewsx + 1, viewsy + vert + 1);

	bg->AddChild(title);
	bg->AddChild(vers);
	bg->AddChild(quitb);
		
	vert = 0;
	title->MoveTo((Bounds().Width() - title->Bounds().Width()) / 2, vert);
	vert = title->Bounds().Height();
	vers->MoveTo((Bounds().Width() - vers->Bounds().Width()) / 2, vert);
	vert += vers->Bounds().Height() + 2;
	vert += 2;


	CPView *v = views;
	while (v) {
		bg->AddChild(v);
		v->MoveTo(0, vert);
		vert += v->Bounds().Height() + 1;
		v = v->Next();
	}
	
	quitb->MoveTo((Bounds().IntegerWidth() - quitb->Bounds().IntegerWidth()) / 2, vert);
	
	BScreen screen;
	MoveTo((screen.Frame().Width() - Bounds().Width()) / 3,
	       (screen.Frame().Height() - Bounds().Height()) / 3);
}

CPWindow::~CPWindow()
{
	while (views) {
		CPView *old = views;
		views = old->Next();
		RemoveChild(old);
	}
	RemoveChild(bg);
}

void CPWindow::BuildIFList(void)
{
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	int rv;
	int ifcount = 0;
	struct ifconf ifc;
	struct ifreq *ifr = NULL;

	if (s > 0) {
		int tryn = 1;
		size_t dsize = sizeof(struct ifreq);
		ssize_t ds = 0;
		ifc.ifc_buf = NULL;
		do {
			ds = tryn++ * dsize;
			if (ifc.ifc_buf)
				free(ifc.ifc_buf);
			ifc.ifc_buf = malloc(ds);
			ifc.ifc_len = ds;
			rv = ioctl(s, SIOCGIFCONF, &ifc, sizeof(ifc));
		} while(rv == 0 && ifc.ifc_len == ds);
		ifcount = ifc.ifc_len / dsize;
		close(s);
	}
	if (ifcount > 0) {
		int i;
		ifr = (struct ifreq *)ifc.ifc_buf;
		
		for (i=0; i < ifcount; i++) {
			if (strcmp("loop0", ifr[i].ifr_name) == 0)
				continue;
			CPView *view = new CPView(this, ifr[i].ifr_name);
			if (!views) {
				views = view;
			} else {
				CPView *iter = views;
				while (iter->Next())
					iter = iter->Next();

				if (iter)
					iter->SetNext(view);
			}
		}
		free(ifc.ifc_buf);
	}
}

void CPWindow::AddChildSize(BRect rect)
{
	if (rect.Width() > viewsx)
		viewsx = (int)rect.Width();
	viewsy += (int)rect.Height();
}

void CPWindow::SetStarted(char *dev)
{
	Lock();
	CPView *iter = views;
	while (iter) {
		if (strcmp(dev, iter->Device()) == 0) {
			iter->SetRunning();
			Unlock();
			return;
		}
		iter = iter->Next();
	}
	Unlock();
}

void CPWindow::SetStopped(char *dev)
{
	bool unlock = false;
	if (!IsLocked()) {
		unlock = true;
		Lock();
	}
	CPView *iter = views;
	while (iter) {
		if (strcmp(dev, iter->Device()) == 0) {
			iter->SetStopped();
			if (unlock)
				Unlock();
			return;
		}
		iter = iter->Next();
	}
	if (unlock)
		Unlock();
}

void CPWindow::MessageReceived(BMessage *msg)
{
//	printf("BeDrift Control Panel::Message Received\n");
	switch (msg->what) {
		case DN_EXIT:
			QuitRequested();
			break;
	}
}

void CPWindow::DisableExit(void)
{
	Lock();
	if (quitb)
		quitb->SetEnabled(false);
	Unlock();
}

void CPWindow::EnableExit(void)
{
	Lock();
	if (quitb)
		quitb->SetEnabled(true);
	Unlock();
}

bool CPWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return(true);
}
