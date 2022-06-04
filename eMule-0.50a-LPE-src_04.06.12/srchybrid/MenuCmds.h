#pragma once

///////////////////////////////////////////////////////////////////////////////
// Menu commands for GUI only

// Menu entries for the application system menu -> require a set of IDs with special restrictions!
#define MP_RESTORE				0x110
#define MP_CONNECT				0x120
#define MP_DISCONNECT			0x130
#define MP_EXIT					0x140

#define MP_MESSAGE				10102
#define MP_DETAIL				10103
#define MP_SHOWLIST				10106

#define MP_CANCEL				10201
#define MP_STOP					10202
#define MP_PAUSE				10203
#define MP_RESUME				10204
#define	MP_OPEN					10206
#define	MP_PREVIEW				10207
//#define MP_HM_CON				10209
#define MP_HM_SRVR				10210
#define MP_HM_TRANSFER			10211
#define MP_HM_SEARCH			10212
#define MP_HM_STATS				10213
#define MP_HM_PREFS				10217
#define MP_HM_OPENINC			10218
#define MP_HM_EXIT				10219
#define MP_TRY_TO_GET_PREVIEW_PARTS 10220
#define MP_ADDSOURCE			10221
#define MP_ALL_A4AF_AUTO		10222
#define MP_META_DATA			10225
#define MP_BOOT					10226
#define MP_RESUMEPAUSED			10228
#define MP_OPENFOLDER			10244
#define	MP_HM_DIRECT_DOWNLOAD	10247
#define	MP_VIEW2_DOWNLOADING	10252
#define	MP_VIEW2_UPLOADING		10253
#define MP_VIEW1_SPLIT_WINDOW	10255
#define MP_VIEW1_HISTORY		10258
#define MP_PAUSEONPREVIEW		10261
#define MP_VIEW1_SHARED			10272
#define MP_VIEW1_SEARCH			10273
#define MP_VIEW1_FILTER			10274

#define MP_PRIOVERYLOW			10300
#define MP_PRIOLOW				10301
#define MP_PRIONORMAL			10302
#define MP_PRIOHIGH				10303
#define MP_PRIOVERYHIGH			10304
#define MP_GETED2KLINK			10305
#define MP_METINFO				10307
#define MP_PERMALL				10308
#define MP_PERMDEFAULT			10309 //Upload Permission
#define MP_PERMNONE				10310
#define MP_CONNECTTO			10311
#define MP_REMOVE				10312
#define MP_REMOVEALL			10313
#define MP_REMOVESELECTED		10314
#define MP_UNBAN				10315
#define MP_ADDTOSTATIC			10316
#define MP_CLCOMMAND			10317
#define MP_PRIOAUTO				10317
#define MP_REMOVEFROMSTATIC		10318
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
#define	MP_UNDO					10334
#define	MP_CUT					10335
#define	MP_PASTE				10336
#define MP_DOWNLOAD_ALPHABETICAL 10337
#define MP_A4AF_CHECK_THIS_NOW	10338
#define	MPG_CUT					10339
#define MP_GETKADSOURCELINK		10340
#define MP_SHOWED2KLINK			10341
#define MP_SETSOURCELIMIT		10342
#define MP_VIEWCOLLECTION		10344
//#define MP_SEARCHRELATED		10350
#define MP_SEARCHAUTHOR			10351
#define MP_RESTORESEARCHPARAMS	10352
#define	MP_FILTER_RESET			10353
#define	MP_FILTER_APPLY_ALL		10354
#define	MP_FILTER_APPLY_SINGLE	10355
#define	MP_SHOW_FILESIZE_DFLT	10356
#define	MP_SHOW_FILESIZE_KBYTE	10357
#define	MP_SHOW_FILESIZE_MBYTE	10358
#define MP_MARKASSPAM			10359
#define MP_UNSHAREFILE			10360
#define MP_NEWCAT				10361

// quick-speed changer
#define MP_QS_U20				10502
#define MP_QS_U40				10504
#define MP_QS_U60				10506
#define MP_QS_U80				10508
#define MP_QS_U100				10510
#define MP_QS_UP10				10512
#define MP_QS_D20				10522
#define MP_QS_D40				10524
#define MP_QS_D60				10526
#define MP_QS_D80				10528
#define MP_QS_D100				10530
#define MP_QS_PA				10533
#define MP_QS_UA				10534

#define MP_ASSIGNCAT			10700	// reserve 100 entries for categories!
#define MP_SCHACTIONS			10800	// reserve 100 entries for schedules
#define MP_CAT_SET0				10900	// reserve 100 entries for change all-cats
#define	MP_PREVIEW_APP_MIN		11200	// reserve 50 entries for preview apps
#define	MP_PREVIEW_APP_MAX		(MP_PREVIEW_APP_MIN+49)
#define MP_FILTERCOLUMNS		11300	// reserve 50 entries for filter columsn


//Xman Xtreme Downloadmanager
#define MP_SWAP_A4AF_TO_THIS	14003
#define MP_SWAP_A4AF_TO_OTHER   14004
#define MP_STOP_CLIENT			14005
#define MP_ALL_A4AF_TO_THIS	    14006
#define MP_ALL_A4AF_TO_OTHER	14007
#define MP_DROPNONEEDEDSRCS		14008
#define MP_DROPQUEUEFULLSRCS	14009
//Xman PowerRelease
#define MP_PRIOPOWER			14010
//Xman add search to cancelled
#define MP_ADDSEARCHCANCELLED	14011

//Xman [MoNKi: -Downloaded History-]
#define MP_VIEWSHAREDFILES		14013
#define MP_CLEARHISTORY			14014
//Xman end
#define	MP_PREALOCATE			14018 //Xman manual file allocation (Xanatos)

#define MP_FLUSHBUFFER			15006// X: [FB] - [FlushBuffer]
#define MP_C0SC					15007// X: [C0SC] - [Clear0SpeedClient]
#define	MP_HM_RESUME_DOWNLOAD	15008
#define MP_AICHHASH				15009// X: [IP] - [Import Parts]
#define MP_SHOWSHAREABLEFILES		15015
#define	MP_HM_REMOVEALLBANNEDCLIENTS	15016

#define	MP_HM_TOGGLESPEEDGRAPH		15017// X: [SGW] - [SpeedGraphWnd]
#define MP_HM_SHOWTOOLBAR 15200
#define MP_HM_SHOWCATTABBAR      15201
#define MP_GRIDLINES            20026
#define MP_PROGRESS            20027 //morph4u :: PercentBar
#define MP_SHUT_EMULE           20028
#define MP_SHUT_PC              20029
#define MP_UPDATE_SERVER        20030
#define MP_SPREADBAR_RESET      20031 // Spread bars [Slugfiller/MorphXT] - Stulle
#define MP_RE_ASK               20032
#define MP_LISTICONS            20033
#define	MP_HM_QUICKSTART		20101 // NEO: QS - [QuickStart] <-- Xanatos --
#define MP_REMOVEUPLOAD         20102
#define MP_PERMUPMANA			20103 //Upload Permission
