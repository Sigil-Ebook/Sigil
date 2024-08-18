; We use CMake's configure_file command to replace ${VAR_NAME} variables
; with actual values. Note the dollar sign; {VAR_NAME} variables are from
; Inno, the ones with the dollar we define with CMake.

#define AppName "Sigil"

[Setup]
AppName={#AppName}${LEGACY_BUILD}
AppVerName={#AppName}${LEGACY_BUILD} ${SIGIL_FULL_VERSION}
AppVersion=${SIGIL_FULL_VERSION}
VersionInfoVersion=${SIGIL_FULL_VERSION}
DefaultDirName={autopf}\{#AppName}${LEGACY_BUILD}
DisableDirPage=no
AllowNoIcons=yes
DefaultGroupName={#AppName}${LEGACY_BUILD}
UninstallDisplayIcon={app}\{#AppName}.exe
AppPublisher=Sigil-Ebook
AppPublisherURL=https://github.com/Sigil-Ebook/Sigil
Compression=${INNO_COMP}
;SolidCompression=yes
OutputDir=..\installer
LicenseFile=${LICENSE_LOCATION}
; Lowest supported windowsversion
MinVersion=${WIN_MIN_VERSION}
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=commandline dialog
OutputBaseFilename={#AppName}-${SIGIL_FULL_VERSION}-Windows${LEGACY_BUILD}${ISS_SETUP_FILENAME_PLATFORM}-Setup
ChangesAssociations=yes
;SetupLogging=yes

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
Source: "{#AppName}\*"; DestDir: "{app}"; Flags: createallsubdirs recursesubdirs ignoreversion

[Components]
; Main files cannot be unchecked. Doesn't do anything, just here for show
Name: main; Description: "{#AppName}"; Types: full compact custom; Flags: fixed
; Desktop icon.
Name: dicon; Description: "Create a desktop icon"; Types: full custom
; File associations
Name: afiles; Description: "Associate ebook files with {#AppName}"
Name: afiles\epub; Description: "EPUB"

[Registry]
; Add Sigil as a global file handler for EPUB and HTML.
; HKLM if admin, HKCU if not
Root: HKA; Subkey: "Software\Classes\.epub\OpenWithList\{#AppName}.exe"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\.htm\OpenWithList\{#AppName}.exe"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\.html\OpenWithList\{#AppName}.exe"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\.xhtml\OpenWithList\{#AppName}.exe"; Flags: uninsdeletekey

; Associate EPUB files if requested.
; HKLM if admin, HKCU if not
Components: afiles\epub; Root: HKA; Subkey: "Software\Classes\.epub"; ValueType: string; ValueName: ""; ValueData: "{#AppName}EPUB"; Flags: uninsdeletevalue uninsdeletekeyifempty
Components: afiles\epub; Root: HKA; Subkey: "Software\Classes\{#AppName}EPUB"; ValueType: string; ValueName: ""; ValueData: "EPUB"; Flags: uninsdeletekey
Components: afiles\epub; Root: HKA; Subkey: "Software\Classes\.epub\OpenWithProgids"; ValueType: string; ValueName: "{#AppName}EPUB"; ValueData: ""; Flags: uninsdeletevalue uninsdeletekeyifempty
Components: afiles\epub; Root: HKA; Subkey: "Software\Classes\{#AppName}EPUB\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#AppName}.exe,0"; Flags: uninsdeletekey
Components: afiles\epub; Root: HKA; Subkey: "Software\Classes\{#AppName}EPUB\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#AppName}.exe"" ""%1"""; Flags: uninsdeletekey

Root: HKA; Subkey: "Software\Classes\Applications\{#AppName}.exe"; ValueType: string; ValueName: "FriendlyAppName"; ValueData: "{#AppName}: a cross-platform EPUB editor"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\Applications\{#AppName}.exe\SupportedTypes"; ValueType: string; ValueName: ".epub"; ValueData: ""
Root: HKA; Subkey: "Software\Classes\Applications\{#AppName}.exe\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#AppName}.exe,0"
Root: HKA; Subkey: "Software\Classes\Applications\{#AppName}.exe\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#AppName}.exe"" ""%1"""

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppName}.exe"
Name: "{group}\Uninstall {#AppName}${LEGACY_BUILD}"; Filename: "{uninstallexe}"
; Optional desktop icon.
; commondesktop if admin, userdesktop if not
Components: dicon; Name: "{autodesktop}\{#AppName}${LEGACY_BUILD}"; Filename: "{app}\{#AppName}.exe"
