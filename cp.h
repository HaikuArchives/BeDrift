/* Control Panel Window and View classes */
#ifndef CP_H
#define CP_H

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

class CPWindow;

/*
class CPOptions : public BView
{
	CPOptions();
	~CPOptions();
private:
	bool testmode;
};
*/

class CPView : public BView
{
public:
	CPView(CPWindow *, char *);
//	~CPView();
	void Start(void);
	void Stop(void);
	void SetNext(CPView *newnext) { next = newnext; }
	CPView *Next() { return next; }
	char *Device() { return device; }
	void SetRunning(void);
	void SetStopped(void);
private:
	void AttachedToWindow();
	void CPView::Draw(BRect);
	void MessageReceived(BMessage *);
	CPWindow *parent;
	CPView *next;
	char *device;
	BStringView *devstr;
	BButton *startb;
	BButton *stopb;
};


class CPWindow : public BWindow
{
public:
	CPWindow();
	~CPWindow();
	void AddChildSize(BRect);
	virtual	bool QuitRequested();
	void DisableExit(void);
	void EnableExit(void);
	void SetStarted(char *);
	void SetStopped(char *);	
private:
	void BuildIFList(void);
	void MessageReceived(BMessage *);

	BView *bg;
	CPView *views;	
	BButton *quitb;
	int viewsx;
	int viewsy;
};

#endif /* CP_H */
