; We use CMake's configure_file command to replace ${VAR_NAME} variables
; with actual values. Note the dollar sign; {VAR_NAME} variables are from
; Inno, the ones with the dollar we define with CMake.

[Setup]
AppName=Sigil
AppVerName=Sigil ${SIGIL_FULL_VERSION}
DefaultDirName={pf}\Sigil
DefaultGroupName=Sigil
UninstallDisplayIcon={app}\Sigil.exe
AppPublisher=Strahinja Markovic
AppPublisherURL=http://code.google.com/p/sigil/
WizardImageFile=compiler:wizmodernimage-IS.bmp
WizardSmallImageFile=compiler:wizmodernsmallimage-IS.bmp
Compression=lzma2
SolidCompression=yes  
OutputDir=..\installer
LicenseFile=${LICENSE_LOCATION}
; WinXP is the lowest supported version
MinVersion=0,5.1
PrivilegesRequired=admin
OutputBaseFilename=Sigil-${SIGIL_FULL_VERSION}-Windows${ISS_SETUP_FILENAME_PLATFORM}-Setup

; "ArchitecturesAllowed=x64" specifies that Setup cannot run on
; anything but x64.
; The ${ISS_ARCH} var is substituted with "x64" or an empty string
ArchitecturesAllowed="${ISS_ARCH}"
; "ArchitecturesInstallIn64BitMode=x64" requests that the install be
; done in "64-bit mode" on x64, meaning it should use the native
; 64-bit Program Files directory and the 64-bit view of the registry.
; The ${ISS_ARCH} var is substituted with "x64" or an empty string
ArchitecturesInstallIn64BitMode="${ISS_ARCH}"

[Files]
Source: "Sigil\*"; DestDir: "{app}"; Flags: createallsubdirs recursesubdirs

[Tasks]
Name: desktopicon; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:" 

[Icons]
Name: "{group}\Sigil";            Filename: "{app}\Sigil.exe"
Name: "{group}\Uninstall Sigil";  Filename: "{uninstallexe}"
Name: "{commondesktop}\Sigil";    Filename: "{app}\Sigil.exe"; Tasks: desktopicon
