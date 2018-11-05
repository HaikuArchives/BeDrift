#include <Box.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>

#include "cp.h"
#include "bedrift.h"

CPView::CPView(CPWindow *father, char *dev)
	: BView(BRect(0,0,0,0), dev, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	next = NULL;
	
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	device = strdup(dev);
	parent = father;
	
	devstr = new BStringView(BRect(0,0,0,0), "our_device", device,
	                         B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	devstr->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	devstr->SetFont(be_bold_font);
	devstr->ResizeToPreferred();
	int devy = ((int)devstr->Bounds().Height()) / 2;
	
	startb = new BButton(BRect(0,0,0,0), "start", "Start", 
	                     new BMessage(DN_START));
	startb->ResizeToPreferred();
	startb->SetEnabled(true);
		
	stopb = new BButton(BRect(0,0,0,0), "stop", "Stop", 
	                     new BMessage(DN_STOP));
	stopb->ResizeToPreferred();
	stopb->SetTarget(this);
	stopb->SetEnabled(false);

	int buttony = ((int)startb->Bounds().Height()) / 2;	
	if (devy > buttony)
		buttony = devy;

	AddChild(devstr);
	AddChild(startb);
	AddChild(stopb);

	startb->MoveTo(200, buttony);
	stopb->MoveTo(200 + startb->Bounds().Width() + 2, buttony); 
	devstr->MoveTo(2, buttony);
	ResizeTo(200 + startb->Bounds().Width() + stopb->Bounds().Width() + 2,
	         2 * startb->Bounds().Height());
	parent->AddChildSize(Bounds());
}

void CPView::Draw(BRect bounds)
{
	StrokeLine(BPoint(2,2), BPoint(Bounds().Width() - 2, 2));
	BView::Draw(bounds);
}

void CPView::AttachedToWindow()
{
	startb->SetTarget(this);
	stopb->SetTarget(this);
}

void CPView::SetRunning(void)
{
	if (stopb) {
		stopb->SetEnabled(true);
		startb->SetEnabled(false);
	}
}

void CPView::SetStopped(void)
{
	if (stopb) {
		stopb->SetEnabled(false);
		startb->SetEnabled(true);
	}
}

void CPView::MessageReceived(BMessage *msg)
{
//	printf("BeDrift Control View::Message Received\n");
	switch (msg->what) {
		case DN_START:
		{
			BMessage smsg(DN_START);
			smsg.AddString("interface", device);
			be_app->PostMessage(&smsg);
			break;
		}
		case DN_STOP:
		{
			BMessage smsg(DN_STOP);
			smsg.AddString("interface", device);
			be_app->PostMessage(&smsg);
			break;
		}		
	}
}

