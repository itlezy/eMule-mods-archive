// MeterIcon.h															12.12.03
//------------------------------------------------------------------------------
#ifndef __METERICON_H__B617C08B_2D12_4140_B845_0AB44DA72EFF__INCLUDED_
#define __METERICON_H__B617C08B_2D12_4140_B845_0AB44DA72EFF__INCLUDED_

HICON CreateMeterIcon(UINT nBaseIcon, 
 					  int cx, int cy,
					  int iMaxVal, int iCurVal, 
					  COLORREF crMeter, COLORREF crBorder);

#endif	// #ifndef __METERICON_H__B617C08B_2D12_4140_B845_0AB44DA72EFF__INCLUDED_
