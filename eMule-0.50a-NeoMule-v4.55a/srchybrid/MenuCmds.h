#pragma once

///////////////////////////////////////////////////////////////////////////////
// Menu commands for GUI only

// Menu entries for the application system menu -> require a set of IDs with special restrictions!
#define MP_RESTORE				0x110
#define MP_CONNECT				0x120
#define MP_DISCONNECT			0x130
#define MP_EXIT					0x140
#define	MP_ABOUTBOX				0x150
#define MP_VERSIONCHECK			0x160
#define MP_MINIMIZETOTRAY		0x170
#define MP_NEOVERSIONCHECK		0x180 // NEO: NVC - [NeoVersionCheck] <-- Xanatos --

#define MP_MESSAGE				10102
#define MP_DETAIL				10103
#define MP_ADDFRIEND			10104
#define MP_REMOVEFRIEND			10105
#define MP_SHOWLIST				10106
#define MP_FRIENDSLOT			10107

#define MP_CANCEL				10201
#define MP_STOP					10202
#define MP_PAUSE				10203
#define MP_RESUME				10204
#define	MP_CLEARCOMPLETED		10205
#define	MP_OPEN					10206
#define	MP_PREVIEW				10207
#define MP_CMT					10208
#define MP_HM_CON				10209
#define MP_HM_SRVR				10210
#define MP_HM_TRANSFER			10211
#define MP_HM_SEARCH			10212
#define MP_HM_FILES				10213
#define MP_HM_MSGS				10214
#define MP_HM_IRC				10215
#define MP_HM_STATS				10216
#define MP_HM_PREFS				10217
#define MP_HM_OPENINC			10218
#define MP_HM_EXIT				10219
#define MP_TRY_TO_GET_PREVIEW_PARTS 10220
#define MP_ADDSOURCE			10221
#define MP_ALL_A4AF_AUTO		10222
#define MP_META_DATA			10225
#define MP_BOOT					10226
#define MP_HM_CONVERTPF			10227
#define MP_RESUMEPAUSED			10228
#define MP_HM_KAD				10229
#define MP_HM_LINK1				10230
#define MP_HM_LINK2				10231
#define MP_HM_LINK3				10232
#define MP_HM_SCHEDONOFF		10233
#define MP_SELECTTOOLBARBITMAPDIR 10234
#define MP_SELECTTOOLBARBITMAP	10235
#define MP_NOTEXTLABELS			10236
#define MP_TEXTLABELS			10237
#define MP_TEXTLABELSONRIGHT	10238
#define	MP_CUSTOMIZETOOLBAR		10239
#define	MP_SELECT_SKIN_FILE		10240
#define	MP_SELECT_SKIN_DIR		10241
#define MP_HM_HELP				10242
#define MP_HM_1STSWIZARD		10243
#define MP_OPENFOLDER			10244
#define	MP_HM_IPFILTER			10245
#define	MP_WEBSVC_EDIT			10246
#define	MP_HM_DIRECT_DOWNLOAD	10247
#define	MP_INSTALL_SKIN			10248
#define	MP_LARGEICONS			10249
#define	MP_SMALLICONS			10250
#define	MP_VIEW2_CLIENTS		10251
#define	MP_VIEW2_DOWNLOADING	10252
#define	MP_VIEW2_UPLOADING		10253
#define	MP_VIEW2_ONQUEUE		10254
#define MP_VIEW1_SPLIT_WINDOW	10255
#define MP_VIEW1_UPLOADING		10256
#define MP_VIEW1_DOWNLOADS		10257
#define MP_VIEW1_ONQUEUE		10258
#define MP_VIEW1_DOWNLOADING	10259
#define MP_VIEW1_CLIENTS		10260

