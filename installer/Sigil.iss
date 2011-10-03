; We use CMake's configure_file command to replace ${VAR_NAME} variables
; with actual values. Note the dollar sign; {VAR_NAME} variables are from
; Inno, the ones with the dollar we define with CMake.

[Setup]
AppName=Sigil
AppVerName=Sigil ${SIGIL_FULL_VERSION}
DefaultDirName={pf}\Sigil
DefaultGroupName=Sigil
UninstallDisplayIcon={app}\Sigil.exe
AppPublisher=John Schember
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
ChangesAssociations=yes

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

[Components]
; Main files cannot be unchecked. Doesn't do anything, just here for show
Name: main; Description: "Sigil"; Types: full compact custom; Flags: fixed
; Desktop icon.
Name: dicon; Description: "Create a desktop icon"; Types: full custom
Name: dicon\common; Description: "For all users"; Types: full custom; Flags: exclusive
Name: dicon\user; Description: "For the current user only"; Flags: exclusive
; File associations
Name: afiles; Description: "Associate ebook files with Sigil"
Name: afiles\epub; Description: "EPUB"
Name: afiles\html; Description: "HTML"
Name: afiles\txt; Description: "TXT"

[Registry]
; Add Sigil as a global file handler for EPUB and HTML.
Root: HKLM; Subkey: "Software\Classes\.epub\OpenWithList\Sigil.exe"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\Classes\.htm\OpenWithList\Sigil.exe"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\Classes\.html\OpenWithList\Sigil.exe"; Flags: uninsdeletekey
Root: HKLM; Subkey: "Software\Classes\.xhtml\OpenWithList\Sigil.exe"; Flags: uninsdeletekey
; Associate EPUB files if requested.
Components: afiles\epub; Root: HKCR; Subkey: ".epub"; ValueType: string; ValueName: ""; ValueData: "SigilEPUB"; Flags: uninsdeletevalue uninsdeletekeyifempty
Components: afiles\epub; Root: HKCR; Subkey: "SigilEPUB"; ValueType: string; ValueName: ""; ValueData: "EPUB"; Flags: uninsdeletekey
Components: afiles\epub; Root: HKCR; Subkey: "SigilEPUB\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\Sigil.exe,0"; Flags: uninsdeletekey
Components: afiles\epub; Root: HKCR; Subkey: "SigilEPUB\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\Sigil.exe"" ""%1"""; Flags: uninsdeletekey
; Associate HTML files if requested. 
Components: afiles\html; Root: HKCR; Subkey: ".html"; ValueType: string; ValueName: ""; ValueData: "SigilHTML"; Flags: uninsdeletevalue uninsdeletekeyifempty
Components: afiles\html; Root: HKCR; Subkey: "SigilHTML"; ValueType: string; ValueName: ""; ValueData: "HTML"; Flags: uninsdeletekey
Components: afiles\html; Root: HKCR; Subkey: "SigilHTML\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\Sigil.exe,0"; Flags: uninsdeletekey
Components: afiles\html; Root: HKCR; Subkey: "SigilHTML\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\Sigil.exe"" ""%1"""; Flags: uninsdeletekey
; Associate TXT files if requested.
Components: afiles\txt; Root: HKCR; Subkey: ".txt"; ValueType: string; ValueName: ""; ValueData: "SigilTXT"; Flags: uninsdeletevalue uninsdeletekeyifempty
Components: afiles\txt; Root: HKCR; Subkey: "SigilTXT"; ValueType: string; ValueName: ""; ValueData: "TXT"; Flags: uninsdeletekey
Components: afiles\txt; Root: HKCR; Subkey: "SigilTXT\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\Sigil.exe,0"; Flags: uninsdeletekey
Components: afiles\txt; Root: HKCR; Subkey: "SigilTXT\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\Sigil.exe"" ""%1"""; Flags: uninsdeletekey

[Icons]
Name: "{group}\Sigil"; Filename: "{app}\Sigil.exe"
Name: "{group}\Uninstall Sigil"; Filename: "{uninstallexe}"
; Optional desktop icon.
Components: dicon\common; Name: "{commondesktop}\Sigil"; Filename: "{app}\Sigil.exe"
Components: dicon\user; Name: "{userdesktop}\Sigil"; Filename: "{app}\Sigil.exe"
