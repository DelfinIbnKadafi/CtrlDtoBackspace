; Inno Setup 6.3 or newer
; Build the exe first (see README), then compile this file.

#define MyAppName "CtrlDtoBackspace"
#define MyAppVersion "1.0.0"
#define MyAppExe "CtrlDtoBackspace.exe"

[Setup]
AppId={{B7D0A3C4-5E21-4F6A-9C3D-7A1E2F8B4D65}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppName}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
DisableWelcomePage=no
DisableDirPage=no
LicenseFile=..\LICENSE
SetupIconFile=..\resources\app.ico
UninstallDisplayIcon={app}\{#MyAppExe}
WizardStyle=modern
WizardImageFile=..\resources\wizard.bmp
WizardSmallImageFile=..\resources\wizard_small.bmp
OutputDir=..\dist
OutputBaseFilename=CtrlDtoBackspace-Setup
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
; admin only for Program Files, user can choose "only me"
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=dialog

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Messages]
WelcomeLabel1=Welcome to CtrlDtoBackspace Setup
WelcomeLabel2=This application allows you to use CTRL + D as a replacement for the Backspace key.%n%nClick Next to continue.
FinishedHeadingLabel=Setup Complete
FinishedLabel=CtrlDtoBackspace has been successfully installed.
FinishedLabelNoIcons=CtrlDtoBackspace has been successfully installed.

[Tasks]
Name: "startup"; Description: "Start {#MyAppName} when Windows starts"; GroupDescription: "Additional tasks:"
Name: "launchapp"; Description: "Launch {#MyAppName} after installation"; GroupDescription: "Additional tasks:"
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional tasks:"; Flags: unchecked

[Files]
Source: "..\build\{#MyAppExe}"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExe}"
Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExe}"; Tasks: desktopicon

[Registry]
; startup entry (current user), removed when uninstall
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "{#MyAppName}"; ValueData: """{app}\{#MyAppExe}"" --minimized"; Flags: uninsdeletevalue; Tasks: startup
; user chose no startup, remove old entry
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueName: "{#MyAppName}"; Flags: deletevalue; Tasks: not startup
; app settings, removed when uninstall
Root: HKCU; Subkey: "Software\{#MyAppName}"; Flags: uninsdeletekey

[Run]
; checkbox on the last page (show only if user pick launch task)
Filename: "{app}\{#MyAppExe}"; Description: "Launch {#MyAppName}"; Flags: nowait postinstall skipifsilent runasoriginaluser; Tasks: launchapp

[Code]
var
  ConsentPage: TWizardPage;
  ConsentCheck: TNewCheckBox;

// next button only work when user check the box
procedure ConsentCheckClick(Sender: TObject);
begin
  WizardForm.NextButton.Enabled := ConsentCheck.Checked;
end;

procedure InitializeWizard;
var
  Memo: TNewMemo;
begin
  ConsentPage := CreateCustomPage(wpLicense, 'Information and Consent', 'Please read how this application works.');

  Memo := TNewMemo.Create(ConsentPage);
  Memo.Parent := ConsentPage.Surface;
  Memo.Left := 0;
  Memo.Top := 0;
  Memo.Width := ConsentPage.SurfaceWidth;
  Memo.Height := ConsentPage.SurfaceHeight - ScaleY(34);
  Memo.ReadOnly := True;
  Memo.WordWrap := True;
  Memo.ScrollBars := ssVertical;
  Memo.Text :=
    'CtrlDtoBackspace lets you press CTRL + D instead of the Backspace key.' + #13#10 + #13#10 +
    'Please know:' + #13#10 +
    '- The application installs a global low-level keyboard hook.' + #13#10 +
    '- CTRL + D will be intercepted and replaced with Backspace.' + #13#10 +
    '- The application can start automatically with Windows.' + #13#10 +
    '- The application runs in the background (system tray).' + #13#10 +
    '- The application does not collect or transmit personal data.' + #13#10 +
    '- The application does not require internet access.' + #13#10 + #13#10 +
    'The hook only checks for CTRL + D. Typed text is never saved or sent anywhere.';

  ConsentCheck := TNewCheckBox.Create(ConsentPage);
  ConsentCheck.Parent := ConsentPage.Surface;
  ConsentCheck.Left := 0;
  ConsentCheck.Top := Memo.Height + ScaleY(10);
  ConsentCheck.Width := ConsentPage.SurfaceWidth;
  ConsentCheck.Caption := 'I understand and I consent to the above.';
  ConsentCheck.OnClick := @ConsentCheckClick;
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = ConsentPage.ID then begin
    WizardForm.NextButton.Enabled := ConsentCheck.Checked;
  end;
end;

// close old running app, so files can be replaced or removed
procedure StopRunningApp;
var
  ExePath: String;
  ResultCode: Integer;
begin
  ExePath := ExpandConstant('{app}\{#MyAppExe}');
  if FileExists(ExePath) then begin
    Exec(ExePath, '--quit', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
    Sleep(800);
  end;
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  StopRunningApp;
  Result := '';
end;

function InitializeUninstall: Boolean;
begin
  StopRunningApp;
  Result := True;
end;
