#include <Box.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#include "bd.h"
#include "cp.h"
#include "bedrift.h"

int main(int argc, char **argv)
{
	BeDrift app;

	app.Run();
	
	return (0);
}

BeDrift :: BeDrift() : BApplication("application/x-vnd.driftnet")
{
	cp = NULL;
	wins = NULL;
			
	cp = new CPWindow();
	cp->Show();
}

BeDrift::~BeDrift()
{
}

void BeDrift::AddWindow(BDWindow *win)
{
	Lock();
	if (wins) {
		BDWindow *w = wins;
		while (w->Next())
			w = w->Next();
		w->SetNext(win);
	} else
		wins = win;
	Unlock();
}

void BeDrift::RemoveWindow(BDWindow *win)
{
	bool unlock = false;
	if (!IsLocked()) {
printf("Locking!!\n");
		unlock = true;
		Lock();
	}
	if (wins == win)
		wins = win->Next();
	else {
		BDWindow *w = wins;
		while (w && w->Next() != win)
			w = w->Next();
		w->SetNext(win->Next());
	}
	cp->SetStopped(win->Device());
	if (unlock)
		Unlock();
}

BDWindow *BeDrift::FindWindow(char *title)
{
	BDWindow *w = wins;
	while (w) {
		if (strcmp(title, w->Device()) == 0)
			return w;
		w = w->Next();
	}
	return NULL;
}

void BeDrift::MessageReceived(BMessage *msg)
{
//	printf("BeDrift::Message Received - %08lx\n", msg->what);
	switch (msg->what) {
		case DN_START:
		{
			char *dev = NULL;
			msg->FindString("interface", (const char**)&dev);
			cp->DisableExit();
			bool ok = false;
			BDWindow *bd = FindWindow(dev);
			if (!bd) {
				bd = new BDWindow(dev, this);
				if (bd->IsValid()) {
					ok = true;
					AddWindow(bd);
					bd->Show();
					bd->SetCPView(cp);
				}
			} else
				ok = bd->StartThread();

			if (ok)
				cp->SetStarted(dev);
			
			cp->EnableExit();
			break;
		}
		case DN_STOP:
		{
			BDWindow *win = NULL;
			char *dev = NULL;
			msg->FindString("interface", (const char**)&dev);
			if ((win = FindWindow(dev)) != NULL) {
				win->StopThread();
				RemoveWindow(win);
				win->Lock();
				win->Quit();
			}
			break;
		}
	}
}
