; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Forgotten Wars"
#define MyAppVersion "1.0"
#define MyAppPublisher "Trill.Net"
#define MyAppURL "https://trill.net/"
#define MyAppExeName "fw.exe"
#define _SrcPath AddBackslash(SourcePath) + ".."

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{7E52C819-379E-4BA4-A481-DDEBC0742D0A}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DisableProgramGroupPage=yes
LicenseFile={#_SrcPath}\LICENSE.txt
OutputDir={#_SrcPath}\Setup\
OutputBaseFilename=fwclient-setup
SetupIconFile={#_SrcPath}\abclient\abclient.ico
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64 
WizardImageFile=C:\Program Files (x86)\Inno Setup 5\WizModernImage-IS.bmp
WizardSmallImageFile={#_SrcPath}\Setup\trill-55.bmp
UninstallDisplayIcon={app}\fw.exe
; trillnet=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Bin\signtool.exe sign /a /n $qTrill.net$q /t http://timestamp.comodoca.com/authenticode /d $qFW$q $f
;SignTool=trillnet

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#_SrcPath}\bin\fw.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#_SrcPath}\bin\libeay32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#_SrcPath}\bin\ssleay32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#_SrcPath}\Setup\config.xml"; DestDir: "{app}"; DestName: "config.xml"; Flags: ignoreversion
Source: "{#_SrcPath}\Setup\AbData.pak"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#_SrcPath}\Setup\Autoload.pak"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#_SrcPath}\Setup\CoreData.pak"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#_SrcPath}\Setup\Data.pak"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#_SrcPath}\bin\d3dcompiler_47.dll"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{commonprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

