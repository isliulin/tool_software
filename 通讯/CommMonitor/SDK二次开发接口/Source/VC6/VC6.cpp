// VC6.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "stdio.h" 
#include "commctrl.h" 
#include <RICHEDIT.h>
#include <commdlg.h>
#include <shellapi.h>



#define MAX_LOADSTRING 100
 


#define  COMM_OPEN    1
#define  COMM_CLOSE   2
#define  COMM_READ    3
#define  COMM_WRITE   4


#define  IDV_VERSION    "3.13"
#define  IDS_APPTITLE   "CommMonitor" IDV_VERSION "串口监视精灵(For VC6.0 DLL版)"
#define  IDS_APPINFO    "	SoftName : " IDS_APPTITLE"\r\n \
	Version  : " IDV_VERSION "\r\n \
	FileSize : 52KB\r\n \
	BuildDate: 2010-02-05 09:55:59\r\n\
	Email    : jfyes@qq.com\r\n\
	QQ       : 348677065\r\n\
	Home     : http:\/\/www.jfyes.com\r\n \
	SoftWare : http:\/\/software.jfyes.com\r\n\r\n \
	Copyright (C) 2003-2010 jfyes网络科技           \r\n"



#pragma pack(push)
#pragma pack(1) //设置为1字节对齐
typedef struct _HookData
{
    BYTE  ComPort;      //串口号  
    BYTE  CommState;    //串口状态 
    HFILE FileHandle;  //被打开的文件句柄     长整型无符号
    int   DataSize;      //数据大小             长整型有符号   
    char  Data [8192] ;   //串口数据
}THookData, *PHookData; 
#pragma pack(pop)



//DLL 实例句柄
HINSTANCE  gPMonitorComm = NULL;

//  TOnData = function (AHook: PHookData): Integer;stdcall;
typedef  LONG(CALLBACK * TOnData)(LONG lParam);

//启动串口监视
typedef BOOL(CALLBACK * TMonitorComm)(DWORD Pid, DWORD ComIndex,  TOnData lpCallFunc);
//关闭串口监视, 返BOOL类型
typedef BOOL(CALLBACK * TUnMonitorComm)(void);
//取得全部进程ID
typedef void(CALLBACK * TGetAllProcess)(HWND hComBox);


//过程变量
TMonitorComm   MonitorComm;
TUnMonitorComm UnMonitorComm;
TGetAllProcess GetAllProcess;

//////////////////////////////
LONG ReadTotal = 0;
LONG WriteTotal = 0;
HWND hMemo = NULL;
HWND hHex = NULL;
//是否是启动监视
BOOL bActive = FALSE;
HINSTANCE HInst;

BOOL MouseLButtonDown = FALSE;





LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);


LONG ColorToRGB(LONG Color)
{
  if (Color < 0) 
    return GetSysColor(Color & 0x000000FF);
  else 
    return Color;
}



void SetRichEditFontColor(LONG AColor)
{
  CHARFORMAT Format;
  //memset(Format, 0, sizeof(CHARFORMAT));
  Format.cbSize = sizeof(CHARFORMAT);
  Format.dwMask = CFM_COLOR;
  Format.crTextColor = ColorToRGB(AColor);
  SendMessage(hMemo, EM_SETCHARFORMAT, SCF_SELECTION, LPARAM(&Format));
}

void SetRichEditFontName(LPCSTR FontName)
{
  CHARFORMAT Format;
  Format.cbSize = sizeof(CHARFORMAT);
  Format.dwMask = CFM_FACE;
  memset(Format.szFaceName, 0, sizeof(Format.szFaceName));
  lstrcpy(Format.szFaceName, FontName);
  SendMessage(hMemo, EM_SETCHARFORMAT, 0, LPARAM(&Format));

}



