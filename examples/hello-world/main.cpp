#include <stdio.h>
#include <ui.h>

#include <vector>

int onClosing(uiWindow *w, void *data)
{
	uiQuit();
	return 1;
}

typedef struct myItemStruct {
	const char* text;
	std::vector<myItemStruct> children;
} myItemStruct;

std::vector<myItemStruct> myTreeData = {
	{ "One", {
		{ "One.1", {} },
		{ "One.2", {} },
	} },
	{ "Two", {
		{ "Two.1", { 
			{ "Two.1.1", {} }
		}
	} } },
};

int mySourceNumChildren(uiTreeViewDataSource *, uiTreeViewModel *, uiTreeViewItem* parent)
{
	if(parent == NULL)
	{
		return myTreeData.size();
	}
	else
	{
		myItemStruct* data = (myItemStruct*)parent->userData;
		return data->children.size();
	}
}

int mySourceGetChild(uiTreeViewDataSource *, uiTreeViewModel *, int index, const uiTreeViewItem* parent, uiTreeViewItem* item)
{
	if(parent == NULL)
	{
		if(index >= myTreeData.size())
			return 0;

		item->text = myTreeData[index].text;
		item->userData = (void*)&myTreeData[index];
	}
	else
	{
		myItemStruct* data = (myItemStruct*)parent->userData;
		if(index >= data->children.size())
			return 0;

		item->text = data->children[index].text;
		item->userData = (void*)&(data->children[index]);
	}

	return 1;
}

int main(void)
{
	uiInitOptions o = {0};
	const char *err;
	uiWindow *w;
	uiTreeView *l;
	uiTreeViewModel *model;

	err = uiInit(&o);
	if (err != NULL) {
		fprintf(stderr, "Error initializing libui-ng: %s\n", err);
		uiFreeInitError(err);
		return 1;
	}

	// Create a new window
	w = uiNewWindow("Hello World!", 300, 30, 0);
	uiWindowOnClosing(w, onClosing, NULL);

	uiTreeViewDataSource src;
	src.GetChild = mySourceGetChild;
	src.NumChildren = mySourceNumChildren;

	model = uiNewTreeViewModel(&src);
	l = uiNewTreeView(model);
	uiWindowSetChild(w, uiControl(l));

	uiControlShow(uiControl(w));
	uiMain();
	uiUninit();
	return 0;
}

