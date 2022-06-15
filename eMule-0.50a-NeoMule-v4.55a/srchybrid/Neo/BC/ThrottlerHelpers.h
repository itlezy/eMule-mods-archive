//this file is part of NeoMule
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


/**
* This function calculated the datarate per given time span, 
* and add's it to the totransfer value.
*
* @param allowedDataRate wanted datarate in kb/s
*
* @param toTransfer positiv or *negativ* amount ot bytes to transfer during this loop
*
* @param timeSinceLastLoop time pased since the last loop
*
*/
__inline void CalcLoopAddOn(float allowedDataRate, int &toTransfer, DWORD timeSinceLastLoop)
{
	if (allowedDataRate == UNLIMITED)
		toTransfer = 0x7fffffff; //max signed value;
	else 
	{
		int currLoopAddOn = (int)(allowedDataRate * 1.024f * timeSinceLastLoop);

		if (toTransfer > 0 && toTransfer + currLoopAddOn < 0)
			toTransfer = 0;
		toTransfer += currLoopAddOn;

		if (toTransfer > (int)(allowedDataRate * 1.024f * 1000)) // compensate up to 1 sec
			toTransfer = (int)(allowedDataRate * 1.024f * 1000);
		//else if (toTransfer < -(int)(max((allowedDataRate * 1.024f * 1000),(theApp.bandwidthControl->GetMSS()*2)))) // fail safe
		//	toTransfer = 0;
	}
}

/**
* This function suspend the loop for the needed amount of time,
* sleep for just as long as we need to get back 
* to having minFragSize/2 bytes to send,
* when the toTransfer is negativ the sleep time will be longer
* long enough to reach again a positiv value
*
* @param toSleep setting how long whould take a egular sleep
*
* @param allowedDataRate wanted datarate in kb/s
*
* @param toTransfer positiv or *negativ* amount ot bytes to transfer
*
* @param timeSinceLastLoop time pased since the last loop
*
* @param minFragSize minimal fragment size
*
*/
__inline void DelayLoopTime(uint32 toSleep, float allowedDataRate, int toTransfer, DWORD timeSinceLastLoop, uint32 minFragSize)
{
	uint32 sleepTime = (uint32)((float)(minFragSize/2)/allowedDataRate);

	if(toTransfer<0)
		sleepTime=max((uint32)((float)(-toTransfer+1)/(allowedDataRate)),sleepTime);

	if(sleepTime < toSleep)
		sleepTime = toSleep;
	else if (sleepTime > toSleep*5)
		sleepTime = toSleep;

	if(timeSinceLastLoop < sleepTime)
		Sleep(sleepTime - timeSinceLastLoop);
}