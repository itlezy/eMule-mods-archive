/*************************************************************************
	Pop-it menu - By Dynamic Drive
	For full source code and more DHTML scripts, visit
	http://www.dynamicdrive.com
	This credit MUST stay intact for use
*************************************************************************/
var ie4=document.all;
var ns6=document.getElementById&&!document.all;
var ns4=document.layers;

function showmenu(e,code,downstate)
{
	if (!document.all&&!document.getElementById&&!document.layers)
		return;

	if(downstate == "complete" || downstate == "completing")
		return;

	clearhidemenu();

	menuobj=ie4? document.all.popmenu : ns6? document.getElementById("popmenu") : ns4? document.popmenu : "";
	menuobj.thestyle=(ie4||ns6)? menuobj.style : menuobj;

	if (ie4||ns6)
		menuobj.innerHTML=code;
	else
	{
		menuobj.document.write('<layer name=gui bgColor=#e6e6e6 onmouseover="clearhidemenu()" onmouseout="hidemenu()">' + code + '<\/layer>');
		menuobj.document.close();
	}

	menuobj.contentwidth=(ie4||ns6)? menuobj.offsetWidth : menuobj.document.gui.document.width;
	menuobj.contentheight=(ie4||ns6)? menuobj.offsetHeight : menuobj.document.gui.document.height;
	eventX=ie4? event.clientX+5 : ns6? e.clientX+5 : e.x+5;
	eventY=ie4? event.clientY : ns6? e.clientY : e.y;

	var rightedge=ie4? document.body.clientWidth-eventX : window.innerWidth-eventX;
	var bottomedge=ie4? document.body.clientHeight-eventY : window.innerHeight-eventY;

	if (rightedge<menuobj.contentwidth)
		menuobj.thestyle.left=ie4? document.body.scrollLeft+eventX-menuobj.contentwidth : ns6? window.pageXOffset+eventX-menuobj.contentwidth : eventX-menuobj.contentwidth;
	else
		menuobj.thestyle.left=ie4? document.body.scrollLeft+eventX : ns6? window.pageXOffset+eventX : eventX;

	if (bottomedge>=menuobj.contentheight)
		menuobj.thestyle.top=eventY + (ie4? document.body.scrollTop : ns6? window.pageYOffset : 0);
	else if (eventY>menuobj.contentheight)
		menuobj.thestyle.top=eventY + (ie4? document.body.scrollTop-menuobj.contentheight : ns6? window.pageYOffset-menuobj.contentheight : -menuobj.contentheight);
	else
		menuobj.thestyle.top=ie4? document.body.scrollTop : ns6? window.pageYOffset : 0;

	menuobj.thestyle.visibility="visible";

	return false;
}

function contains_ns6(a, b)
{
	if (!b)
		return false;

	while (b.parentNode)
		if ((b = b.parentNode) == a)
			return true;

	return false;
}

function hidemenu()
{
	if (window.menuobj)
		menuobj.thestyle.visibility=(ie4||ns6)? "hidden" : "hide";
}

function dynamichide(e)
{
	if (ie4&&!menuobj.contains(e.toElement))
		hidemenu();
	else if (ns6&&e.currentTarget!= e.relatedTarget && !contains_ns6(e.currentTarget, e.relatedTarget))
		hidemenu();
}

function delayhidemenu()
{
	if (ie4||ns6||ns4)
		delayhide=setTimeout("hidemenu()",500);
}

function clearhidemenu()
{
	if (window.delayhide)
		clearTimeout(delayhide);
}

function highlightmenu(e,state)
{
	if (document.all)
		source_el=event.srcElement;
	else if (document.getElementById)
		source_el=e.target;

	if (source_el.className=="menuitems")
	{
		source_el.id=(state=="on")? "mouseoverstyle" : "";
	}
	else
	{
		while(source_el.id!="popmenu")
		{
			source_el=document.getElementById? source_el.parentNode : source_el.parentElement;

			if (source_el.className=="menuitems")
			{
				source_el.id=(state=="on")? "mouseoverstyle" : "";
			}
		}
	}
}

if (ie4||ns6)
	document.onclick=hidemenu;
