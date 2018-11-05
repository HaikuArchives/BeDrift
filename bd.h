/* BeDrift classes */

#ifndef BD_H
#define BD_H

#include <Application.h>
#include <Window.h>
#include <View.h>
#include <Button.h>
#include <Message.h>
#include <Screen.h>
#include <MenuItem.h>
#include <MenuField.h>
#include <StringView.h>
#include <Bitmap.h>

#include "bdcapture.h"

class CPWindow;
class CPView;
class BDWindow;
class BeDrift;

class BDBitmap 
{
public:
	BDBitmap(BBitmap *, int, int, BDView *);
	~BDBitmap();
	void DrawBitmap(void);
	bool Intersects(BRect);
	bool Intersects(BPoint);
	void Adjust(int, int);
	int Width() { return b->Bounds().IntegerWidth(); }
	int Height() { return b->Bounds().IntegerHeight(); }
	void MoveTo(int nx, int ny) { x = nx; y = ny; }
	BDBitmap *Next() { return next; };
	void SetNext(BDBitmap *bn) { next = bn; };
	void Save();
private:
	BDBitmap *next;
	BBitmap *b;
	int x;
	int y;
	BDView *view;
};

class BDView : public BView
{
public:
	BDView(BDWindow *);
	void Draw(BRect);
	bool IsValid() { return true; };
	void AddBitmap(BBitmap *);
	void RemoveBitmap(BDBitmap *);
	void Adjust(int, int);
	void FrameResized(float, float);
	void MouseDown(BPoint);
private:
	BDWindow *parent;
	BDBitmap *pics;
	int qty;
	int viewx, viewy;
	int maxy;
};

class BDWindow : public BWindow
{
public:
	BDWindow(char *, BeDrift *);
	~BDWindow();
	bool IsValid() { return true; };
	BDWindow *Next() { return next; }
	void SetNext(BDWindow *w) { next = w; }
	BDView *View() { return v; }
	char *Device() { return device; }
	bool StartThread() { return interface->StartThread(); }
	bool StopThread() { return interface->StopThread(); }
	void SetCPView(CPWindow *c) { cpw = c; }
private:
	BDWindow *next;
	BDView *v;
	BDCapture *interface;
	BeDrift *parent;
	char *device;
	CPWindow *cpw;
};

class BeDrift : public BApplication
{
public:
	BeDrift();
	~BeDrift();
	void MessageReceived(BMessage *);
	void AddWindow(BDWindow *);
	void RemoveWindow(BDWindow*);
	BDWindow *FindWindow(char *);
private:
	CPWindow *cp;
	BDWindow *wins;
};

#endif /* BEDRIFT_H */
