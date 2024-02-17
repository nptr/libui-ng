#ifndef __LIBUI_TREEVIEWITEM_HPP__
#define __LIBUI_TREEVIEWITEM_HPP__

typedef struct uiTreeViewItem {
	char* Text;		    	//!< The items text.
	const void* UserData;	//!< User data field for the data source to identify the underlying item.
} uiTreeViewItem;

#endif

