// ########## Image Preload ##########
function preload_images(pics)
{
	var images = new Array;

	if (document.images)
	{
		for (var pic_num=0; pic_num<pics.length; pic_num++)
		{
			images[pic_num] = new Image();
			images[pic_num].src = pics[pic_num];
		}
	}

	return (images);
}
// ########################################


// ########## ShowHide ##########
function ShowHide(id)
{
	var itm = null;

	if (document.getElementById)
		itm = document.getElementById(id);
	else if (document.all)
		itm = document.all[id];
	else if (document.layers)
		itm = document.layers[id];

	if (!itm)
	{
		// do nothing
	}
	else if (itm.style)
	{
		if (itm.style.display == "none")
			itm.style.display = "";
		else
			itm.style.display = "none";
	}
	else
	{
		itm.visibility = "show";
	}
}
// ########################################


// ########## MenuLock ##########
function menulockmain()
{
	if (menulockedstate)
	{
		image_swap("menuicon","/menudown.gif");
		menulock_pos = '0';
		menulock();
	}
}

function image_swap(id,imgsrc)
{
	document.getElementById(id).setAttribute("src",imgsrc);
}

function menulock()
{
	var bAppName = window.navigator.appName;
	var isIE = (bAppName.indexOf("Explorer") >= 0);

	if (isIE)
		setInterval ('menulock_ie()',5);
	else
		menulock_css();
}

function menulock_css()
{
	var pos = menulock_pos;
	var menu_height = document.getElementById('menu').scrollHeight;

	document.getElementById('menu').style.float = "top";
	document.getElementById('menu').style.width = "100%";
	document.getElementById('menu').style.position = "fixed";
	document.getElementById('menu').style.top = pos + "px";
	if (menu_height)
		document.body.style.marginTop = menu_height + "px";
	else
		document.body.style.marginTop = "145px";
}

function menulock_ie()
{
	var pos = menulock_pos;
	var sc = document.body.scrollTop;
	pos = Math.round((sc + pos)/10);
	document.getElementById('menu').style.top = pos + "px";
}
// ########################################


// ########## Copy text to clipboard (IE/Mozilla) ##########
function ToClipboard(txt)
{
	if(window.clipboardData)
	{
		window.clipboardData.clearData();
		window.clipboardData.setData("Text", txt);
	}
	else if(navigator.userAgent.indexOf("Opera")!=-1)
	{
		window.location=txt;
	}
	else if (window.netscape)
	{
		try
		{
			netscape.security.PrivilegeManager.enablePrivilege("UniversalXPConnect");
		}
		catch (e)
		{
			alert("Permission denied! Browse to 'about:config' in your browser\nand set 'signed.applets.codebase_principal_support' to true");
		}

		var clip = Components.classes['@mozilla.org/widget/clipboard;1'].createInstance(Components.interfaces.nsIClipboard);

		if (!clip)
			return;

		var trans = Components.classes['@mozilla.org/widget/transferable;1'].createInstance(Components.interfaces.nsITransferable);

		if (!trans)
			return;

		trans.addDataFlavor('text/unicode');
		var str = new Object();
		var len = new Object();
		var str = Components.classes["@mozilla.org/supports-string;1"].createInstance(Components.interfaces.nsISupportsString);
		var copytext=txt;
		str.data=copytext;
		trans.setTransferData("text/unicode",str,copytext.length*2);
		var clipid=Components.interfaces.nsIClipboard;

		if (!clip)
			return false;

		clip.setData(trans,null,clipid.kGlobalClipboard);
	}
}
// ########################################


// ########## Warn when inputing login information in caps ##########
var warnedalready = false;

function capsDetect(e)
{
	if(warnedalready) { return; }
	if(!e) { e = window.event; }
	if(!e) { return; }
	var theKey = e.which ? e.which : (e.keyCode ? e.keyCode : (e.charCode ? e.charCode : 0));
	var theShift = e.shiftKey || (e.modifiers && (e.modifiers & 4));
	if ((theKey > 64 && theKey < 91 && !theShift) || (theKey > 96 && theKey < 123 && theShift)) { alert(capsError); warnedalready = true; }
}
// ########################################
