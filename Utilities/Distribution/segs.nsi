; Segs.nsi
;
; This script is based on example1.nsi, but it remember the directory, 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; It will install Segs.nsi into a directory that the user selects,
!include "MUI.nsh"

; The name of the installer
Name "Segs"

; The file to write
OutFile "segs_install.exe"

; The default installation directory
InstallDir $PROGRAMFILES\Segs

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Segs" "Install_Dir"

!define MUI_LANGDLL_REGISTRY_ROOT "HKCU" 
!define MUI_LANGDLL_REGISTRY_KEY "Software\Segs" 
!define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"


;--------------------------------

; Pages
Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles
;--------------------------------
  !insertmacro MUI_LANGUAGE "English" ;first language is the default language
;
Function .onInit

  !insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd

Function un.onInit

  !insertmacro MUI_UNGETLANGUAGE
  
FunctionEnd
;
  LangString DESC_Section1 ${LANG_ENGLISH} "SEGS (required)."
  LangString DESC_Section2 ${LANG_ENGLISH} "Start menu shortcuts"
  LangString DESC_Section3 ${LANG_ENGLISH} "Uninstall"

;  !insertmacro MUI_FUNCTIONS_DESCRIPTION_BEGIN
;    !insertmacro MUI_DESCRIPTION_TEXT ${Section1} ${DESC_Section1}
;    !insertmacro MUI_DESCRIPTION_TEXT ${Section2} ${DESC_Section2}
;    !insertmacro MUI_DESCRIPTION_TEXT ${Section3} ${DESC_Section3}
;  !insertmacro MUI_FUNCTIONS_DESCRIPTION_END

; The stuff to install

Section !$(DESC_Section1) sec1

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "Segs.exe"
  File "calc_md5s.exe"
  File "pigg_extract.exe"
  File "*.dll"
  File "*.txt"
  File "required_digests"
  File /r tool_src
  File /r config_files
  File /r data

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\NSIS_Segs "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Segs" "DisplayName" "NSIS Segs"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Segs" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Segs" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Segs" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
SectionEnd

; Optional section (can be disabled by the user)
Section !$(DESC_Section2) Section2
  CreateDirectory "$SMPROGRAMS\Segs"
  CreateShortCut "$SMPROGRAMS\Segs\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Segs\Segs.lnk" "$INSTDIR\Segs.exe" ""
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall" Section3
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Segs"
  DeleteRegKey HKLM SOFTWARE\NSIS_Segs

  ; Remove files and uninstaller
  Delete $INSTDIR\*.*
  Delete $INSTDIR\config_files\*.*
  RMDir "$INSTDIR\config_files"

  Delete $INSTDIR\tool_src\*.*
  RMDir "$INSTDIR\tool_src"

  Delete $INSTDIR\data\*.*
  RMDir "$INSTDIR\data"

  Delete $INSTDIR\uninstall.exe
  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Segs\*.*"
  ; Remove directories used
  RMDir "$SMPROGRAMS\Segs"
  RMDir "$INSTDIR"

SectionEnd
