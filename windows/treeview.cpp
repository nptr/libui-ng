#include "uipriv_windows.hpp"

struct uiTreeViewModel {
	uiTreeViewDataSource *data;
	uiTreeView* tree;
};

struct uiTreeView {
	uiWindowsControl c;
	HWND hwnd;
	uiTreeViewModel* model;
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

HTREEITEM WindowsAddTreeItem(HWND hTree, HTREEITEM hParent, HTREEITEM hPrev, uiTreeViewItem* item)
{
	TVITEM tvi;
    TVINSERTSTRUCT tvins;
	HTREEITEM newItem;
	WCHAR *wtext;

	wtext = toUTF16(item->text);

	ZeroMemory(&tvi, sizeof(TVITEM));
	tvi.mask = TVIF_TEXT | TVIF_PARAM;
	tvi.pszText = wtext;
	tvi.cchTextMax = wcslen(wtext);
	tvi.lParam = (LPARAM)item->userData;

	ZeroMemory(&tvins, sizeof(TVINSERTSTRUCT));
	tvins.hParent = hParent;
	tvins.item = tvi;
	tvins.hInsertAfter = hPrev;

	newItem = TreeView_InsertItem(hTree, &tvins);
	uiprivFree(wtext);
	return newItem;
}

void AddChildrenToTree(uiTreeViewModel *m, uiTreeViewItem* parent, HTREEITEM hParent, HTREEITEM hPrev)
{
	HTREEITEM hNext = hPrev;

	int n = m->data->NumChildren(m->data, m, parent);
	for(int i = 0; i < n; ++i)
	{
		uiTreeViewItem child;
		if(m->data->GetChild(m->data, m, i, parent, &child) != 0)
		{
			hNext = WindowsAddTreeItem(m->tree->hwnd, hParent, hNext, &child);
			AddChildrenToTree(m, &child, hNext, TVI_FIRST);
		}
	}
}

void uiTreeViewModelDataChanged(uiTreeViewModel *m)
{
	if(!m->tree || !m->data)
	{
		return;
	}

	AddChildrenToTree(m, NULL, TVI_ROOT, TVI_LAST);
}


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
		TVS_HASBUTTONS | TVS_HASLINES,
		hInstance, NULL,
		TRUE);
	t->model->tree = t;
	uiTreeViewModelDataChanged(t->model);
	//uiWindowsRegisterWM_COMMANDHandler(b->hwnd, onWM_COMMAND, uiControl(b));
	//uiButtonOnClicked(b, defaultOnClicked, NULL);

	return t;
}