int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.



	//加载DLL
    gPMonitorComm = LoadLibrary("PMonitorComm.dll"); 
	if (!gPMonitorComm) 
	{	  
		
	  MessageBox(GetForegroundWindow(), "加载PMonitorComm.dll失败，请检察该DLL是否存在。", "提示", MB_OK | MB_ICONERROR);
	  return 0;
	}

    //取得func地址
    MonitorComm   = (TMonitorComm)GetProcAddress(gPMonitorComm, "MonitorComm");
	UnMonitorComm = (TUnMonitorComm)GetProcAddress(gPMonitorComm, "UnMonitorComm");
	GetAllProcess = (TGetAllProcess)GetProcAddress(gPMonitorComm, "GetAllProcess");


  if (!MonitorComm)  
  {
    MessageBox(0, "MonitorComm function address无效。", "提示", MB_OK | MB_ICONERROR);
    return TRUE;
  }

  if (!UnMonitorComm)  
  {
    MessageBox(0, "UnMonitorComm function address无效。", "提示", MB_OK | MB_ICONERROR);
    return TRUE;
  }

  
  HMODULE RichEditModule = LoadLibrary("RICHED32.DLL");


  if (!GetAllProcess)  
  {
    MessageBox(0, "GetAllProcess function address无效。", "提示", MB_OK | MB_ICONERROR);
    return TRUE;
  }

    HInst = hInstance;
    return DialogBox(HInst, (LPCTSTR)IDD_ABOUTBOX, 0, (DLGPROC)About);

    //释放DLL库
	FreeLibrary(gPMonitorComm);

	if (RichEditModule) FreeLibrary(RichEditModule);
}


 

LRESULT CALLBACK InitDlg(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
  int i;
  char Number[255] = {0};

  for (i =0; i<=255;i++)
  {
     if (i==0)  lstrcpy(Number, "所有COM口");
	 else sprintf(Number, "COM%d", i);
     //添加到ComBox
     SendMessage(GetDlgItem(hDlg, IDC_COM), CB_ADDSTRING, 0, LONG(&Number));
  } // end for i 

  SendMessage(GetDlgItem(hDlg, IDC_COM), CB_SETCURSEL, 0, 0);

  if (GetAllProcess) GetAllProcess(GetDlgItem(hDlg, IDC_PID));

  SendMessage(hDlg, WM_SETICON, ICON_BIG, (LONG)LoadIcon(HInst, (LPCSTR)IDI_APP));
  SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LONG)LoadIcon(HInst, (LPCSTR)IDI_APP)); 



  //LoadString(HInst,  IDS_APPTITLE, Number, sizeof(Number));
  SetWindowText(hDlg, IDS_APPTITLE);
  SetDlgItemText(hDlg, IDC_STATE, Number);


	//hMemo = GetDlgItem(hDlg, IDC_DATAMEMO);
	hMemo = GetDlgItem(hDlg, IDC_REDTDATA);
	SetRichEditFontName("Fixedsys");

	hHex = GetDlgItem(hDlg, IDC_HEX);		 
	SendMessage(hHex, BM_SETCHECK, 1, 0);

	SetTimer(hDlg, 1, 2000, NULL);
	InsertMenu(GetSystemMenu(hDlg, FALSE), MAXWORD, MF_SEPARATOR, 0, "-");   
    InsertMenu(GetSystemMenu(hDlg, FALSE), MAXWORD, MF_BYCOMMAND, IDM_ABOUT, "关于\tJfyes");


  return TRUE;
}                              


DWORD GetPid(LPCSTR lpText)
{
  int len;
  char buf[6];

  len = lstrlen(lpText);
  for (int i=0;i<len;i++)
  {
    if (lpText[i] == 32)
	{
      buf[i] = 0x00;
	  break;
	}
	else
      buf[i]=lpText[i];
  }

  return DWORD(atol(buf)); 
}
 

