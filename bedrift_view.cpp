#include "bedrift.h"
#include <Box.h>
#include <stdio.h>
#include <sys/ioctl.h>
extern "C" {
#include <sys/socket.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <stdlib.h>
#include <string.h>
}

DriftView::DriftView(BRect rect)
	   	   : BView(rect, "driftview", B_FOLLOW_ALL, B_WILL_DRAW)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetFont(be_bold_font);
	SetFontSize(24);
}


void DriftView::AddBitmap(BBitmap *b)
{
	printf("DriftView::AddBitmap(%p)\n", b);
	if (!b->IsValid())
		return;
	srand(real_time_clock());
	int xrand = rand() % (int)Bounds().Width();
	int yrand = rand() % (int)Bounds().Height();
	printf("xrand = %d, yrand = %d\n", xrand, yrand);
	printf("bitmap %f, %f\n", b->Bounds().Width(), b->Bounds().Height());
	printf("view %f, %f\n", Bounds().Width(), Bounds().Height());
	int maxx = (int)Bounds().Width() - (int)b->Bounds().Width();
	int maxy = (int)Bounds().Height() - (int)b->Bounds().Height();
	if (xrand > maxx)
		xrand = maxx;
	if (yrand > maxy)
		yrand = maxy;
printf("xrand = %d, yrand = %d, maxx = %d, maxy = %d\n", 
       xrand, yrand, maxx, maxy);
	BPoint drawat(xrand, yrand);
	DrawBitmap(b, drawat);
}
