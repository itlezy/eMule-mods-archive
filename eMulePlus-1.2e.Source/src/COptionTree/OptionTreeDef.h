// COptionTree
//
// License
// -------
// This code is provided "as is" with no expressed or implied warranty.
// 
// You may use this code in a commercial product with or without acknowledgement.
// However you may not sell this code or any modification of this code, this includes 
// commercial libraries and anything else for profit.
//
// I would appreciate a notification of any bugs or bug fixes to help the control grow.
//
// History:
// --------
//	See License.txt for full history information.
//
//
// Copyright (c) 1999-2002 
// ComputerSmarts.net 
// mattrmiller@computersmarts.net

#ifndef OT_TREEDEF
#define OT_TREEDEF

// Option Definitions
// -- Settings
#define OT_INFOWINDOWHEIGHT					50						// Information window height
#define OT_DEFHEIGHT						21						// Default height of an item
#define OT_SPACE							5						// Default horizontal spacing
#define OT_EXPANDBOX						9						// Size of the expand box
#define OT_CHECKBOX							14						// Size of the check box
#define OT_EXPANDCOLUMN						16						// Width of the expand column
#define OT_PNINDENT							16						// Child level indent
#define OT_COLRNG							5						// Width of splitter
#define OT_EXPANDBOXHALF					(OT_EXPANDBOX / 2)		// Half of expand box
#define OT_DEFLABEL							_T("No Item Selected")	// Default label for no selected item in the info window
#define OT_DEFINFO							_T("Select an item to see its description.") // Default info for no selected item in the info window
#define OT_RESIZEBUFFER						25						// Resize limit for right and left of client so bar doesn't become invisible
#define OT_TREELIST_ID						30000					// ID of the list tree
#define OT_TIMER							1000					// The ID for a timer
// -- Item Settings
#define OT_ITEM_STATIC						1
#define OT_ITEM_CHECKBOX					2
#define OT_ITEM_COLOR						3
#define OT_ITEM_COMBOBOX					4
#define OT_ITEM_DATE						5
#define OT_ITEM_EDIT						6
#define OT_ITEM_IMAGE						7
#define OT_ITEM_RADIO						8
#define OT_ITEM_SPINNER						9
#define OT_ITEM_FONT						10
#define OT_ITEM_FILE						11
#define OT_ITEM_IPADDRESS					12
// -- Tree Items
#define OT_OPTIONS_SHOWINFOWINDOW			0x0001					// Show information window
#define OT_OPTIONS_NOTIFY					0x0002					// Send parent notifications		
#define OT_OPTIONS_DEFINFOTEXTNOSEL			0x0004					// Show default info text for no selected item, otherwise blank		
#define OT_OPTIONS_SHADEEXPANDCOLUMN		0x0008					// Shade the expand column
#define OT_OPTIONS_SHADEROOTITEMS			0x0010					// Shade the root items
// -- Edit Items
#define OT_EDIT_MLHEIGHT					75						// Multiline height
// -- Combo Box Items
#define OT_COMBO_DROPDOWNHEIGHT				100						// Drop down default height
// -- Check Box Items
#define OT_CHECKBOX_DEFCHECKTEXT			_T("Checked")			// Default checked text
#define OT_CHECKBOX_DEFUNCHECKTEXT			_T("UnChecked")			// Default un checked text
#define OT_CHECKBOX_SIZE					14.2					// Size of the check box
// -- Radio Items
#define OT_RADIO_VSPACE						2						// Vertical space between radios
#define OT_RADIO_SIZE						14.2					// Size of radio
// -- Spinner Items
#define OT_SPINNER_WIDTH					15						// Width of the spinner button
// -- Color Items
#define OT_COLOR_MORECOLORS					_T("More Colors...")	// Text for more colors
#define OT_COLOR_AUTOMATIC					_T("Automatic")			// Text for more automatic
#define OT_COLOR_SIZE						14.2					// The size for the color square
// -- Image Items
#define OT_IMAGE_MARGIN						10						// The margin for the popup window
#define OT_IMAGE_IMAGESPACE					10						// The space between images in the popup window
#define OT_IMAGE_NOSELECTION				_T("No selection made.")// Text for no selection selected
#define OT_IMAGE_MAXIMAGES					100						// The maximum number of images
// -- File Items
#define OT_FILE_NOSELECTION					_T("No selection made.")// Text for no selection selected

// NOTE: The following are highly important and should not need to be changed
// --------------------------------------------------------------------------


