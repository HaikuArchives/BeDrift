#include <Box.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>

#include "bd.h"
#include "cp.h"
#include "bedrift.h"

BDWindow :: BDWindow(char *title, BeDrift *top) 
			: BWindow(BRect(50,50,450,450), title, B_TITLED_WINDOW, 0 )
{
	v = new BDView(this);
	parent = top;
	next = NULL;
	
	if (v->IsValid() == false)
		printf("Failed to create a BDView\n");
	else {
		AddChild(v);
		interface = new BDCapture(v);
		if (interface->SetInterface(title, true) == false) {
			printf("Failed to open %s for promiscuous\n", title);
			if (interface->SetInterface(title, true) != true)
				return;
		}	
	}
	device = strdup(title);
}

BDWindow::~BDWindow() 
{
	StopThread();
	BMessage smsg(DN_STOP);
	smsg.AddString("interface", device);
	be_app->PostMessage(&smsg);
	if (v)
		RemoveChild(v);
}
