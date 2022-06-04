; create by leuk_he on 10-01-2006
; (c) 2006-2009 leuk_he (leukhe at gmail dot com)

[Setup]
AppName=eMule
AppVerName=eMule ScarAngel 4.1
AppPublisher=Stulle
AppPublisherURL=http://scarangel.sourceforge.net/
AppSupportURL=http://forum.emule-project.net/index.php?showforum=97
AppUpdatesURL=https://sourceforge.net/projects/scarangel/files/
AppCopyright=(c) 2011 Stulle

UsePreviousAppDir=yes
DirExistsWarning=No
DefaultDirName={pf}\eMule
DefaultGroupName=eMule
AllowNoIcons=yes
; gpl:   no accept is required, see point 5 of the gpl
InfoBeforeFile=..\staging\license\license.txt
WizardImageFile=ScarBanner.bmp
WizardImageStretch=no

; this dir:
OutputDir=.
OutputBaseFilename=scaremuleversion-installer
SetupIconFile=..\srchybrid\res\scar_inst_256.ico
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
Name: german; MessagesFile: compiler:Languages\German.isl; InfoBeforeFile: ..\staging\license\license-GER.txt
Name: italian; MessagesFile: compiler:Languages\Italian.isl; InfoBeforeFile: ..\staging\license\license-IT.txt
Name: spanish; MessagesFile: compiler:Languages\Spanish.isl; InfoBeforeFile: ..\staging\license\license-SP.txt
Name: polish; MessagesFile: compiler:Languages\Polish.isl
Name: french; MessagesFile: compiler:Languages\French.isl; InfoBeforeFile: ..\staging\license\license-FR.txt
Name: BrazilianPortuguese; MessagesFile: compiler:Languages\BrazilianPortuguese.isl; InfoBeforeFile: ..\staging\license\license-PT_BR.txt
Name: ChineseSimpl; MessagesFile: ChineseSimp-12-5.1.11.isl; InfoBeforeFile: ..\staging\license\license-CN.txt
Name: ChineseTrad; MessagesFile: ChineseTrad-2-5.1.11.isl
Name: turkish; MessagesFile: Turkish-3-5.1.11.isl; InfoBeforeFile: ..\staging\license\license-TR.txt

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
Source: ..\staging\Template.eMuleSkin.ini; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\Template.Notifier.ini; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\changelog xtreme.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\Changelog.ScarAngel.txt; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\dbghelp.dll; DestDir: {app}; Flags: ignoreversion
Source: ..\staging\eMule.exe; DestDir: {app}; Flags: ignoreversion
;official changelogs and license files are in the license subdir
Source: ..\staging\license\*; DestDir: {app}\license; Flags: ignoreversion recursesubdirs createallsubdirs
Source: ..\staging\webserver\*; DestDir: {app}\webserver; Flags: ignoreversion recursesubdirs createallsubdirs
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
;Source: ..\staging\emule\config\AC_SearchStrings.dat; DestDir: {app}\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: progsdir
Source: ..\staging\emule\config\countryflag.dll; DestDir: {app}\config; Flags: ignoreversion; tasks: progsdir
Source: ..\staging\emule\config\countryflag32.dll; DestDir: {app}\config; Flags: ignoreversion; tasks: progsdir
Source: ..\staging\emule\config\eMule Light.tmpl; DestDir: {app}\config; Flags: ignoreversion; tasks: progsdir
Source: ..\staging\emule\config\Multiuser eMule.tmpl; DestDir: {app}\config; Flags: ignoreversion; tasks: progsdir
Source: ..\staging\emule\config\eMule.tmpl; DestDir: {app}\config; Flags: ignoreversion; tasks: progsdir
Source: ..\staging\emule\config\startup.wav; DestDir: {app}\config; Flags: ignoreversion; tasks: progsdir
Source: ..\staging\emule\config\webservices.dat; DestDir: {app}\config; Flags: ignoreversion; components: configfiles; tasks: progsdir
;Source: ..\staging\emule\config\server.met; DestDir: {app}\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; tasks: progsdir
;Source: ..\staging\emule\config\addresses.dat; DestDir: {app}\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: progsdir
Source: ..\staging\emule\config\nodes.dat; DestDir: {app}\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: progsdir
;not needed, will automatically loaded on first start
;Source: ..\staging\emule\config\ip-to-country.csv; DestDir: {app}\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: progsdir
;Source: ..\staging\emule\config\ipfilter.dat; DestDir: {app}\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfilesipf; tasks: progsdir
Source: ..\staging\emule\config\staticservers.dat; DestDir: {app}\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: progsdir
; to assing correct rights:
Source: ..\staging\emule\config\preferences.ini; DestDir: {app}\config; Flags: ignoreversion onlyifdoesntexist uninsneveruninstall; Permissions: users-modify; tasks: progsdir

