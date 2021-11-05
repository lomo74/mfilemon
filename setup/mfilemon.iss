; MFILEMON - print to file with automatic filename assignment
; Copyright (C) 2007-2021 Lorenzo Monti
;
; This program is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 3
; of the License, or (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#define SrcApp "..\win32\release\mfilemon.dll"
#define FileVerStr GetVersionNumbersString(SrcApp)
#define StripBuild(str VerStr) Copy(VerStr, 1, RPos(".", VerStr) - 1)
#define AppVerStr StripBuild(FileVerStr)
#define AppName "Multi file port monitor (mfilemon)"

[Setup]
AppId={{A932243F-381F-434C-B18E-4F09D2F015F8}
AppName={#AppName}
AppVersion={#AppVerStr}
AppVerName={#AppName} {#AppVerStr}
AppPublisher=Lorenzo Monti
AppPublisherURL=http://mfilemon.sourceforge.net/
AppSupportURL=http://mfilemon.sourceforge.net/
AppUpdatesURL=http://mfilemon.sourceforge.net/
UninstallDisplayName={#AppName} {#AppVerStr}
VersionInfoCompany=Lorenzo Monti
VersionInfoCopyright=Copyright © 2007-2021 Lorenzo Monti
VersionInfoDescription={#AppName} setup program
VersionInfoProductName={#AppName}
VersionInfoVersion={#FileVerStr}

CreateAppDir=yes
DefaultDirName={autopf}\mfilemon
DefaultGroupName=Multi File Port Monitor
DisableWelcomePage=no

; we take care of these on our own
CloseApplications=no
RestartApplications=no

OutputBaseFilename=mfilemon-setup
Compression=lzma
SolidCompression=yes
ArchitecturesAllowed=x86 x64
ArchitecturesInstallIn64BitMode=x64
MinVersion=0,6.1sp1

LicenseFile=gpl-3.0.rtf

SignTool=lorenzomonti /d "{#AppName}"

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "it"; MessagesFile: "compiler:Languages\Italian.isl"

[CustomMessages]
en.errRegister=Error in port monitor registration!
en.errUnregister=Error in port monitor unregistration! Continue with removal anyway?
en.stoppingSpooler=Stopping spooler...
en.startingSpooler=Starting spooler...

it.errRegister=Errore nella registrazione del port monitor!
it.errUnregister=Errore nella deregistrazione del port monitor! Continuare ugualmente con la rimozione?
it.stoppingSpooler=Arresto dello spooler...
it.startingSpooler=Avvio dello spooler...

[Files]
; x64 files
Source: "..\x64\release\mfilemon.dll"; DestDir: "{sys}"; Flags: promptifolder replacesameversion; Languages: en; Check: Is_x64
Source: "..\x64\release\mfilemonUI.dll"; DestDir: "{sys}"; Flags: promptifolder replacesameversion; Languages: en; Check: Is_x64
Source: "..\x64\release-ita\mfilemon.dll"; DestDir: "{sys}"; Flags: promptifolder replacesameversion; Languages: it; Check: Is_x64
Source: "..\x64\release-ita\mfilemonUI.dll"; DestDir: "{sys}"; Flags: promptifolder replacesameversion; Languages: it; Check: Is_x64
; x86 files
Source: "..\win32\release\mfilemon.dll"; DestDir: "{sys}"; Flags: promptifolder replacesameversion; Languages: en; Check: Is_x86
Source: "..\win32\release\mfilemonUI.dll"; DestDir: "{sys}"; Flags: promptifolder replacesameversion; Languages: en; Check: Is_x86
Source: "..\win32\release-ita\mfilemon.dll"; DestDir: "{sys}"; Flags: promptifolder replacesameversion; Languages: it; Check: Is_x86
Source: "..\win32\release-ita\mfilemonUI.dll"; DestDir: "{sys}"; Flags: promptifolder replacesameversion; Languages: it; Check: Is_x86
; files common to either architectures
Source: "..\misc\docs\ghostscript-mfilemon-howto.html"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\misc\docs\images\*"; DestDir: "{app}\images"; Flags: ignoreversion
Source: "release\setuphlp.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\misc\conf\*"; DestDir: "{app}\conf"; Flags: ignoreversion

[Icons]
Name: "{group}\ghostscript-mfilemon howto"; Filename: "{app}\ghostscript-mfilemon-howto.html"; WorkingDir: "{app}";

[Code]
var
  bIsAnUpdate: Boolean;
  bDeleteMonOk: Boolean;

{----------------------------------------------------------------------------------------}
function RegisterMonitor: LongBool;
external 'RegisterMonitor@files:setuphlp.dll stdcall setuponly';

function UnregisterMonitor: LongBool;
external 'UnregisterMonitor@{app}\setuphlp.dll stdcall uninstallonly';

function IsMonitorRegistered: LongBool;
external 'IsMonitorRegistered@files:setuphlp.dll stdcall setuponly';

function IsMonitorRegisteredUninstall: LongBool;
external 'IsMonitorRegistered@{app}\setuphlp.dll stdcall uninstallonly';

{----------------------------------------------------------------------------------------}
function Is_x86: Boolean;
begin
  Result := (ProcessorArchitecture = paX86);
end;

{----------------------------------------------------------------------------------------}
function Is_x64: Boolean;
begin
  Result := (ProcessorArchitecture = paX64);
end;

{----------------------------------------------------------------------------------------}
function IsWizardFormCreated: Boolean;
var
  frm: TWizardForm;
begin
  Result := True;
  try
    frm := WizardForm;
  except
    Result := False;
  end;
end;

{----------------------------------------------------------------------------------------}
procedure StopSpooler;
var
  res: Integer;
begin
  { Avoid Internal error: An attempt was made to access WizardForm before it has been created.}
  if IsWizardFormCreated then
    WizardForm.StatusLabel.Caption := ExpandConstant('{cm:stoppingSpooler}');
  Exec(ExpandConstant('{sys}\net.exe'), 'stop Spooler', '', SW_HIDE, ewWaitUntilTerminated, res);
end;

{----------------------------------------------------------------------------------------}
procedure StartSpooler;
var
  res: Integer;
begin
  { Avoid Internal error: An attempt was made to access WizardForm before it has been created. }
  if IsWizardFormCreated then
    WizardForm.StatusLabel.Caption := ExpandConstant('{cm:startingSpooler}');
  Exec(ExpandConstant('{sys}\net.exe'), 'start Spooler', '', SW_HIDE, ewWaitUntilTerminated, res);
end;

{----------------------------------------------------------------------------------------}
function DestinationFilesExist: Boolean;
begin
  Result := FileExists(ExpandConstant('{sys}\mfilemon.dll')) and
            FileExists(ExpandConstant('{sys}\mfilemonUI.dll'));
end;

{----------------------------------------------------------------------------------------}
function InitializeSetup: Boolean;
begin
  bIsAnUpdate := DestinationFilesExist;
  Result := True;
end;

{----------------------------------------------------------------------------------------}
procedure CurStepChanged(CurStep: TSetupStep);
begin
  case CurStep of
    ssInstall:
      begin
        if bIsAnUpdate then begin
          Log('CODE: Stopping printer spooler.');
          StopSpooler;
        end;
      end;
    ssPostInstall:
      begin
        if bIsAnUpdate then begin
          Log('CODE: Starting printer spooler.');
          StartSpooler;
          if not IsMonitorRegistered then begin
            Log('CODE: Monitor is not registered.');
            if not RegisterMonitor then begin
              Log('CODE: Could not register monitor.');
              MsgBox(CustomMessage('errRegister'), mbError, MB_OK);
            end;
          end;
        end else begin
          if not RegisterMonitor then begin
            Log('CODE: Could not register monitor.');
            StopSpooler;
            StartSpooler;
            if not IsMonitorRegistered then begin
              if not RegisterMonitor then begin
                Log('CODE: Could not register monitor, second try.');
                MsgBox(CustomMessage('errRegister'), mbError, MB_OK);
              end;
            end;
          end;
        end;
      end;
  end;
end;

{----------------------------------------------------------------------------------------}
function InitializeUninstall: Boolean;
begin
  bDeleteMonOk := UnregisterMonitor;
  if bDeleteMonOk then
    Result := True
  else begin
    Log('CODE: Could not unregister monitor.');
    if IsMonitorRegisteredUninstall then begin
      StopSpooler;
      StartSpooler;
      bDeleteMonOk := UnregisterMonitor;
      if bDeleteMonOk then begin
        Log('CODE: Could unregister monitor, second try.');
        Result := True;
      end else begin
        Log('CODE: Could not unregister monitor, second try.');
        if MsgBox(CustomMessage('errUnregister'), mbError, MB_YESNO) = IDYES then
          Result := True
        else
          Result := False;
      end;
    end else begin
      Log('CODE: Monitor not found, unregister ok.');
      Result := True;
    end;
  end;
end;

{----------------------------------------------------------------------------------------}
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  case CurUninstallStep of
    usUninstall:
      begin
        if not bDeleteMonOk then begin
          Log('CODE: stop spooler.');
          StopSpooler;
        end;
      end;
    usPostUninstall:
      begin
        if not bDeleteMonOk then begin
          Log('CODE: start spooler.');
          StartSpooler;
        end;
      end;
  end;
end;