#define MP_PRIOVERYLOW			10300
#define MP_PRIOLOW				10301
#define MP_PRIONORMAL			10302
#define MP_PRIOHIGH				10303
#define MP_PRIOVERYHIGH			10304
#define MP_GETED2KLINK			10305
#define MP_GETHTMLED2KLINK		10306
#define MP_METINFO				10307
// NEO: SSP - [ShowSharePermissions] -- Xanatos --
//#define MP_PERMALL				10308
//#define MP_PERMFRIENDS			10309
//#define MP_PERMNONE				10310
#define MP_CONNECTTO			10311
#define MP_REMOVE				10312
#define MP_REMOVEALL			10313
#define MP_REMOVESELECTED		10314
#define MP_UNBAN				10315
#define MP_ADDTOSTATIC			10316
#define MP_CLCOMMAND			10317
#define MP_PRIOAUTO				10317
#define MP_REMOVEFROMSTATIC		10318
#define MP_VIEWFILECOMMENTS		10319
#define MP_CAT_ADD				10321
#define MP_CAT_EDIT				10322
#define MP_CAT_REMOVE			10323
#define MP_SAVELOG				10324
#define MPG_DELETE				10325
#define	MP_COPYSELECTED			10326
#define	MP_SELECTALL			10327
#define	MP_AUTOSCROLL			10328
#define MP_RESUMENEXT			10329
#define MPG_ALTENTER			10330
#define MPG_F2					10331
#define	MP_RENAME				10332
#define	MP_FIND					10333
#define	MP_UNDO					10334
#define	MP_CUT					10335
#define	MP_PASTE				10336
#define MP_DOWNLOAD_ALPHABETICAL 10337
#define MP_A4AF_CHECK_THIS_NOW	10338
#define	MPG_CUT					10339
#define MP_GETKADSOURCELINK		10340
#define MP_SHOWED2KLINK			10341
#define MP_SETSOURCELIMIT		10342
#define MP_CREATECOLLECTION		10343
#define MP_VIEWCOLLECTION		10344
#define MP_MODIFYCOLLECTION		10345
#define MP_SHAREDIR				10346
#define MP_SHAREDIRSUB			10347
#define MP_UNSHAREDIR			10348
#define MP_UNSHAREDIRSUB		10349
#define MP_SEARCHRELATED		10350
#define MP_SEARCHAUTHOR			10351
#define MP_RESTORESEARCHPARAMS	10352
#define	MP_FILTER_RESET			10353
#define	MP_FILTER_APPLY_ALL		10354
#define	MP_FILTER_APPLY_SINGLE	10355
#define	MP_SHOW_FILESIZE_DFLT	10356
#define	MP_SHOW_FILESIZE_KBYTE	10357
#define	MP_SHOW_FILESIZE_MBYTE	10358
#define MP_MARKASSPAM			10359

// NEO: NV - [NeoVersion] -- Xanatos -->
#define MP_X_LINK1				20001
#define MP_X_LINK2				20002
#define MP_X_LINK3				20003
// NEO: NV END <-- Xanatos --

#define	MP_ENABLE_SPEEDMETER	21000 // NEO: PSM - [PlusSpeedMeter] <-- Xanatos --

#define	MP_HM_QUICKSTART		20101 // NEO: QS - [QuickStart] <-- Xanatos --

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
#define MP_HM_VOODOOLIST		20102
#endif // VOODOO // NEO: VOODOO END

#define	MP_HM_TRAY_LOCL			20385  // NEO: TPP - [TrayPasswordProtection] <-- Xanatos --

#define MP_EDITSERVER			20111 // NEO: MOD - [EditServer] <-- Xanatos --
// NEO: PIX - [PartImportExport] -- Xanatos -->
#define	MP_EXPORT				20004
#define	MP_IMPORT				20005
// NEO: PIX END <-- Xanatos --

#define MP_PREALOCATE			20006 // NEO: MOD - [SpaceAllocate] <-- Xanatos --

// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
#define MP_MTD_MENU				20007
#define MP_MTD_MOVE				20008
#define MP_MTD_SELECT			20009
#define MP_MTD_LOAD				20010
#define MP_MTD_LOADDIR			20011
#define MP_MTD_UNLOAD			20012

#define MP_TEMPAUTO				20899
#define MP_TEMPLIST				20900 
// NEO: MTD END <-- Xanatos --

#define MP_ADD_UNC				20013 // NEO: SSD - [ShareSubDirectories] <-- Xanatos --

// NEO: SSF - [ShareSingleFiles] -- Xanatos -->
#define MP_SHARE_FILE			20014
#define MP_UNSHARE_FILE			20015 
// NEO: SSF END <-- Xanatos --

// NEO: MCM - [ManualClientManagement] -- Xanatos -->
#define MP_FORCEA4AF			10370
#define MP_SWAP_TO_A4AF			10371
#define MP_SWAP_FROM_A4AF		10372

#define MP_DROP_CLIENT			20204
#define MP_STOP_CLIENT			20208
#define MP_SWAP_TO_CLIENT		20205
#define MP_SWAP_FROM_CLIENT		20206
#define MP_LOCK_CLIENT			20207
// NEO: MCM END <-- Xanatos --

// NEO: AKF - [AllKnownFiles] -- Xanatos -->
#define MP_VIEWS_SHARED			20016
#define MP_VIEWS_KNOWN			20017
// NEO: AKF END <-- Xanatos --

// NEO: FCFG - [FileConfiguration] -- Xanatos -->
#define MP_TWEAKS				20101
#define MP_FCFG_CLEAR			20102
#define MP_FCFG_COPY			20103
#define MP_FCFG_PASTE			20104
#define MP_FCFG_PASTE2			20105
// NEO: FCFG END <-- Xanatos --

