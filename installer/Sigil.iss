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
; Image filenames have changed and correct defaults will be used anyway
;WizardImageFile=compiler:wizmodernimage-IS.bmp
;WizardSmallImageFile=compiler:wizmodernsmallimage-IS.bmp
Compression=lzma2/ultra
SolidCompression=yes
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
Source: vendor\vcredist.exe; DestDir: {tmp}

[Components]
; Main files cannot be unchecked. Doesn't do anything, just here for show
Name: main; Description: "{#AppName}"; Types: full compact custom; Flags: fixed
; Desktop icon.
Name: dicon; Description: "Create a desktop icon"; Types: full custom
; File associations
Name: afiles; Description: "Associate ebook files with {#AppName}"
Name: afiles\epub; Description: "EPUB"
; Cancel runtime query/install if desired.
;Name: vcruntime; Description: "Check if bundled VS runtime install is necessary? (admin required)"; Types: full custom
Name: vcruntimeadmin; Description: "Check if bundled VS runtime install is necessary? (admin required)"; Types: full custom; Check: IsAdminInstallMode
Name: vcruntimeuser; Description: "Check if bundled VS runtime install is necessary? (admin required)"; Types: full custom; Check: not IsAdminInstallMode

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

[InstallDelete]
; Restructuring done in 0.9.8 makes this folder residual.
Type: filesandordirs; Name: "{app}\python3"
; Might be moving to precompiled python files in future.
; and to keep install directory clean for future enhancement possibilities
Type: filesandordirs; Name: "{app}\plugin_launchers\python"
Type: filesandordirs; Name: "{app}\python3lib"
Type: filesandordirs; Name: "{app}\Lib"
Type: filesandordirs; Name: "{app}\DLLs"
Type: filesandordirs; Name: "{app}\Scripts"
; Moving to standard naming of python interpreter. 
; So remove the old name if present.
Type: files; Name: "{app}\sigil-python3.exe"

[Run]
; The following command detects whether or not the vc++ runtime needs to be installed.
;Components: vcruntime; Filename: {tmp}\vcredist.exe; Check: NeedsVCRedistInstall; Parameters: "/passive /norestart /Q:a /c:""msiexec /qb /i vcredist.msi"" "; StatusMsg: Checking for VC++ RunTime ...
Components: vcruntimeadmin; Filename: {tmp}\vcredist.exe; Check: IsAdminInstallMode and NeedsVCRedistInstall; Parameters: "/passive /norestart /Q:a /c:""msiexec /qb /i vcredist.msi"" "; StatusMsg: Checking for VC++ RunTime ...
Components: vcruntimeuser; Filename: {tmp}\vcredist.exe; Check: (not IsAdminInstallMode) and NeedsVCRedistInstall; Parameters: "/passive /norestart /Q:a /c:""msiexec /qb /i vcredist.msi"" "; Flags: runasoriginaluser; StatusMsg: Checking for VC++ RunTime ...

[Code]

function CompareVersion(V1, V2: string): Integer;
// Compare version strings
// Returns 0, if the versions are equal.
// Returns -1, if the V1 is older than the V2.
// Returns 1, if the V1 is newer than the V2.
var
  P, N1, N2: Integer;
begin
  Result := 0;
  while (Result = 0) and ((V1 <> '') or (V2 <> '')) do
  begin
    P := Pos('.', V1);
    if P > 0 then
    begin
      N1 := StrToInt(Copy(V1, 1, P - 1));
      Delete(V1, 1, P);
    end
      else
    if V1 <> '' then
    begin
      N1 := StrToInt(V1);
      V1 := '';
    end
      else
    begin
      N1 := 0;
    end;

    P := Pos('.', V2);
    if P > 0 then
    begin
      N2 := StrToInt(Copy(V2, 1, P - 1));
      Delete(V2, 1, P);
    end
      else
    if V2 <> '' then
    begin
      N2 := StrToInt(V2);
      V2 := '';
    end
      else
    begin
      N2 := 0;
    end;

    if N1 < N2 then Result := -1
      else
    if N1 > N2 then Result := 1;
  end;
end;

function VCinstalled(const regKey: string): Boolean;
{ Function for Inno Setup Compiler }
{ Returns True if same or later Microsoft Visual C++ 2015 Redistributable is installed, otherwise False. }
var
  major: Cardinal;
  minor: Cardinal;
  bld: Cardinal;
  rbld: Cardinal;
  installed_ver, min_ver: String;

begin
  Result := False;
  { Mimimum version of the VC++ Redistributable needed (currently 14.26.28 and later) }
  min_ver := '14.26.28000.0';
  Log('Minimum VC++ 2015-2019 Redist version is: ' + min_ver);

  if RegQueryDWordValue(HKEY_LOCAL_MACHINE, regKey, 'Major', major) then begin
    if RegQueryDWordValue(HKEY_LOCAL_MACHINE, regKey, 'Minor', minor) then begin
      if RegQueryDWordValue(HKEY_LOCAL_MACHINE, regKey, 'Bld', bld) then begin
        if RegQueryDWordValue(HKEY_LOCAL_MACHINE, regKey, 'RBld', rbld) then begin
            installed_ver := IntToStr(major) + '.' + IntToStr(minor) + '.' + IntToStr(bld) + '.' + IntToStr(rbld);
            Log('VC++ 2015-2019 Redist version is: ' + installed_ver);
            { Version info was found. Return true if later or equal to our min_ver redistributable }
            // Note brackets required because of weird operator precendence
            //Result := (major >= 14) and (minor >= 23) and (bld >= 27820) and (rbld >= 0)
            //Log('Installed Version ' + installed_ver + ' >= Minimum Version ' + min_ver + Format(': %d', [IntToStr((CompareVersion(installed_ver, min_ver) >= 0))]));
            Result := (CompareVersion(installed_ver, min_ver) >= 0)
        end;
      end;
    end;
  end;