// Definitions
// -- Hit test
#define OT_HIT_LABEL						(WM_USER + 1000)		// Label
#define OT_HIT_COLUMN						(WM_USER + 1001)		// Column
#define OT_HIT_EXPAND						(WM_USER + 1002)		// Expand
#define OT_HIT_ATTRIBUTE					(WM_USER + 1003)		// Attribute
#define OT_HIT_CLIENT						(WM_USER + 1004)		// Client
// -- Notification to user
#define OT_NOTIFY_FIRST						(0U-1100U)
#define OT_NOTIFY_INSERTITEM				(OT_NOTIFY_FIRST - 1)	// Insert item
#define OT_NOTIFY_DELETEITEM				(OT_NOTIFY_FIRST - 2)	// Delete item
#define OT_NOTIFY_DELETEALLITEMS			(OT_NOTIFY_FIRST - 3)	// Delete all items
#define OT_NOTIFY_ITEMCHANGED				(OT_NOTIFY_FIRST - 5)	// Item changed
#define OT_NOTIFY_ITEMBUTTONCLICK			(OT_NOTIFY_FIRST - 6)	// Item button click
#define OT_NOTIFY_SELCHANGE					(OT_NOTIFY_FIRST - 7)	// Selection changed
#define OT_NOTIFY_ITEMEXPANDING				(OT_NOTIFY_FIRST - 8)	// Item expanding
#define OT_NOTIFY_COLUMNCLICK				(OT_NOTIFY_FIRST - 9)	// Column click
#define OT_NOTIFY_PROPCLICK					(OT_NOTIFY_FIRST - 10)	// Property click
// -- Notication to controls
#define OT_NOTIFY_COMMITCHANGES				WM_USER + 0x0102		// Loosing focus
#define OT_NOTIFY_FORCEREDRAW				WM_USER + 0x0103		// Force redraw
#define OT_NOTIFY_UP						WM_USER + 0x0104		// Up key pressed
#define OT_NOTIFY_DOWN						WM_USER + 0x0105		// Down key pressed
// -- Menu Definitions
#define OT_MES_UNDO							_T("&Undo")				// Text for undo
#define OT_MES_CUT							_T("Cu&t")				// Text for cut
#define OT_MES_COPY							_T("&Copy")				// Text for copy
#define OT_MES_PASTE						_T("&Paste")			// Text for paste
#define OT_MES_DELETE						_T("&Delete")			// Text for delete
#define OT_MES_SELECTALL					_T("Select &All")		// Text for select all
#define OT_MES_NSELECTALL					WM_USER + 0x7000		// Command for select all
// -- Color  Items
#define OT_COLOR_SELCHANGE					WM_USER + 1001			// Color picker selection change
#define OT_COLOR_DROPDOWN					WM_USER + 1002			// Color picker drop down
#define OT_COLOR_CLOSEUP					WM_USER + 1003			// Color picker close up
#define OT_COLOR_SELENDOK					WM_USER + 1004			// Color picker end OK
#define OT_COLOR_SELENDCANCEL				WM_USER + 1005			// Color picker end (cancelled)
#define OT_COLOR_DEFAULTBOXVALUE			-3						// Default box value
#define OT_COLOR_CUSTOMBOXVALUE				-2						// Custom box value
#define OT_COLOR_INVALIDCOLOR				-1						// Invalid color value
#define OT_COLOR_MAXCOLORS					100						// Maximum number of colors
// -- Image  Items
#define OT_IMAGE_CLOSE						WM_USER + 1001			// Image picker window close
// -- Font Selection Items
#define OT_FS_NOTIFY_APPLY					WM_USER + 1000			// Apply notification
// -- IP Address
#define OT_IPADDRESS_KILLFOCUS				WM_USER + 1002			// Edit is loosing focus
#define OT_IPADDRESS_NEXTEDIT				WM_USER + 1003			// Edit needs to change focus to next edit

// Options
// -- Edit Items
#define OT_EDIT_MULTILINE					0x00040000L				// Multiline edit
#define OT_EDIT_PASSWORD					0x00000400L				// Password edit
#define OT_EDIT_NUMERICAL					0x00000200L				// Numerical edit
// -- Check Box Items
#define OT_CHECKBOX_SHOWTEXT				0x00040000L				// Show check text	
#define OT_CHECKBOX_SHOWCHECK				0x00000400L				// Show check box
// -- Spinner Items
#define OT_EDIT_WRAPAROUND					0x00040000L				// Wrap around
#define OT_EDIT_USEREDIT					0x00000400L				// Allow user edit
// -- Color Items
#define OT_COLOR_SHOWHEX					0x00040000L				// Show hex instead of RGB
#define OT_COLOR_LIVEUPDATE					0x00000400L				// Allow  smaple to show updates
// -- Image Items
#define OT_IMAGE_SHOWTEXT					0x00040000L				// Show text in item sample
// -- Font Selection Items
#define OT_FS_TTONLY						0x00040000L				// True type fonts only
#define OT_FS_USEDEFAULT					0x00000400L				// Default button
#define OT_FS_USEAPPLY						0x00000200L				// Apply button
#define OT_FS_USECHARFORMAT					0x00000100L				// Char format
#define OT_FS_CUSTOMSAMPLE					0x00000800L				// Custom sample
#define OT_FS_FONTNAMESAMPLE				0x00001000L				// Use font name sample
#define OT_FS_NOTEXTCOLOR					0x00020000L				// No text color
#define OT_FS_NOEFFECTS						0x00008000L				// No effects
#define OT_FS_NOSTYLES						0x00800000L				// No styles
#define OT_FS_NOFACE						0x00080000L				// No face
// -- File Items
#define OT_FILE_SHOWFULLPATH				0x00040000L				// Show full paths in sample
#define OT_FILE_SHOWFILENAME				0x00000400L				// Show file name in sample
#define OT_FILE_SHOWFILETITLE				0x00000200L				// Show file title in sample
#define OT_FILE_SHOWFILEEXT					0x00000100L				// Show file extention in sample
#define OT_FILE_SHOWFILEDIR					0x00000800L				// Show file directory in sample
#define OT_FILE_SHOWFILEDRIVE				0x00001000L				// Show file drive in sample
#define OT_FILE_OPENDIALOG					0x00020000L				// Open file dialog
#define OT_FILE_SELECTDIALOG				0x00008000L				// Select folder
// -- Hyperlink
#define OT_HL_HOVER							0x00040000L				// Use hover color
#define OT_HL_VISITED						0x00000400L				// Use visited color
#define OT_HL_UNDERLINEHOVER				0x00000200L				// Underline when mouse is hover
#define OT_HL_UNDERLINE						0x00000100L				// Underline always

#endif // !OT_TREEDEF

