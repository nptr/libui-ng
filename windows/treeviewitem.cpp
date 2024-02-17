#include "uipriv_windows.hpp"
#include "treeviewitem.hpp"

uiTreeViewItem *uiNewTreeViewItem()
{
    uiTreeViewItem * item = uiprivNew(uiTreeViewItem);
    item->Text = NULL;
    item->UserData = NULL;
    return item;
}

void uiFreeTreeViewItem(uiTreeViewItem *item)
{
    if(item->Text)
        uiprivFree(item->Text);

	uiprivFree(item);
}

void uiTreeViewItemSetText(uiTreeViewItem *item, const char* text)
{
    if(item->Text)
        uiprivFree(item->Text);

    item->Text = (char *) uiprivAlloc((strlen(text) + 1) * sizeof (char), "char[] (uiTreeViewItem)");
	strcpy(item->Text, text);
}

const char* uiTreeViewItemText(const uiTreeViewItem *item)
{
    return item->Text;
}

void uiTreeViewItemSetUserData(uiTreeViewItem *item, const void* userdata)
{
    item->UserData = userdata;
}

const void* uiTreeViewItemUserData(const uiTreeViewItem *item)
{
    return item->UserData;
}