#define MP_PRIORELEASE			20210 // NEO: SRS - [SmartReleaseSharing] <-- Xanatos --

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
#define MP_EXPORTCLIENT			20400
#define MP_LOADSOURCE			20401
#define MP_SAVESOURCE			20402
#define MP_IMPORTSOURCE			20403
#define MP_EXPORTSOURCE			20404
#endif // NEO_SS // NEO: NSS END <-- Xanatos --
#ifdef NEO_CD // NEO: SFL - [SourceFileList] -- Xanatos -->
#define MP_FINDSOURCES			20405
#endif // NEO_CD // NEO: SFL END <-- Xanatos --

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
#define MP_HM_SOURCELIST		20383
#endif // NEO_CD // NEO: NCD END <-- Xanatos --

// NEO: MSR - [ManualSourceRequest] -- Xanatos -->
#define MP_COLLECT_XS_SOURCES	20251
#define MP_COLLECT_SVR_SOURCES	20252
#define MP_COLLECT_KAD_SOURCES	20253
#define MP_COLLECT_UDP_SOURCES	20254
#define MP_COLLECT_ALL_SOURCES	20255
#ifdef VOODOO // NEO: VOODOOx - [VoodooSourceExchange]
#define MP_COLLECT_VOODOO_SOURCES	20256
#endif // VOODOO // NEO: VOODOOx END

#define MP_AHL_INCREASE			20270
#define MP_AHL_DECREASE			20271
// NEO: MSR END <-- Xanatos --

// NEO: MSD - [ManualSourcesDrop] -- Xanatos -->
#define MP_DROP_NNP				20261
#define MP_DROP_FULLQ			20262
#define MP_DROP_HIGHQ			20263
#define MP_DROP_WAITINGRETRY	20390
#define MP_DROP_CACHED			20397 // NEO: XSC - [ExtremeSourceCache]
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
#define MP_DROP_LOADED			20391
#endif // NEO_SS // NEO: NSS END
#define MP_DROP_TOMANY			20264
#define MP_DROP_LOW2LOW			20269
// NEO: MSD END <-- Xanatos --

// NEO: MCS - [ManualChunkSelection] -- Xanatos -->
#define MP_PARTWANTED			20217 
#define MP_CLEARWANTED			20218
#define MP_LINEARWANTED			20219
// NEO: MCS END <-- Xanatos --

#define MP_FORCE				20201 // NEO: OCF - [OnlyCompleetFiles] <-- Xanatos --
#define MP_STANDBY				20202 // NEO: SD - [StandByDL] <-- Xanatos --
#define MP_SUSPEND				20204 // NEO: SC - [SuspendCollecting] <-- Xanatos --

// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
#define MP_CAT_SHOWHIDEPAUSED	10360
#define MP_CAT_SETRESUMEORDER	10361
#define	MP_CAT_ORDERAUTOINC		10362
#define MP_CAT_ORDERSTEPTHRU	10363
#define MP_CAT_ORDERALLSAME		10364
//#define MP_CAT_RESUMENEXT		10365 // MP_RESUMENEXT
#define	MP_CAT_PAUSELAST		10366
#define MP_CAT_STOPLAST			10367
#define MP_CAT_MERGE			10368


#define MP_FORCEA4AF			10370
#define MP_SWAP_TO_A4AF			10371
#define MP_SWAP_FROM_A4AF		10372

// 10373 to 10375 reserved for A4AF menu items.
#define MP_CAT_A4AF				10373
// NEO: NXC END <-- Xanatos --

#define	MP_VIEWN_0				20900 // NEO: NTB - [NeoToolbarButtons] <-- Xanatos --

// NEO: NPT - [NeoPartTraffic] -- Xanatos -->
#define MP_CLEARALLSTATS		20201 
#define MP_CLEARSTATS			20202 
// NEO: NPT END <-- Xanatos --
#define MP_RECALC_IPS			20203 // NEO: IPS - [InteligentPartSharing] <-- Xanatos --
#define MP_RECALC_PS			20204 // NEO: MOD - [ReleaseMenu] <-- Xanatos --
#define MP_CLEARCOMMENTS		20205 // NEO: XCs - [SaveComments] <-- Xanatos --

// NEO: MPS - [ManualPartSharing] -- Xanatos -->
#define MP_PARTNORMAL			20203
#define MP_PARTON				20204
#define MP_PARTHIDE				20205
#define MP_PARTBLOCK			20206

#define MP_CLEARBLOCK			20206 
#define MP_COPYBLOCK			20207
#define MP_PASTEBLOCK			20208
// NEO: MPS END <-- Xanatos --

