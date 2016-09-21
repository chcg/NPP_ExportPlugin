#pragma once

#include "Docking.h"
#include "dockingResource.h"
#include "Notepad_plus_msgs.h"
#include "PluginInterface.h"
#include <stdio.h>

#include "resource.h"
#include "shlwapi.h"
#include "commctrl.h"
#include "commdlg.h"

#include "ExportStructs.h"

#define nbFunc	5

//Global variables
HINSTANCE hDLL;							//Handle of the DLL instance
bool initializedPlugin;					//Boolean that used used to track whether the plugin is has been initialized or not
NppData nppData;						//Structure that defines the Notepad++ instance that uses the plugin
FuncItem funcItem[nbFunc];				//Array that specifies the plugins menu commands

TCHAR * dllPath, * dllName, * pluginName, * iniFile;		//String pointers that specify the name and path of the plugin DLL and the path to the settings file

CurrentScintillaData mainCSD;

//Forward function declarations
BOOL APIENTRY DllMain(HANDLE hModule,DWORD ul_reason_for_call,LPVOID lpReserved);

extern "C" __declspec(dllexport) void setInfo(NppData notepadPlusData);
extern "C" __declspec(dllexport) const TCHAR * getName();
extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF);
extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode);
extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam);

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode();
#endif //UNICODE

HWND getCurrentHScintilla(int which);

void initializePlugin();
void deinitializePlugin();

void readSettings();
void writeSettings();

void createWindows();
void destroyWindows();

void doExportRTF();
void doExportHTML();
void doClipboardRTF();
void doClipboardHTML();
void doClipboardAll();

void initScintillaData(CurrentScintillaData * csd);
void fillScintillaData(CurrentScintillaData * csd, int start, int end);
void deinitScintillaData(CurrentScintillaData * csd);

BOOL saveFile(TCHAR * filebuffer, int buffersize, const TCHAR * filters);

void exportHTML(bool isClipboard, HANDLE exportFile);
void exportRTF(bool isClipboard, HANDLE exportFile);
void exportTXT(bool isClipboard, HANDLE exportFile);

void err(const TCHAR * msg) {
	MessageBox(nppData._nppHandle, msg, TEXT("Error"), MB_OK);
}