int  FormatHex(HWND hHex, char *d, int len, LPSTR lpOutText)
{
    //char buf[8912*3]={0};
	char hex[4]={0};
    char h = 0;

	if (SendMessage(hHex, BM_GETCHECK, 0, 0) != BST_CHECKED)
    {
	    lstrcat(lpOutText, d);
		return len;  
	}
    

    for (int i=0; i<len; i++)
    {
	    //因为是无符号，所以要这右移4位
        int j = (d[i] >> 4) & 0xF; 
        if (j <= 9)  h = j + '0';     
        else h = j + 'A' - 10;  

        memset(hex, 0, sizeof(hex)); 
		sprintf(hex, "%C", h);
	    lstrcat(lpOutText, hex);  

        j = d[i] & 0xF;
        if (j <= 9)  h = j + '0';  
        else  h = j + 'A' - 10;  

	    sprintf(hex, "%C ", h);
	    lstrcat(lpOutText, hex); 	  
	}
    //MessageBox(0, buf, "提示", MB_OK | MB_ICONINFORMATION); 
		  
    return lstrlen(lpOutText);
}
  


int GetMemoCount(HWND hMemo)
{
	int Result = SendMessage(hMemo, EM_GETLINECOUNT, 0, 0);
    if(SendMessage(hMemo, 
		           EM_LINELENGTH, 
		           SendMessage(hMemo, EM_LINEINDEX, Result - 1, 0), 
				   0) == 0)
	  Result--;
	return Result;

 
}

void MemoPut(HWND hMemo, int Index, char *data)
{
  SendMessage(hMemo, EM_SETSEL, 666536+Index, 666536);
  SendMessage(hMemo, EM_REPLACESEL, true,  (LPARAM)data);  
}
 


LONG CALLBACK AOnData(LONG lParam)  //THookData* p
{ 
  PHookData p = NULL;
  p = PHookData(lParam);

 
 
  if (p) 
  {
     char buf[255] = {0};
    char data[8912*3+64] = {0};
    if (p->CommState == COMM_OPEN){
	  sprintf(buf, "打开 COM%d \r\n",  p->ComPort);
	  }
	else if (p->CommState == COMM_CLOSE){
	  sprintf(buf, "关闭 COM%d \r\n",  p->ComPort);
	  }
	else if ((p->CommState == COMM_READ) || (p->CommState == COMM_WRITE))
	{
	  if (p->CommState == COMM_READ) {
	     sprintf(buf, "COM%d, Read: %d(Bytes) ", p->ComPort, p->DataSize);
         ReadTotal += p->DataSize;
		  SetRichEditFontColor(0xFF0000);
	  }
	  else { 
		 sprintf(buf, "COM%u, Write: %d(Bytes) ", p->ComPort, p->DataSize);
         WriteTotal += p->DataSize;
		 SetRichEditFontColor(0x008000);
	  }
      // MessageBox(0, p->Data, "提示", MB_OK | MB_ICONINFORMATION); 

      // sprintf(data, "%s", buf);
	  lstrcat(data, buf); 
	  FormatHex(hHex, p->Data, p->DataSize, data);
      lstrcat(data, "\r\n"); 

	   
      MemoPut(hMemo, GetMemoCount(hMemo)+1, data);

      //更新状态
	  GetDlgItemText(GetParent(hMemo), IDC_PID, buf, sizeof(buf));
      memset(data, 0, sizeof(data));
      sprintf(data, "%s, WriteTotal: %u(Bytes)  |  ReadTotal: %u(Bytes)",  buf, WriteTotal, ReadTotal);
      SetDlgItemText(GetParent(hMemo), IDC_STATE, data);


	  return IDOK;
   

	}
   SetRichEditFontColor(0x000000);
   MemoPut(hMemo, GetMemoCount(hMemo)+1, buf);

  }  
  return IDOK;
}





