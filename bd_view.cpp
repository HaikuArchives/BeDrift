#include <Box.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <BitmapStream.h>

#include "bd.h"
#include "cp.h"
#include "bedrift.h"

BDBitmap::BDBitmap(BBitmap *bb, int xx, int yy, BDView *v)
{
	b = bb;
	x = xx;
	y = yy;
	view = v;
	next = NULL;
}

BDBitmap::~BDBitmap()
{
}

void BDBitmap::DrawBitmap()
{
	BPoint p(x, y);
	view->DrawBitmap(b, p);
}

bool BDBitmap::Intersects(BRect where)
{
	if (x + b->Bounds().IntegerWidth() - where.left > 0 &&
	    y + b->Bounds().IntegerHeight() - where.top > 0)
		return true;
	return false;
}

bool BDBitmap::Intersects(BPoint where)
{
	if (x <= (int)where.x && x + b->Bounds().IntegerWidth() >= (int)where.x &&
	    y <= (int)where.y && y + b->Bounds().IntegerHeight() >= (int)where.y)
		return true;
	return false;
}

void BDBitmap::Adjust(int dx, int dy)
{
	x -= dx;
	y -= dy;
	if (y < 0 && (abs(y) > b->Bounds().IntegerHeight()))
		view->RemoveBitmap(this);
}

void BDBitmap::Save(void)
{
/*
	int fd = open("/boot/home/saved.jpg", O_WRONLY | O_CREAT);
	if (fd < 0)
		return;
	BBitmapStream *bs = new BBitmapStream(b);
	char *buff = (char*)malloc(bs->Size());
	bs->ReadAt(0, buff, bs->Size());
	write(fd, buff, bs->Size());
	close(fd);
	free(buff);
	delete bs;
*/
}

BDView :: BDView(BDWindow *win) 
			: BView(win->Bounds(), "view", B_FOLLOW_ALL_SIDES, 
			        B_FRAME_EVENTS | B_WILL_DRAW)
{
	parent = win;
	viewx = viewy = 0;
	pics = NULL;
	qty = 0;
}

void BDView::FrameResized(float newx, float newy)
{
	parent->Lock();
	BDBitmap *b = pics;
	int curx = 0, cury = 0;
	int maxx = (int)newx;
	int maxyy = (int)newy;
	int maxpy = 0;
	
	while (b) {
		int px = b->Width();
		int py = b->Height();
		if (curx + px > maxx) {
			curx = 0;
			cury += maxpy;
			maxpy = py;
		}
		if (py > maxpy)
			maxpy = py;
		b->MoveTo(curx, cury);
		curx += px;
		b = b->Next();
	}
	if (cury + maxpy > maxyy) {
		int dy = (cury + maxpy) - maxyy;
		Adjust(0, dy);
		cury -= dy;
	}
	Invalidate(Bounds());		

	parent->Unlock();

	viewx = curx;
	viewy = cury;
	maxy = maxpy;
}

/* Adjust() for the bitmap may cause it to be deleted, so
 * keep next pointer available.
 */
void BDView::Adjust(int dx, int dy)
{
	BDBitmap *p = pics, *pn = NULL;
	while (p) {
		pn = p->Next();
		p->Adjust(dx, dy);
		p = pn;
	}
	viewx -= dx;
	viewy -= dy;
}

void BDView::AddBitmap(BBitmap *bm)
{
	int x, y;
	
	x = (int)bm->Bounds().Width();
	y = (int)bm->Bounds().Height();

	parent->Lock();

	if (viewx + x > Bounds().Width()) {
		viewy += maxy;
		maxy = y;
		if (viewy + maxy > Bounds().IntegerHeight()) {
			Adjust(0, y);
			Invalidate(Bounds());
		}
		viewx = 0;
	} else {
		if (y > maxy) {
			if (viewy + y > Bounds().IntegerHeight()) {
				Adjust(0, y - maxy);
				Invalidate(Bounds());
			}
			maxy = y;
		}
	}

	BDBitmap *nb = new BDBitmap(bm, viewx, viewy, this);

	if (pics) {
		BDBitmap *pn = pics;
		while (pn->Next()) 
			pn = pn->Next();
		pn->SetNext(nb);
	} else
		pics = nb;

	nb->DrawBitmap();
	parent->Unlock();
	viewx += x;
}

void BDView::RemoveBitmap(BDBitmap *b)
{
	if (pics == b)
		pics = b->Next();
	else {
		BDBitmap *p = pics;
		while (p && p->Next() != b)
			p = p->Next();
		if (p)
			p->SetNext(b->Next());
	}
	delete b;
}

void BDView::MouseDown(BPoint where)
{
	BMessage *msg = parent->CurrentMessage();
	uint32 clicks = msg->FindInt32("clicks");
	if (clicks > 2) {
		BDBitmap *b = pics;
		while (b) {
			if (b->Intersects(where))
				b->Save();
			b = b->Next();
		}
	}
}

void BDView::Draw(BRect where)
{
	if (pics) {
		BDBitmap *pb = pics;
		while (pb) {
			if (pb->Intersects(where))
				pb->DrawBitmap();
			pb = pb->Next();
		}
	}
	BView::Draw(where);
}
