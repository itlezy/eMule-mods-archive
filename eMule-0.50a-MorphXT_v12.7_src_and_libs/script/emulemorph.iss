; create by leuk_he on 10-01-2006
; (c) 2006-2009 leuk_he (leukhe at gmail dot com)

[Setup]
AppName=eMule MorphXT
; !!!!!!!!!!  Following always needs to be updated  !!!!!!!!!!
AppVerName=eMule MorphXT 12.7
AppPublisher=Morph team
AppPublisherURL=http://emulemorph.sourceforge.net/
AppSupportURL=http://forum.emule-project.net/index.php?showforum=28
AppUpdatesURL=http://sourceforge.net/project/showfiles.php?group_id=72158&package_id=107495
AppCopyright=(c) 2012 Morph team

UsePreviousAppDir=yes
DirExistsWarning=No
DefaultDirName={pf}\eMule
DefaultGroupName=eMule
AllowNoIcons=yes
; gpl:   no accept is required, see point 5 of the gpl
InfoBeforeFile=..\staging\license.txt
WizardImageFile=MorphBanner.bmp
WizardImageStretch=no

; this dir:
OutputDir=.
OutputBaseFilename=morphemuleversion-installer
SetupIconFile=..\srchybrid\res\mod\installerico.ico
Compression=lzma
;fast for debug:
;Compression=zip
SolidCompression=yes
;emule does not work on win98 due to high "rc" resource usage.
minversion=0,5.0


PrivilegesRequired=poweruser


[Languages]
Name: english; MessagesFile: compiler:Default.isl
;Name: "dutch"; MessagesFile: "compiler:Languages\Dutch.isl"
Name: german; MessagesFile: compiler:Languages\German.isl; InfoBeforeFile: ..\staging\license-GER.txt
Name: italian; MessagesFile: compiler:Languages\Italian.isl; InfoBeforeFile: ..\staging\license-IT.txt
Name: spanish; MessagesFile: compiler:Languages\Spanish.isl; InfoBeforeFile: ..\staging\license-SP.txt
Name: polish; MessagesFile: compiler:Languages\Polish.isl
Name: french; MessagesFile: compiler:Languages\French.isl; InfoBeforeFile: ..\staging\license-FR.txt
Name: BrazilianPortuguese; MessagesFile: compiler:Languages\BrazilianPortuguese.isl; InfoBeforeFile: ..\staging\license-PT_BR.txt
Name: ChineseSimpl; MessagesFile: ChineseSimp-12-5.1.11.isl; InfoBeforeFile: ..\staging\license-CN.txt
Name: ChineseTrad; MessagesFile: ChineseTrad-2-5.1.11.isl
Name: turkish; MessagesFile: Turkish-3-5.1.11.isl; InfoBeforeFile: ..\staging\license-TR.txt

[Dirs]
;make dir writeable for other users than administrator
Name: {app}\config; Permissions: users-modify; tasks: progsdir
Name: {app}\temp; Permissions: users-modify; tasks: progsdir
Name: {app}\incoming; Permissions: users-modify; tasks: progsdir

Name: {commonappdata}\eMule\config; Permissions: users-modify; components: configfiles; tasks: commonapp
Name: {commonappdata}\eMule\temp; Permissions: users-modify; tasks: commonapp
Name: {commonappdata}\eMule\incoming; Permissions: users-modify; tasks: commonapp
; DO not make dir for user directory userwrteable since that would be openeing up the system to not permitted users.

