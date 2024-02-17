#include "uipriv_windows.hpp"
#include "treeviewitem.hpp"

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

static void windowsGetTreeItem(HWND hTree, HTREEITEM hItem, uiTreeViewItem* item)
{
	TVITEM tvi;
	ZeroMemory(&tvi, sizeof(TVITEM));
	tvi.mask = TVIF_TEXT | TVIF_PARAM;
		
	// TODO: Retrieve the item text and user data from the HTREEITEM
	// and fill in the uiTreeViewItem.
}

static HTREEITEM windowsAddTreeItem(HWND hTree, HTREEITEM hParent, HTREEITEM hPrev, uiTreeViewItem* item)
{
	TVITEM tvi;
    TVINSERTSTRUCT tvins;
	HTREEITEM newItem;
	WCHAR *wtext;

	wtext = toUTF16(uiTreeViewItemText(item));

	ZeroMemory(&tvi, sizeof(TVITEM));
	tvi.mask = TVIF_TEXT | TVIF_PARAM;
	tvi.pszText = wtext;
	tvi.cchTextMax = wcslen(wtext);
	tvi.lParam = (LPARAM)uiTreeViewItemUserData(item);

	ZeroMemory(&tvins, sizeof(TVINSERTSTRUCT));
	tvins.hParent = hParent;
	tvins.item = tvi;
	tvins.hInsertAfter = hPrev;

	newItem = TreeView_InsertItem(hTree, &tvins);
	uiprivFree(wtext);
	return newItem;
}

static void addChildrenToTree(uiTreeViewModel *m, const uiTreeViewItem *parent, HTREEITEM hParent, HTREEITEM hPrev)
{
	HTREEITEM hNext = hPrev;

	int n = m->data->NumChildren(m->data, m, parent);
	for(int i = 0; i < n; ++i)
	{
		uiTreeViewItem* child = uiNewTreeViewItem();
		if(m->data->GetChild(m->data, m, i, parent, child) != 0)
		{
			hNext = windowsAddTreeItem(m->tree->hwnd, hParent, hNext, child);
			addChildrenToTree(m, child, hNext, TVI_FIRST);
		}
		uiFreeTreeViewItem(child);
	}
}

void uiTreeViewModelDataChanged(uiTreeViewModel *m, const uiTreeViewItemList* updateList)
{
	if(!m->tree || !m->data)
	{
		return;
	}

	// TODO: implement partial updates with `updateList`.
	TreeView_DeleteAllItems(m->tree->hwnd);
	addChildrenToTree(m, NULL, TVI_ROOT, TVI_LAST);
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
	uiWindowsEnsureDestroyWindow(t->hwnd);

	// detach table from model
	t->model->tree = NULL;


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

uiTreeViewItemList uiTreeViewGetSelection(uiTreeView * t)
{
	HTREEITEM root;
	HTREEITEM item;
	int i;
	uiTreeViewItemList list;
	list.NumItems = TreeView_GetSelectedCount(t->hwnd);
	list.Items = NULL;

	if(list.NumItems == 0)
	{
		return list;
	}

	list.Items = (uiTreeViewItem *)uiprivAlloc(list.NumItems * sizeof(uiTreeViewItem *), "uiTreeViewItem[] (uiTreeView)");
	
	i = 0;
	root = TreeView_GetRoot(t->hwnd);
	item = TreeView_GetNextSelected(t->hwnd, root);
	while(item != NULL && i < list.NumItems)
	{
		item = TreeView_GetNextSelected(t->hwnd, item);

		// TODO: Use windowsGetTreeItem() to populate in list.Items[i].
	}
	
	return list;
}

uiTreeView *uiNewTreeView(uiTreeViewModel* m)
{
	uiTreeView *t;

	uiWindowsNewControl(uiTreeView, t);

	t->model = m;
	t->hwnd = uiWindowsEnsureCreateControlHWND(TVS_EX_DOUBLEBUFFER,
		WC_TREEVIEW, L"",
		TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT,
		hInstance, NULL,
		TRUE);
	t->model->tree = t;
	uiTreeViewModelDataChanged(t->model, NULL);
	//uiWindowsRegisterWM_COMMANDHandler(b->hwnd, onWM_COMMAND, uiControl(b));

	return t;
}