// NEO: SSP - [ShowSharePermissions] -- Xanatos -->
#define MP_PERMALL				20122
#define MP_PERMFRIENDS			20123
#define MP_PERMNONE				20124
#define MP_PERMDEFAULT			20125 
// NEO: SSP END <-- Xanatos --

// NEO: PP - [PasswordProtection] -- Xanatos -->
#define MP_PWPROT_HIDE			20140
#define MP_PWPROT_SHOW			20141
#define MP_PWPROT_SET			20142
#define MP_PWPROT_CHANGE		20143
#define MP_PWPROT_UNSET			20144
// NEO: PP END <-- Xanatos --

#define MP_DROP_ALL_FRIENDSLOTS	20404 // NEO: NMFS - [NiceMultiFriendSlots] <-- Xanatos --
// NEO: TFL - [TetraFriendLinks] -- Xanatos -->
#define MP_COPY_FRIENDLINK		20402
#define MP_PASTE_FRIENDLINK		20403
// NEO: TFL END <-- Xanatos --

#define	MP_COPYFEEDBACK			20197 // NEO: FB - [FeedBack] <-- Xanatos --

#define MP_SEARCHHASH			20198 // NEO: MOD - [SearchHash] <-- Xanatos --

#define MP_MASSRENAME			20150  // NEO: MMR - [MorphMassRemane] <-- Xanatos --

// NEO: VSF - [VirtualSharedFiles] -- Xanatos -->
#define MP_IOM_VIRTFILE			20362
#define MP_IOM_VIRTDIR			20363
#define MP_IOM_VIRTSUBDIR		20364
#define MP_IOM_VIRTREMOVE		20365
#define MP_IOM_VIRTPREFS		20366
#define MP_IOM_NEW_ENTRY		20367
#define	MP_IOM_SET_DIR			20368
#define	MP_IOM_SET_SUBDIR		20369
#define	MP_IOM_REMOVE			20370
#define MP_IOM_COPY				20371
#define MP_LOCAL_FILES			20005
// NEO: VSF END <-- Xanatos --

// NEO: CRC - [MorphCRCTag] -- Xanatos -->
#define MP_CRC32_CALCULATE		20151
#define MP_CRC32_TAG			20152
#define MP_CRC32_ABORT			20153
#define MP_CRC32_RECALCULATE	20154
// NEO: CRC END <-- Xanatos --

// quick-speed changer
#define MP_QS_U10				10501
#define MP_QS_U20				10502
#define MP_QS_U30				10503
#define MP_QS_U40				10504
#define MP_QS_U50				10505
#define MP_QS_U60				10506
#define MP_QS_U70				10507
#define MP_QS_U80				10508
#define MP_QS_U90				10509
#define MP_QS_U100				10510
#define MP_QS_UPC				10511
#define MP_QS_UP10				10512
#define MP_QS_UPL				10513
#define MP_QS_D10				10521
#define MP_QS_D20				10522
#define MP_QS_D30				10523
#define MP_QS_D40				10524
#define MP_QS_D50				10525
#define MP_QS_D60				10526
#define MP_QS_D70				10527
#define MP_QS_D80				10528
#define MP_QS_D90				10529
#define MP_QS_D100				10530
#define MP_QS_DC				10531
#define MP_QS_DL				10532
#define MP_QS_PA				10533
#define MP_QS_UA				10534

#define MP_WEBURL				10600	// reserve 100 entries for weburls!
#define MP_ASSIGNCAT			10700	// reserve 100 entries for categories!
#define MP_SCHACTIONS			10800	// reserve 100 entries for schedules
#define MP_CAT_SET0				10900	// reserve 100 entries for change all-cats
#define MP_TOOLBARBITMAP		11000	// reserve 100 entries for toolbar bitmaps
#define	MP_SKIN_PROFILE			11100	// reserve 100 entries for skin profiles
#define	MP_PREVIEW_APP_MIN		11200	// reserve 50 entries for preview apps
#define	MP_PREVIEW_APP_MAX		(MP_PREVIEW_APP_MIN+49)
#define MP_FILTERCOLUMNS		11300	// reserve 50 entries for filter columsn

#define Irc_Join				10240
#define Irc_Close				10241
#define Irc_Priv				10242
#define Irc_AddFriend			10243
#define	Irc_SendLink			10244
#define Irc_SetSendLink			10245
#define Irc_Kick				10246
#define Irc_Ban					10247
#define Irc_KB					10248
#define Irc_Slap				10249
//Note: reserve at least 50 ID's (Irc_OpCommands-Irc_OpCommands+49).
#define Irc_OpCommands			10250
//Note: reserve at least 100 ID's (Irc_ChanCommands-Irc_ChanCommands+99).
#define Irc_ChanCommands		Irc_OpCommands+50
