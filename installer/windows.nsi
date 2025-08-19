; NSIS installer script for ultraDYN VST3
; Usage: makensis /DVERSION=1.2.3 /DVST3SOURCE=dist/windows /DPRODUCT_NAME=ultraDYN installer/windows.nsi

!define PRODUCT_NAME "${PRODUCT_NAME}"
!define PRODUCT_VERSION "${VERSION}"
!define VST3_SOURCE "${VST3SOURCE}"

OutFile "${PRODUCT_NAME}-${PRODUCT_VERSION}-Windows-Setup.exe"
InstallDir "$PROGRAMFILES64\${PRODUCT_NAME}"
RequestExecutionLevel admin
Unicode true

!include "MUI2.nsh"

!define MUI_ABORTWARNING
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section "Install VST3"
  SetOutPath "$INSTDIR"
  CreateDirectory "$COMMONFILES64\VST3"
  File /r "${VST3_SOURCE}\*.vst3"
  CopyFiles /SILENT "$INSTDIR\*.vst3" "$COMMONFILES64\VST3\"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Uninstall"
  Delete "$COMMONFILES64\VST3\${PRODUCT_NAME}.vst3"
  RMDir /r "$INSTDIR"
SectionEnd