;appdir
Source: ..\staging\emule\config\AC_ServerMetURLs.dat; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: commonapp
;Source: ..\staging\emule\config\AC_SearchStrings.dat; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: commonapp
Source: ..\staging\emule\config\countryflag.dll; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion; tasks: commonapp
Source: ..\staging\emule\config\countryflag32.dll; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion; tasks: commonapp
Source: ..\staging\emule\config\eMule Light.tmpl; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion; tasks: commonapp
Source: ..\staging\emule\config\Multiuser eMule.tmpl; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion; tasks: commonapp
Source: ..\staging\emule\config\eMule.tmpl; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion; tasks: commonapp
Source: ..\staging\emule\config\startup.wav; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion; tasks: commonapp
Source: ..\staging\emule\config\webservices.dat; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion; components: configfiles; tasks: commonapp
;Source: ..\staging\emule\config\server.met; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; tasks: commonapp
;Source: ..\staging\emule\config\addresses.dat; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: commonapp
Source: ..\staging\emule\config\nodes.dat; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: commonapp
;not needed, will automatically loaded on first start
;Source: ..\staging\emule\config\ip-to-country.csv; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: commonapp
;Source: ..\staging\emule\config\ipfilter.dat; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfilesipf; tasks: commonapp
Source: ..\staging\emule\config\staticservers.dat; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: commonapp
; to assing correct rights:
Source: ..\staging\emule\config\preferences.ini; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist uninsneveruninstall; Permissions: users-modify; tasks: commonapp

