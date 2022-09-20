#include "stdafx.h"
#include "emule.h"
#include "OtherFunctions.h"
#include "SafeFile.h"
#include "Preferences.h"
#include "emuleDlg.h"
#include "DAMessageBox.h"

#include "AdunanzA.h"




void AduTipBlock(uint32 adutip) {
	thePrefs.m_AduTips |= adutip;
}

bool AduTipShow(uint32 adutip) {
	if (adutip == ADUTIP_FAKE)
		return false;

	return !(thePrefs.m_AduTips & adutip);
}