void OnStartMonitor(HWND hDlg)
{

  if (!bActive)
  {
      char buf[255] = {0};
	  //memset(&buf, 0, sizeOf(buf));

	  GetDlgItemText(hDlg, IDC_PID, buf, sizeof(buf));

	  DWORD Pid = GetPid(buf);
	  DWORD ComID = SendMessage(GetDlgItem(hDlg, IDC_COM), CB_GETCURSEL, 0, 0);


	  if (Pid < 5) 
	  {
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "无效的进程ID: %d", Pid);
		MessageBox(hDlg, buf, "提示", MB_OK | MB_ICONERROR); 
		return;
	  }

    //memset(buf, 0, sizeof(buf));
    //sprintf(buf, "进程ID: %d, COM = %d", Pid, ComID);
    //MessageBox(hDlg, buf, "提示", MB_OK | MB_ICONINFORMATION); 
	 
	//(TOnData)AOnData
    bActive = MonitorComm(Pid,  ComID, (TOnData)AOnData);
	if (bActive){

	  SetDlgItemText(hDlg, IDC_START, "停止监视");

      char p[64] = {0};
      memset(buf, 0, sizeof(buf));
	  GetDlgItemText(hDlg, IDC_PID, p, sizeof(p));
      sprintf(buf, "打开监视成功, %s\r\n", p);
	  SetDlgItemText(hDlg, IDC_REDTDATA, buf);
      CheckMenuItem(GetMenu(hDlg), IDC_START, MF_CHECKED);
	}

  } else
  {

    UnMonitorComm();
    bActive = FALSE;
	MemoPut(hMemo, GetMemoCount(hMemo)+1, "成功关闭监视\r\n");
    SetDlgItemText(hDlg, IDC_START, "启动监视");
	CheckMenuItem(GetMenu(hDlg), IDC_START, MF_UNCHECKED);
  }
    
	
  return;
  
}

void FindItem(LPCSTR AStr, HWND hComBox)
{
  int  i;
  char buf[24];
  char line[24];
  //首先更新ComBox
  int iCount = SendMessage(hComBox, CB_GETCOUNT, 0, 0);
  for (i = 0; i< iCount; i++)
  {
    int Len = SendMessage(hComBox, CB_GETLBTEXTLEN, i, 0);
    if (Len != CB_ERR)  
    {
      memset(line , 0, sizeof(line));
      if (SendMessage(hComBox, CB_GETLBTEXT, i, LONG(&line)) == 0)  
        continue;

      memset(buf, 0, sizeof(buf));
	  lstrcpyn(buf, line, lstrlen(AStr)+1);

	  if (lstrcmp(buf, AStr) == 0)
	  {
	     SendMessage(hComBox, CB_SETCURSEL, i, 0);
		 break;
	  }
         
    }
  }
}


void GetFromWindowPid(HWND hComBox)
{
  tagPOINT Pt;
  DWORD PID;
  HWND H;
  char Buf[24]= {0};
 
  //如果打开了监控
  //if ActiveCapture then Exit;

  if (!GetCursorPos(&Pt)) return;

  H = WindowFromPoint(Pt);
  if (H > 0)
  {
    GetWindowThreadProcessId(H, &PID);

    if (PID < 5) return;
    memset(Buf, 0, sizeof(Buf));
    sprintf(Buf, "%-8d ", PID);
    FindItem(Buf, hComBox);  
   }
}

BOOL ClacPoint(LPARAM lParam)
{
  tagPOINT Pt;
  Pt.x  = LOWORD(lParam);
  Pt.y  = HIWORD(lParam);
  if ( (Pt.x >= 6)&& (Pt.x <= 6+32) && (Pt.y >= 12) && (Pt.y <= 12+32) )  
     return TRUE;
  else return FALSE;
}