[Files]
;todo show correct languge in startup
Source: ..\staging\emule.exe; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\Template.eMuleSkin.ini; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\Template.Notifier.ini; DestDir: {app}; Flags: ignoreversion
;Source: ..\staging\changelog.ger.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\Changelog.MorphXT.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\changelog.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\eMule.exe; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\license.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\license-DK.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\license-FR.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\license-GER.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\license-GR.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\license-HE.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\license-IT.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\license-KO.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\license-LT.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\license-PT_BR.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\license-PT_PT.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\license-RU.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\license-SP.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\license-TR.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\webserver\*; DestDir: {app}\webserver; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ..\staging\wapserver\*; DestDir: {app}\wapserver; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ..\staging\readme.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\eMule.chm; DestDir: {app}; Flags: ignoreversion  onlyifdoesntexist; components: helpfiles
Source: ..\staging\eMule.1031.chm; DestDir: {app}; Flags: ignoreversion  onlyifdoesntexist; components: helpfiles
Source: ..\staging\unrar.dll; DestDir: {app}; components: tpartytools\unrar
Source: ..\staging\unrarlicense.txt; DestDir: {app}; Flags: ignoreversion; components: tpartytools\unrar
Source: ..\staging\mediainfo.dll; DestDir: {app}; Flags: ignoreversion onlyifdoesntexist; components: tpartytools\mediainfo
Source: ..\staging\mediainfo_ReadMe_DLL.txt; DestDir: {app}; Flags: ignoreversion  onlyifdoesntexist; components: tpartytools\mediainfo
; shell extension files
Source: ..\eMuleShellExt\Release\eMuleShellExt.dll; DestDir: {app}; Flags: regserver ignoreversion; components: shellextention
Source: ..\eMuleShellExt\doc\eMule Shell Extension.htm; DestDir: {app}; Flags: ignoreversion; components: shellextention
Source: ..\eMuleShellExt\doc\eMule Shell Extension-DE.PNG; DestDir: {app}; Flags: ignoreversion; components: shellextention

Source: ..\staging\lang\*; DestDir: {app}\lang; Flags: ignoreversion recursesubdirs createallsubdirs; Components: langs

; task: config is in progdir:
Source: ..\staging\emule\config\AC_ServerMetURLs.dat; DestDir: {app}\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: progsdir
Source: ..\staging\emule\config\AC_SearchStrings.dat; DestDir: {app}\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: progsdir
Source: ..\staging\emule\config\countryflag.dll; DestDir: {app}\config; Flags: ignoreversion; tasks: progsdir
Source: ..\staging\emule\config\countryflag32.dll; DestDir: {app}\config; Flags: ignoreversion; tasks: progsdir
Source: ..\staging\emule\config\eMule Light.tmpl; DestDir: {app}\config; Flags: ignoreversion; tasks: progsdir
Source: ..\staging\emule\config\Multiuser eMule.tmpl; DestDir: {app}\config; Flags: ignoreversion; tasks: progsdir
Source: ..\staging\emule\config\eMule.tmpl; DestDir: {app}\config; Flags: ignoreversion; tasks: progsdir
Source: ..\staging\emule\config\startup.wav; DestDir: {app}\config; Flags: ignoreversion; tasks: progsdir
Source: ..\staging\emule\config\webservices.dat; DestDir: {app}\config; Flags: ignoreversion; components: configfiles; tasks: progsdir
Source: ..\staging\emule\config\XMLNews.dat; DestDir: {app}\config; Flags: ignoreversion; components: configfiles; tasks: progsdir
Source: ..\staging\emule\config\server.met; DestDir: {app}\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; tasks: progsdir
Source: ..\staging\emule\config\addresses.dat; DestDir: {app}\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: progsdir
Source: ..\staging\emule\config\nodes.dat; DestDir: {app}\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: progsdir
Source: ..\staging\emule\config\GeoIPCountryWhois.csv; DestDir: {app}\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: progsdir
Source: ..\staging\emule\config\ipfilter.dat; DestDir: {app}\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfilesipf; tasks: progsdir
Source: ..\staging\emule\config\staticservers.dat; DestDir: {app}\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: progsdir
; to assing correct rights:
Source: ..\staging\emule\config\preferences.ini; DestDir: {app}\config; Flags: ignoreversion onlyifdoesntexist uninsneveruninstall; Permissions: users-modify; tasks: progsdir

