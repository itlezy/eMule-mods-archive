/** @file		P2PThreat.h
 *  @brief	Detect bad apps (virus/worms) that could threaten file sharing
 *  @author	netfinity
 */
#pragma once

/**
 *  @brief	Detects bad apps (virus/worms) that could threaten file sharing
 */
class CP2PThreat
{
public:
	bool IsMachineInfected();
private:
	bool TestRegistryKey(HKEY hParent, LPCTSTR pszKey, LPCTSTR pszName, LPCTSTR pszValue);
};

extern CP2PThreat	theP2PThreat;