void OnMouseEvent(HWND hMain, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
 if (!bActive)
  switch (uMsg)
  {
  case  WM_LBUTTONUP: 
                       ReleaseCapture();
                       if (MouseLButtonDown)  GetFromWindowPid(GetDlgItem(hMain, IDC_PID));                         

                       MouseLButtonDown = FALSE;
                       break;
  case  WM_LBUTTONDOWN:  
                       MouseLButtonDown = ClacPoint(lParam);
                       if (MouseLButtonDown)  {                   
                         SetCursor(LoadIcon(HInst, (LPCSTR)IDI_APP));
                         SetCapture(hMain);
                       }
					   break;
                     
  case  WM_MOUSEMOVE:    if (MouseLButtonDown || ClacPoint(lParam))  {
                      
                        if (MouseLButtonDown) GetFromWindowPid(GetDlgItem(hMain, IDC_PID));

                        SetCursor(LoadIcon(HInst, (LPCSTR)IDI_APP));
					    break;
                     }
   }
}

void OnResize(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
   int nWidth  = LOWORD(lParam);  // width of client area 
   int nHeight = HIWORD(lParam);
   
   HWND  hCLose = GetDlgItem(hDlg, ID_OK);
   if (hCLose) 
   {
     MoveWindow(hCLose, nWidth-75-6, 16, 75, 22, TRUE);
	 RedrawWindow(hCLose, NULL, 0, RDW_INVALIDATE);
   }


   //MoveWindow(hMemo, 6, 45, nWidth-12, nHeight-66, TRUE);
   MoveWindow(hMemo, 6, 45, nWidth-12, nHeight-66, TRUE);
   MoveWindow(GetDlgItem(hDlg, IDC_STATE), 2, nHeight-18, nWidth-2, 18, TRUE);

}


void OnPaint(HWND hDlg)
{
  tagPAINTSTRUCT lpPaint;
  BeginPaint(hDlg, &lpPaint);
  HDC DC = GetDC(hDlg);
  DrawIcon(DC, 6, 12, LoadIcon(HInst, (LPCSTR)IDI_APP));
  ReleaseDC(hDlg, DC);
  EndPaint(hDlg, &lpPaint);
}


void OnTimer(HWND hDlg)
{
  GetAllProcess(GetDlgItem(hDlg, IDC_PID));

  if (bActive)
  if (SendMessage(GetDlgItem(hDlg, IDC_PID), CB_GETCURSEL, 0, 0) < 0)  
      PostMessage(hDlg, WM_COMMAND, IDC_START, 0);    
}


BOOL SaveDialog(HWND hMain, LPSTR lpFileName, LPCSTR lpFilter, LPCSTR lpDir)
{

  OPENFILENAME    FDialog;
  memset(&FDialog, 0, sizeof(OPENFILENAME));

  FDialog.lStructSize = sizeof(tagOFNA); 
  FDialog.hwndOwner =  hMain;
  FDialog.hInstance = HInst;
  FDialog.nMaxFile  = MAX_PATH;

  FDialog.lpstrFilter = lpFilter;
  FDialog.lpstrFile = lpFileName;

  FDialog.lpstrTitle  = "Save file";
  FDialog.lpstrDefExt = ".txt";

  FDialog.lpstrInitialDir  = lpDir;
  FDialog.Flags = OFN_HIDEREADONLY;
  return GetSaveFileName(&FDialog);
}

 

LONG WriteFileBuffer(LPCSTR AFileName, LPCSTR Buf, LONG Len)
{
    LONG lpWriteTotal = 0;
	HANDLE hf = CreateFile(AFileName, 
	                      GENERIC_READ | GENERIC_WRITE, 
						  0, 
						  NULL, 
						  CREATE_ALWAYS, 
						  FILE_ATTRIBUTE_NORMAL, 
						  NULL);

	if (hf != INVALID_HANDLE_VALUE)
	{
		lpWriteTotal = _lwrite((HFILE)hf, Buf, Len);
		_lclose((HFILE) hf);
		
	}
	return lpWriteTotal;
}

 