;mydocuments   xp
Source: ..\staging\emule\config\AC_ServerMetURLs.dat; DestDir: {userappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; OnlyBelowVersion: 0,6
;Source: ..\staging\emule\config\AC_SearchStrings.dat; DestDir: {userappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\countryflag.dll; DestDir: {userappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\countryflag32.dll; DestDir: {userappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\eMule Light.tmpl; DestDir: {userappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\Multiuser eMule.tmpl; DestDir: {userappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\eMule.tmpl; DestDir: {userappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\startup.wav; DestDir: {userappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\webservices.dat; DestDir: {userappdata}\eMule\config; Flags: ignoreversion; components: configfiles; tasks: userdata; OnlyBelowVersion: 0,6
;Source: ..\staging\emule\config\server.met; DestDir: {userappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; tasks: userdata; OnlyBelowVersion: 0,6
;Source: ..\staging\emule\config\addresses.dat; DestDir: {userappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\nodes.dat; DestDir: {commonappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; OnlyBelowVersion: 0,6
;not needed, will automatically loaded on first start
;Source: ..\staging\emule\config\ip-to-country.csv; DestDir: {userappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; OnlyBelowVersion: 0,6
;Source: ..\staging\emule\config\ipfilter.dat; DestDir: {userappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfilesipf; tasks: userdata; OnlyBelowVersion: 0,6
Source: ..\staging\emule\config\staticservers.dat; DestDir: {userappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; OnlyBelowVersion: 0,6


;mydocuments   vista
Source: ..\staging\emule\config\AC_ServerMetURLs.dat; DestDir: {localappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; minversion: 0,6
;Source: ..\staging\emule\config\AC_SearchStrings.dat; DestDir: {localappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\countryflag.dll; DestDir: {localappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\countryflag32.dll; DestDir: {localappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\eMule Light.tmpl; DestDir: {localappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\Multiuser eMule.tmpl; DestDir: {localappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\eMule.tmpl; DestDir: {localappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\startup.wav; DestDir: {localappdata}\eMule\config; Flags: ignoreversion; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\webservices.dat; DestDir: {localappdata}\eMule\config; Flags: ignoreversion; components: configfiles; tasks: userdata; minversion: 0,6
;Source: ..\staging\emule\config\server.met; DestDir: {localappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; tasks: userdata; minversion: 0,6
;Source: ..\staging\emule\config\addresses.dat; DestDir: {localappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\nodes.dat; DestDir: {localappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; minversion: 0,6
;not needed, will automatically loaded on first start
;Source: ..\staging\emule\config\ip-to-country.csv; DestDir: {localappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; minversion: 0,6
;Source: ..\staging\emule\config\ipfilter.dat; DestDir: {localappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfilesipf; tasks: userdata; minversion: 0,6
Source: ..\staging\emule\config\staticservers.dat; DestDir: {localappdata}\eMule\config; Flags: ignoreversion onlyifdoesntexist; Permissions: users-modify; components: configfiles; tasks: userdata; minversion: 0,6


; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Components]
Name: Main; Description: {cm:corefiles}; Types: full compact custom; FLAGS: fixed
Name: helpfiles; Description: {cm:helpfile}; Types: full
Name: langs; Description: {cm:languagesupport}; Types: full
Name: shellextention; Description: {cm:shellextention}; Types: full
Name: configfiles; Description: {cm:configfiles}; Types: full
;Name: configfilesipf; Description: {cm:configfilesipf}
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
Filename: {app}\_eMule_Support__german_english.url; Section: InternetShortcut; Key: URL; String: http://www.emule-web.de/board
Filename: {app}\_eMule_Support__german_english.url; Section: InternetShortcut; Key: IconFile; String: http://www.emule-web.de/favicon.ico
Filename: {app}\_eMule-MODs__Infos_Ratings_Downloads.url; Section: InternetShortcut; Key: URL; String: http://www.emule-mods.de/
Filename: {app}\_Fake_Free__Server-List_and_MET.url; Section: InternetShortcut; Key: URL; String: http://www.server-met.de/
Filename: {app}\_get_updated_IP-Filter.url; Section: InternetShortcut; Key: URL; String: http://www.emule-mods.de/?mods=ipfilter
Filename: {app}\_get_updated_nodes-dat.url; Section: InternetShortcut; Key: URL; String: http://www.emule-mods.de/?mods=ipfilter
Filename: {app}\emulescarhome.url; Section: InternetShortcut; Key: URL; String: http://scarangel.sourceforge.net/

[Icons]
Name: {group}\eMule ScarAngel; Filename: {app}\emule.exe; Comment: eMule ScarAngel
Name: {group}\{cm:ProgramOnTheWeb,eMule ScarAngel}; Filename: {app}\emulescarhome.url
Name: {group}\eMule_Support__german_english; Filename: {app}\_eMule_Support__german_english.url
Name: {group}\eMule-MODs__Infos_Ratings_Downloads; Filename: {app}\_eMule-MODs__Infos_Ratings_Downloads.url
Name: {group}\Fake_Free__Server-List_and_MET; Filename: {app}\_Fake_Free__Server-List_and_MET.url
Name: {group}\get_updated_IP-Filter; Filename: {app}\_get_updated_IP-Filter.url
Name: {group}\get_updated_nodes-dat; Filename: {app}\_get_updated_nodes-dat.url
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
Filename: netsh; Parameters: "firewall add allowedprogram ""{app}\emule.exe"" ""eMule ScarAngel"" ENABLE"; flags: runminimized shellexec; Tasks: firewall; OnlyBelowVersion: 0,6
Filename: netsh; Parameters: "advfirewall firewall add rule name=""eMule ScarAngel"" dir=in action=allow program=""{app}\emule.exe"" enable=yes description=""eMule ScarAngel inbound traffic"" profile=any"; flags: runminimized shellexec; Tasks: firewall; MinVersion: 0,6
Filename: netsh; Parameters: "advfirewall firewall add rule name=""eMule ScarAngel"" dir=out action=allow program=""{app}\emule.exe"" enable=yes description=""eMule ScarAngel outbound traffic"" profile=any"; flags: runminimized shellexec; Tasks: firewall; MinVersion: 0,6

[UninstallRun]
Filename: {app}\emule.exe; Parameters: uninstall
Filename: netsh; Parameters: "firewall delete allowedprogram ""{app}\emule.exe"""; flags: runminimized shellexec; Tasks: firewall; OnlyBelowVersion: 0,6
Filename: netsh; Parameters: "advfirewall firewall delete rule name=""eMule ScarAngel"" profile=any program=""{app}\emule.exe"""; flags: runminimized shellexec; Tasks: firewall; MinVersion: 0,6


[UninstallDelete]
Type: files; Name: {app}\emulescarhome.url
Type: files; Name: {app}\_eMule_Support__german_english.url
Type: files; Name: {app}\_eMule-MODs__Infos_Ratings_Downloads.url
Type: files; Name: {app}\_Fake_Free__Server-List_and_MET.url
Type: files; Name: {app}\_get_updated_IP-Filter.url
Type: files; Name: {app}\_get_updated_nodes-dat.url
Type: files; Name: {app}\antiLeech.dll

[CustomMessages]
; This section specifies phrazes and words not specified in the ISL files
; Avoid customizing the ISL files since they will change with each version of Inno Setup.
; English
;dutch.tasks_firewall=Voeg een uitzonderings regel toe aan de windows firewall.
;dutch.dialog_firewall=Setup heeft emule niet als uitzondering aan de Windows Firewall kunnen toevoegen .%nWellicht moet u dit nog handmatig doen.
;dutch.assocurl=eD2K links doorsturen naar eMule.


#include "english.isl"

;translations in sepeate file now 10.1

#include "italian.isl"
#include "spanish.isl"
#include "german.isl"
#include "portugesebr.isl"
#include "french.isl"
;codepage:
#include "polish.isl"
;codepage:
#include "emulechinese.iss"
;codepage 950:
#include "cn_tw.isl"
;codepage:
#include "turkish.isl"

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