;appdir
Source: ..\staging\emule\config\AC_ServerMetURLs.dat; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: commonapp
Source: ..\staging\emule\config\AC_SearchStrings.dat; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: commonapp
Source: ..\staging\emule\config\countryflag.dll; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion; tasks: commonapp
Source: ..\staging\emule\config\countryflag32.dll; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion; tasks: commonapp
Source: ..\staging\emule\config\eMule Light.tmpl; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion; tasks: commonapp
Source: ..\staging\emule\config\Multiuser eMule.tmpl; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion; tasks: commonapp
Source: ..\staging\emule\config\eMule.tmpl; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion; tasks: commonapp
Source: ..\staging\emule\config\startup.wav; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion; tasks: commonapp
Source: ..\staging\emule\config\webservices.dat; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion; components: configfiles; tasks: commonapp
Source: ..\staging\emule\config\XMLNews.dat; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion; components: configfiles; tasks: commonapp
Source: ..\staging\emule\config\server.met; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; tasks: commonapp
Source: ..\staging\emule\config\addresses.dat; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: commonapp
Source: ..\staging\emule\config\nodes.dat; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: commonapp
Source: ..\staging\emule\config\GeoIPCountryWhois.csv; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: commonapp
Source: ..\staging\emule\config\ipfilter.dat; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfilesipf; tasks: commonapp
Source: ..\staging\emule\config\staticservers.dat; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: commonapp
; to assing correct rights:
Source: ..\staging\emule\config\preferences.ini; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist uninsneveruninstall; Permissions: users-modify; tasks: commonapp

