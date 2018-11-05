#include "bedrift.h"
#include <Box.h>
#include <stdio.h>
#include <sys/ioctl.h>

DriftWindow::DriftWindow(BRect rect) 
				: BWindow(rect, "BeDrift", B_TITLED_WINDOW, 0 )
{
	dv = new DriftView(BRect(Bounds()));
	AddChild(dv);
}

DriftWindow::~DriftWindow() 
{
	if (dv) {
		RemoveChild(dv);
		delete dv;
	}
	dv = NULL;
}

void DriftWindow::AddBitmap(BBitmap *b) 
{
	if (dv && b) {
		Lock();
		dv->AddBitmap(b);
		Unlock();
	}
	if (b)
		delete b;
	qty++;
}
