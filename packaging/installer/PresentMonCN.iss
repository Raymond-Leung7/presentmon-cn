#ifndef SourceDir
  #error SourceDir is required
#endif

#ifndef OutputDir
  #error OutputDir is required
#endif

#ifndef AppVersion
  #error AppVersion is required
#endif

#define MyAppName "PresentMon CN"
#define MyAppPublisher "Raymond-Leung7"
#define MyAppExeName "PresentMon-CN.exe"
#define MyAppId "{{4FB635A7-C10B-4B7E-9E9D-0D1D60A4D365}"

[Setup]
AppId={#MyAppId}
AppName={#MyAppName}
AppVersion={#AppVersion}
AppVerName={#MyAppName} {#AppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL=https://github.com/Raymond-Leung7/presentmon-cn
AppSupportURL=https://github.com/Raymond-Leung7/presentmon-cn/issues
AppUpdatesURL=https://github.com/Raymond-Leung7/presentmon-cn/releases
AppCopyright=Copyright (C) Raymond-Leung7 and PresentMon contributors
DefaultDirName={localappdata}\Programs\PresentMon CN
DefaultGroupName={#MyAppName}
DisableDirPage=auto
DisableProgramGroupPage=yes
UsePreviousAppDir=yes
UsePreviousGroup=yes
UsePreviousTasks=yes
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir={#OutputDir}
OutputBaseFilename=PresentMon-CN-{#AppVersion}-Setup-x64
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
ShowLanguageDialog=no
CloseApplications=yes
RestartApplications=no
RestartIfNeededByRun=no
SetupLogging=yes
Uninstallable=yes
CreateUninstallRegKey=yes
UninstallDisplayName={#MyAppName}
UninstallDisplayIcon={app}\{#MyAppExeName}
VersionInfoCompany={#MyAppPublisher}
VersionInfoDescription={#MyAppName} Setup
VersionInfoVersion={#AppVersion}
VersionInfoProductName={#MyAppName}

[Languages]
Name: "chinesesimp"; MessagesFile: "compiler:Default.isl,ChineseSimplified.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateADesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#SourceDir}\*"; DestDir: "{app}"; Excludes: "portable.mode"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; WorkingDir: "{app}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: "{app}\app\debug.log"
Type: files; Name: "{app}\app\pmui-init-log-*.txt"
Type: filesandordirs; Name: "{app}\app\cef-cache"
Type: filesandordirs; Name: "{localappdata}\Raymond-Leung7\PresentMon-CN\cef-cache"
Type: filesandordirs; Name: "{localappdata}\Raymond-Leung7\PresentMon-CN\logs"
Type: dirifempty; Name: "{localappdata}\Raymond-Leung7\PresentMon-CN"
Type: dirifempty; Name: "{localappdata}\Raymond-Leung7"
Type: dirifempty; Name: "{app}\app"
Type: dirifempty; Name: "{app}"