end;

function NeedsVCRedistInstall: Boolean;
begin
  if NOT IsWin64 then
    { 32-bit OS, 32-bit installer }
    Result := not (VCinstalled('SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\X86'))
  else if Is64BitInstallMode then
    { 64-bit OS, 64-bit installer }
    Result := not (VCinstalled('SOFTWARE\WOW6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x64'))
  else
    { 64-bit OS, 32-bit installer }
    Result := not (VCinstalled('SOFTWARE\WOW6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x86'));  
end;

(* function NeedsVCRedistInstall: Boolean;
// Return True if VC++ redist included with Sigil Installer needs to be installed.
var
  reg_key, installed_ver, min_ver: String;
  R: Integer;
begin
  Result := True;
  // Mimimum version of the VC++ Redistributable needed (currently 14.26.28 and later).
  min_ver := '14.26.28000';
  if IsWin64 and not Is64BitInstallMode then
    // 32-bit version being installed on 64-bit machine
    reg_key := 'SOFTWARE\WoW6432Node\Microsoft\DevDiv\vc\servicing\14.0\RuntimeMinimum'
  else
    reg_key := 'SOFTWARE\Microsoft\DevDiv\vc\servicing\14.0\RuntimeMinimum';

  // If there's a compatible version of the 14.XX runtime already installed; use it.
  if RegQueryStringValue(HKEY_LOCAL_MACHINE, reg_key, 'Version', installed_ver) then
    begin
      //MsgBox('Registry key: ' + reg_key, mbInformation, MB_OK);
      //MsgBox('Installed version: ' + installed_ver, mbInformation, MB_OK);
      //MsgBox('Minimum version: ' + min_ver, mbInformation, MB_OK);
      R := CompareVersion(installed_ver, min_ver);
      // If installed VC++ runtime version is equal or newer than
      // the minimum version specified, then don't run
      // the bundled VC++ redistributable installer
      if R >= 0 then
        Result := False;
    end
 end; *)

function CmdLineParamExists(const Value: string): Boolean;
var
  I: Integer;
begin
  Result := False;
  for I := 1 to ParamCount do
    if CompareText(Uppercase(ParamStr(I)), Value) = 0 then
    begin
      Result := True;
      Exit;
    end;
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = wpSelectComponents then
    if (not IsAdminInstallMode) and ((not CmdLineParamExists('/SILENT')) and (not CmdLineParamExists('/VERYSILENT'))) then
    begin
      // Runtime query/install component unchecked by default
      // in User mode installs. Checked in Admin installs.
      WizardForm.ComponentsList.Checked[4] := False;
      //WizardForm.ComponentsList.ItemEnabled[4] := False;
    end;
end;

// Warn when unchecking component to check for, and install
// if necessary, the Visual Studio runtime distributable.
// Use /SUPPRESSMSGBOXES in conjunction with /SILENT or
// /VERYSILENT on the command line to suppress this warning.
function NextButtonClick(CurPageID: Integer): Boolean;
var
  msg: String;
begin
  Result := True;
  msg := 'The option to check for/install the VS' + #13#10 +
        'runtime is unchecked. Please make sure a' + #13#10 +
        'compatible version of the Visual Studio' + #13#10 +
        'VC++ runtime is already installed (by you' + #13#10 +
        'or an admin), or click "No" and check' + #13#10 +
        'the box before proceeding.' + #13#10 + #13#10 +
        'You will need admin privileges to' + #13#10 +
        'to install the runtime.' + #13#10 + #13#10 +
        'Do you wish to proceed as is?';
  if CurPageID = wpSelectComponents then begin
    if IsAdminInstallMode then begin
      if (not WizardIsComponentSelected('vcruntimeadmin')) then
        Result := SuppressibleMsgBox(msg, mbInformation, MB_YESNO, IDYES) = IDYES
    end else
      if (not WizardIsComponentSelected('vcruntimeuser')) then
        Result := SuppressibleMsgBox(msg, mbInformation, MB_YESNO, IDYES) = IDYES;
  end;
end;


// Old version of above.
(* function NextButtonClick(CurPageID: Integer): Boolean ;
begin
  Result := True;
  if CurPageID = wpSelectComponents then
  begin
    if (not WizardIsComponentSelected('vcruntime')) and (not IsAdminInstallMode)  then
      Result := MsgBox('When installing for the current user only, you are' + #13#10 +
        'responsible for insuring that the proper Visual Studio' + #13#10 +
        'runtime distributable is installed.' + #13#10 + #13#10 +
        'Do you wish to continue?' , mbInformation, MB_YESNO) = IDYES;
  end;
end; *)
