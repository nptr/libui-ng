#include "uipriv_windows.hpp"

struct uiTreeViewModel {
	uiTreeViewDataSource *data;
	uiTreeView* tree;
};

uiTreeViewModel *uiNewTreeViewModel(uiTreeViewDataSource *data)
{
	uiTreeViewModel *m;

	m = uiprivNew(uiTreeViewModel);
	m->data = data;
	m->tree = nullptr;
	return m;
}

void uiFreeTreeViewModel(uiTreeViewModel *m)
{
	uiprivFree(m);
}

struct uiTreeView {
	uiWindowsControl c;
	HWND hwnd;
	uiTreeViewModel* model;
};

static BOOL onWM_COMMAND(uiControl *c, HWND hwnd, WORD code, LRESULT *lResult)
{
	// uiTreeView *t = uiTreeView(c);
	return FALSE;
}

static void uiTreeViewDestroy(uiControl *c)
{
	uiTreeView *t = uiTreeView(c);

	//uiWindowsUnregisterWM_COMMANDHandler(t->hwnd);
	// uiWindowsEnsureDestroyWindow(t->hwnd);
	uiFreeControl(uiControl(t));
}

uiWindowsControlAllDefaultsExceptDestroy(uiTreeView)

#define treeViewHeight 200
#define treeViewWidth 200

static void uiTreeViewMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	uiTreeView *t = uiTreeView(c);
	SIZE size;
	uiWindowsSizing sizing;
	int x, y;

    x = treeViewWidth;
	y = treeViewHeight;
	uiWindowsGetSizing(t->hwnd, &sizing);
	uiWindowsSizingDlgUnitsToPixels(&sizing, NULL, &y);
	*height = y;
}

uiTreeView *uiNewTreeView(uiTreeViewModel* m)
{
	uiTreeView *t;

	uiWindowsNewControl(uiTreeView, t);

	t->model = m;
	t->hwnd = uiWindowsEnsureCreateControlHWND(0,
		WC_TREEVIEW, L"",
		TVS_HASLINES,
		hInstance, NULL,
		TRUE);

	//uiWindowsRegisterWM_COMMANDHandler(b->hwnd, onWM_COMMAND, uiControl(b));
	//uiButtonOnClicked(b, defaultOnClicked, NULL);

	return t;
}