;mydocuments   xp
Source: ..\staging\emule\config\AC_ServerMetURLs.dat; DestDir: {userappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\AC_SearchStrings.dat; DestDir: {userappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\countryflag.dll; DestDir: {userappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\countryflag32.dll; DestDir: {userappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\eMule Light.tmpl; DestDir: {userappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\Multiuser eMule.tmpl; DestDir: {userappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\eMule.tmpl; DestDir: {userappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\startup.wav; DestDir: {userappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\webservices.dat; DestDir: {userappdata}\eMule\config; Flags: ignoreversion; components: configfiles; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\XMLNews.dat; DestDir: {userappdata}\eMule\config; Flags: ignoreversion; components: configfiles; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\server.met; DestDir: {userappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\addresses.dat; DestDir: {userappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\GeoIPCountryWhois.csv; DestDir: {userappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\ipfilter.dat; DestDir: {userappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfilesipf; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\staticservers.dat; DestDir: {userappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; OnlyBelowVersion: 0,6


;mydocuments   vista
Source: ..\staging\emule\config\AC_ServerMetURLs.dat; DestDir: {localappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\AC_SearchStrings.dat; DestDir: {localappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\countryflag.dll; DestDir: {localappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\countryflag32.dll; DestDir: {localappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\eMule Light.tmpl; DestDir: {localappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\Multiuser eMule.tmpl; DestDir: {localappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\eMule.tmpl; DestDir: {localappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\startup.wav; DestDir: {localappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\webservices.dat; DestDir: {localappdata}\eMule\config; Flags: ignoreversion; components: configfiles; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\XMLNews.dat; DestDir: {localappdata}\eMule\config; Flags: ignoreversion; components: configfiles; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\server.met; DestDir: {localappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\addresses.dat; DestDir: {localappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\nodes.dat; DestDir: {localappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\GeoIPCountryWhois.csv; DestDir: {localappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\ipfilter.dat; DestDir: {localappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfilesipf; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\staticservers.dat; DestDir: {localappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; minversion: 0,6


; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Components]
Name: Main; Description: {cm:corefiles}; Types: full compact custom; FLAGS: fixed
Name: helpfiles; Description: {cm:helpfile}; Types: full
Name: langs; Description: {cm:languagesupport}; Types: full
Name: shellextention; Description: {cm:shellextention}; Types: full
Name: configfiles; Description: {cm:configfiles}; Types: full
Name: configfilesipf; Description: {cm:configfilesipf}
;Types: full Not because ipfilter is too old at current release speed
Name: tpartytools; Description: {cm:tparty}; Types: full
Name: tpartytools\mediainfo; Description: {cm:mediainfo}; Types: full
Name: tpartytools\unrar; Description: {cm:unrar}; Types: full


[Tasks]
; trick it here: since i connot find documentatuib how to set a check box programmatically from
; pascal script there are just 3 instances with a different check to set the default according
; to the current regististry (default is userdata on vista and above, prog dir on xp and below [like official])
Name: progsdir; Description: {cm:progdir}; Check: UsePublicUserDirectories2; GroupDescription: {cm:locofdata}; Flags: exclusive
Name: commonapp; Description: {cm:appdir}; Check: UsePublicUserDirectories2; GroupDescription: {cm:locofdata}; Flags: exclusive  unchecked; MinVersion: 0,6.0
Name: userdata; Description: {cm:userdocdir}; Check: UsePublicUserDirectories2; GroupDescription: {cm:locofdata}; Flags: exclusive unchecked

Name: progsdir; Description: {cm:progdir}; Check: UsePublicUserDirectories1; GroupDescription: {cm:locofdata}; Flags: exclusive unchecked
Name: commonapp; Description: {cm:appdir}; Check: UsePublicUserDirectories1; GroupDescription: {cm:locofdata}; Flags: exclusive; MinVersion: 0,6.0
Name: userdata; Description: {cm:userdocdir}; Check: UsePublicUserDirectories1; GroupDescription: {cm:locofdata}; Flags: exclusive unchecked

Name: progsdir; Description: {cm:progdir}; Check: UsePublicUserDirectories0; GroupDescription: {cm:locofdata}; Flags: exclusive  unchecked
Name: commonapp; Description: {cm:appdir}; Check: UsePublicUserDirectories0; GroupDescription: {cm:locofdata}; Flags: exclusive  unchecked; MinVersion: 0,6.0
Name: userdata; Description: {cm:userdocdir}; Check: UsePublicUserDirectories0; GroupDescription: {cm:locofdata}; Flags: exclusive
;; - end trick with CHECK
Name: desktopicon; Description: {cm:CreateDesktopIcon}; flags: unchecked
Name: firewall; Description: {cm:tasks_firewall}; MinVersion: 0,5.01sp2
Name: urlassoc; Description: {cm:tasks_assocurl}; GroupDescription: {cm:othertasks}

[INI]
Filename: {app}\emulemorphhome.url; Section: InternetShortcut; Key: URL; String: http://emulemorph.sourceforge.net/

[Icons]
Name: {group}\eMule MorphXT; Filename: {app}\emule.exe; Comment: eMule MorphXT
Name: {group}\{cm:ProgramOnTheWeb,eMule Morph}; Filename: {app}\emulemorphhome.url
Name: {group}\{cm:UninstallProgram,eMule}; Filename: {uninstallexe}
Name: {group}\Shell extension doc; Filename: {app}\eMule Shell Extension.htm; components: shellextention
Name: {userdesktop}\eMule; Filename: {app}\emule.exe; Tasks: desktopicon

[Registry]
Root: HKCR; Subkey: ed2k; ValueType: string; ValueData: URL:ed2k Protocol; Flags: uninsdeletekey; Tasks: urlassoc
Root: HKCR; Subkey: ed2k; ValueName: URL Protocol; ValueType: string; Flags: uninsdeletekey; Tasks: urlassoc
Root: HKCR; Subkey: ed2k\Shell; ValueType: string; Flags: uninsdeletekey; Tasks: urlassoc
Root: HKCR; Subkey: ed2k\Shell\open; ValueType: string; Flags: uninsdeletekey; Tasks: urlassoc
Root: HKCR; Subkey: ed2k\Shell\open\Command; ValueType: string; ValueData: "{app}\emule.exe ""%1"""; Flags: uninsdeletekey; Tasks: urlassoc

Root: HKCR; Subkey: .emulecollection; ValueType: string; ValueName: ; ValueData: eMule; Flags: uninsdeletevalue; Tasks: urlassoc
Root: HKCR; Subkey: eMule; ValueType: string; ValueName: ; ValueData: eMule; Flags: uninsdeletekey; Tasks: urlassoc
Root: HKCR; Subkey: eMule\DefaultIcon; ValueType: string; ValueName: ; ValueData: {app}\eMule.exe,1; Tasks: urlassoc
Root: HKCR; Subkey: eMule\shell\open\Command; ValueType: string; ValueData: "{app}\emule.exe ""%1"""; Flags: uninsdeletekey; Tasks: urlassoc
; shell extension uninstall:
Root: HKCR; Subkey: .met; ValueType: string; ValueName: ; ValueData: "metfile "; Flags: uninsdeletevalue; components: shellextention
Root: HKCR; Subkey: .part; ValueType: string; ValueName: ; ValueData: "partfile "; Flags: uninsdeletevalue; components: shellextention
Root: HKCR; Subkey: CLSID\{{5F081689-CE7D-43E7-8B11-DAD99A4A96D6}; ValueName: ; ValueData: eMule shell extension; ValueType: string; Flags: uninsdeletevalue; components: shellextention
Root: HKCR; Subkey: CLSID\{{5F081689-CE7D-43E7-8B11-DAD99A4A96D6}\InprocServer32; ValueName: ; ValueData: {app}\eMuleShellExt.dll; ValueType: string; Flags: uninsdeletevalue; components: shellextention
Root: HKCR; Subkey: CLSID\{{5F081689-CE7D-43E7-8B11-DAD99A4A96D6}\InprocServer32; ValueName: ThreadingModel; ValueData: Both; ValueType: string; Flags: uninsdeletevalue; components: shellextention
Root: HKCR; Subkey: metfile; ValueType: string; ValueName: Details; ValueData: "prop:DocTitle;Artist;Album;Duration;Bitrate;Write"; Flags: uninsdeletevalue; components: shellextention
Root: HKCR; Subkey: metfile; ValueType: string; ValueName: infoTip; ValueData: "prop:DocTitle;Artist;Album;Duration;Bitrate;Write"; Flags: uninsdeletevalue; components: shellextention
Root: HKCR; Subkey: metfile; ValueType: string; ValueName: Tileinfo; ValueData: "prop:DocTitle;Size"; Flags: uninsdeletevalue; components: shellextention
Root: HKCR; Subkey: metfile\DefaultIcon; ValueType: string; ValueName: ; ValueData: {app}\eMuleShellExt.dll,-201; Flags: uninsdeletevalue; components: shellextention
Root: HKCR; Subkey: partfile\ShellEx\PropertyHandler; ValueType: string; ValueName: """"""; ValueData: {{5F081689-CE7D-43E7-8B11-DAD99A4A96D6}}; Flags: uninsdeletevalue; components: shellextention
Root: HKCR; Subkey: partfile; ValueType: string; ValueName: Details; ValueData: "prop:DocTitle;Artist;Album;Duration;Bitrate;Write"; Flags: uninsdeletevalue; components: shellextention
Root: HKCR; Subkey: partfile; ValueType: string; ValueName: infoTip; ValueData: "prop:DocTitle;Artist;Album;Duration;Bitrate;Write"; Flags: uninsdeletevalue; components: shellextention
Root: HKCR; Subkey: partfile; ValueType: string; ValueName: Tileinfo; ValueData: "prop:DocTitle;Size"; Flags: uninsdeletevalue; components: shellextention
Root: HKCR; Subkey: partfile\DefaultIcon; ValueType: string; ValueName: ; ValueData: {app}\eMuleShellExt.dll,-202; Flags: uninsdeletevalue; components: shellextention
Root: HKCR; Subkey: partfile\ShellEx\PropertyHandler; ValueType: string; ValueName: """"""; ValueData: {{5F081689-CE7D-43E7-8B11-DAD99A4A96D6}; Flags: uninsdeletevalue; components: shellextention
Root: HKCR; Subkey: Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved; ValueType: string; ValueName: """{{5F081689-CE7D-43E7-8B11-DAD99A4A96D6}"""; ValueData: eMule shell extension; Flags: uninsdeletevalue; components: shellextention
; datadir

Root: HKCU; Subkey: SOFTWARE\eMule; Flags: uninsdeletekey
;install correct value based on task:
Root: HKCU; Subkey: SOFTWARE\eMule; ValueType: dword; ValueName: UsePublicUserDirectories; ValueData: 002; Flags: uninsdeletekey; Tasks: progsdir
Root: HKCU; Subkey: SOFTWARE\eMule; ValueType: dword; ValueName: UsePublicUserDirectories; ValueData: 001; Flags: uninsdeletekey; Tasks: commonapp
Root: HKCU; Subkey: SOFTWARE\eMule; ValueType: dword; ValueName: UsePublicUserDirectories; ValueData: 000; Flags: uninsdeletekey; Tasks: userdata

[Run]
Filename: {app}\emule.exe; Description: {cm:LaunchProgram,eMule}; Flags: nowait postinstall skipifsilent; OnlyBelowVersion: 0,6
Filename: netsh; Parameters: "firewall add allowedprogram ""{app}\emule.exe"" ""eMule MorphXT"" ENABLE"; flags: runminimized shellexec; Tasks: firewall; OnlyBelowVersion: 0,6
Filename: netsh; Parameters: "advfirewall firewall add rule name=""eMule MorphXT"" dir=in action=allow program=""{app}\emule.exe"" enable=yes description=""eMule MorphXT inbound traffic"" profile=any"; flags: runminimized shellexec; Tasks: firewall; MinVersion: 0,6
Filename: netsh; Parameters: "advfirewall firewall add rule name=""eMule MorphXT"" dir=out action=allow program=""{app}\emule.exe"" enable=yes description=""eMule MorphXT outbound traffic"" profile=any"; flags: runminimized shellexec; Tasks: firewall; MinVersion: 0,6

[UninstallRun]
Filename: {app}\emule.exe; Parameters: uninstall
Filename: netsh; Parameters: "firewall delete allowedprogram ""{app}\emule.exe"""; flags: runminimized shellexec; Tasks: firewall; OnlyBelowVersion: 0,6
Filename: netsh; Parameters: "advfirewall firewall delete rule name=""eMule MorphXT"" profile=any program=""{app}\emule.exe"""; flags: runminimized shellexec; Tasks: firewall; MinVersion: 0,6


[UninstallDelete]
Type: files; Name: {app}\emulemorphhome.url

[CustomMessages]
; This section specifies phrazes and words not specified in the ISL files
; Avoid customizing the ISL files since they will change with each version of Inno Setup.
; English
;dutch.tasks_firewall=Voeg een uitzonderings regel toe aan de windows firewall.
;dutch.dialog_firewall=Setup heeft emule niet als uitzondering aan de Windows Firewall kunnen toevoegen .%nWellicht moet u dit nog handmatig doen.
;dutch.assocurl=eD2K links doorsturen naar eMule.


#include "morphenglish.isl"

;translations in sepeate file now 10.1

#include "morphitalian.isl"
#include "morphspanish.isl"
#include "morphgerman.isl"
#include "morphportugesebr.isl"
#include "morphfrench.isl"
;codepage:
#include "morphpolish.isl"
;codepage:
#include "emulemorphchinese.iss"
;codepage 950:
#include "morphcn_tw.isl"
;codepage:
#include "morphturkish.isl"

; Code sections need to be the last section in a script or the compiler will get confused
[Code]
const
  WM_CLOSE = $0010;
  NET_FW_SCOPE_ALL = 0;
  NET_FW_IP_VERSION_ANY = 2;
var
  registrychecked: Boolean;
  registrychecksuccessful: Boolean;
  UsePublicUserDirectories: Cardinal;
  Version: TWindowsVersion;

Procedure ReadRegInternal() ;
begin
   UsePublicUserDirectories:=-1;
   if RegQueryDWordValue(HKEY_CURRENT_USER, 'SOFTWARE\eMule',
          'UsePublicUserDirectories',UsePublicUserDirectories) then begin
			registrychecksuccessful:= true;
		end
		else
		begin
			registrychecksuccessful:= false;
		end;
	GetWindowsVersionEx(Version);
    registrychecked:=true;
end;

function UsePublicUserDirectories2(): Boolean;
begin
	if not registrychecked then begin
		ReadRegInternal()
	end;
	if UsePublicUserDirectories = 2 then
	begin
		Result:= true;
	end
    else
    begin
	   if (not registrychecksuccessful) and (Version.Major < 6) then begin
			Result:= true;
		end
		else
		begin
			Result:= False;
		end
	end
end;

function UsePublicUserDirectories1(): Boolean;
begin
	if not registrychecked then begin
		ReadRegInternal()
	end;
	if UsePublicUserDirectories = 1 then
	begin
		Result:= true;
	end
    else
    begin
       Result:= False;
    end
end;

function UsePublicUserDirectories0(): Boolean;
begin
	if not registrychecked then begin
		ReadRegInternal()
	end;
	if UsePublicUserDirectories = 0 then
	begin
		Result:= true;
	end
	else
    begin
	   if (not registrychecksuccessful) and (Version.Major >= 6) then begin
			Result:= true;
		end
		else
		begin
			Result:= False;
		end
	end
end;





Procedure CurStepChanged(CurStep: TSetupStep);
var
  pref: string;
  //Reset: boolean;
Begin
  if CurStep=ssPostInstall then begin
       if IsTaskSelected('progsdir') then
        begin
         pref := ExpandConstant('{app}\config\preferences.ini');
       end;
        if IsTaskSelected('commonapp') then
        begin
           pref := ExpandConstant('{commonappdata}\eMule\config\preferences.ini');
       end;
       if IsTaskSelected('userdata') then   //; todo vista dir for userdata
       begin
           pref := ExpandConstant('{userappdata}\eMule\config\preferences.ini');
       end;

    if CompareText(activelanguage,'dutch')=0 then
       if  not IniKeyExists('eMule', 'Language', pref) then
       begin
          SetIniString('eMule', 'Language','1043',pref );
       end;
    if CompareText(activelanguage,'german')=0 then
       if not IniKeyExists('eMule', 'Language', pref) then
       begin
          SetIniString('eMule', 'Language','1031',pref );
       end;
   if CompareText(activelanguage,'italian')=0 then
       if not IniKeyExists('eMule', 'Language', pref) then
       begin
          SetIniString('eMule', 'Language','1040',pref );
       end;
           if CompareText(activelanguage,'polish')=0 then
       if not IniKeyExists('eMule', 'Language', pref) then
       begin
          SetIniString('eMule', 'Language','1045',pref );
       end;
    if CompareText(activelanguage,'spanish')=0 then
       if  not IniKeyExists('eMule', 'Language', pref) then
       begin
          SetIniString('eMule', 'Language','1034',pref );
       end;
    if CompareText(activelanguage,'french')=0 then
       if  not IniKeyExists('eMule', 'Language', pref) then
       begin
          SetIniString('eMule', 'Language','1036',pref );
       end;
     if CompareText(activelanguage,'BrazilianPortuguese')=0 then
       if  not IniKeyExists('eMule', 'Language', pref) then
       begin
          SetIniString('eMule', 'Language','1046',pref );
       end;
     if CompareText(activelanguage,'ChineseSimpl')=0 then
       if  not IniKeyExists('eMule', 'Language', pref) then
       begin
          SetIniString('eMule', 'Language','2052',pref );
       end;
      //
    if  not IniKeyExists('eMule', 'SetSystemACP', pref)  then
        if  not FileExists('{app}\known.met') then
        begin
        // for a new installion system uni code is recommended anyway:
            SetIniString('eMule', 'SetSystemACP','0',pref );
        end;
   end;

//  ;if CurStep=ssDone then Reset := ResetLanguages;
End;