void OnCommand(HWND hDlg, UINT ID, HWND hFocus, UINT NotifyCode)
{
	if (ID == IDC_START)     OnStartMonitor(hDlg);		
	if (ID == IDC_ClearEdit) SetDlgItemTextA(hDlg, IDC_REDTDATA, NULL);			
	if (ID == ID_OK)         PostMessage(hDlg, WM_CLOSE, 0, 0);
	if (ID == IDM_HELP)      ShellExecute(hDlg, "open", "commmonitor.chm", NULL, NULL, SW_SHOW);
	if (ID == IDM_TOP)
	{
		if (GetMenuState(GetMenu(hDlg), IDM_TOP, MF_BYCOMMAND) == MF_CHECKED)  
        {
			SetWindowPos(hDlg, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			CheckMenuItem(GetMenu(hDlg), IDM_TOP, MF_UNCHECKED);
        } else {
			SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			CheckMenuItem(GetMenu(hDlg), IDM_TOP, MF_CHECKED);
		}
	}
	if ((ID == IDM_HOME)||(ID == IDM_SOFT) || (ID == IDM_ABOUT))
	{
	   char lpAppInfo[512] = {0};
	   if (ID == IDM_ABOUT)
	   {
		//	LoadString(HInst, IDS_APPINFO, lpAppInfo, sizeof(lpAppInfo));

			MessageBox(hDlg, IDS_APPINFO, "提示", MB_OK | MB_ICONINFORMATION);
		}

        memset(lpAppInfo, 0, sizeof(lpAppInfo));
		if (ID == IDM_SOFT)   
		   lstrcpy(lpAppInfo, "http://software.jfyes.com/");
        else 
		   lstrcpy(lpAppInfo, "http://www.jfyes.com/");
		/////////////////
	    ShellExecute  (0, "open", lpAppInfo, NULL, NULL, SW_SHOW);
	}
	if (ID == IDM_SAVE)
	{
	   char lpFileName [MAX_PATH] = {"CommMonitor.log"};

        
       if(SaveDialog(hDlg, lpFileName,  "LogFile\0*.log\0All file\0*.*\0", "."))
	   {
		 LONG dwSize = GetWindowTextLength(hMemo)+1;

		 LPCSTR lpText = (LPCSTR) malloc(dwSize);
		 LONG lpReadSize = GetDlgItemText(hDlg, IDC_REDTDATA, (char *)lpText, dwSize);
		 if (lpReadSize > 0) 
			WriteFileBuffer(lpFileName, 
			                lpText, 
							lpReadSize);
         free((void *)lpText);         
	   }
	}

}

void OnClose(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	if (MessageBox(hDlg, "确定要退出吗？", "提示", MB_OKCANCEL | MB_ICONINFORMATION) != IDOK) 
		return;

	KillTimer(hDlg, 1);
	if (bActive)  OnStartMonitor(hDlg);
	EndDialog(hDlg, LOWORD(wParam));
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{   //注意Break;	不能掉了;
	    case WM_INITDIALOG:  return SetWindowLong(hDlg, DWL_MSGRESULT, (LONG)InitDlg(hDlg, wParam, lParam));
	    case WM_TIMER:       OnTimer(hDlg); break;
	    case WM_PAINT:       OnPaint(hDlg); break;
		case WM_SYSCOMMAND:  OnCommand(hDlg, (UINT)LOWORD(wParam),  (HWND)lParam, HIWORD(wParam)); break;		
		case WM_COMMAND:     OnCommand(hDlg, (UINT)LOWORD(wParam),  (HWND)lParam, HIWORD(wParam)); break;
		case WM_CLOSE:       OnClose(hDlg, wParam, lParam);  return TRUE;
        case WM_MOUSEMOVE:   OnMouseEvent(hDlg, message, wParam, lParam); break;
	    case WM_LBUTTONUP:   OnMouseEvent(hDlg, message, wParam, lParam); break;
        case WM_LBUTTONDOWN: OnMouseEvent(hDlg, message, wParam, lParam); break;
		case WM_SIZE:        OnResize(hDlg, wParam, lParam); break;  
	}
    return FALSE;
}
