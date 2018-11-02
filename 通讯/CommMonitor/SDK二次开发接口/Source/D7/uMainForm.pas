{
---------------------------
提示
---------------------------
PMonitorComm.dll.dll 串口监视精灵 试用版DLL，100次调用得重启目标程序。
---------------------------

}


unit uMainForm;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ComCtrls, ExtCtrls, Buttons, Menus, ActnList;


const
  IDS_VERSION  = '3.13';
  IDS_APPTITLE = 'CommMonitor'+IDS_VERSION+' 串口监视精灵(FOR Delphi DLL版)';
  IDS_APPINFO  = #13#10#13#10+
                 '         SoftName : ' + IDS_APPTITLE+ #13#10+
                 '         Version  : V' + IDS_VERSION  +#13#10 +
                 '         BuildDate: 2010-02-03 23:55:59'#13#10 +
                 '         Email    : jfyes@qq.com'#13#10 +
                 '         QQ       : 348677065'#13#10 +
                 '         Home     : http://www.jfyes.com'#13#10 +
                 '         SoftWare : http://software.jfyes.com'#13#10#13#10 +
                 '     Copyright (C) 2003-2010 jfyes网络科技                 '#13#10#13#10#13#10#13#10+
                 '     提供二次开发: DLL 或 控件 http://software.jfyes.com'#13#10;


  SS_START  = '启动监视';
  SS_STOP   = '停止监视';



const
  COMM_OPEN         = $01;
  COMM_CLOSE        = $02;
  COMM_READ         = $03;
  COMM_WRITE        = $04;
  
type
 
  THookData = packed record
    ComPort:    Byte;        //串口号
    CommState:  Byte;        //HOOK TYPE
    FileHandle: THandle;    //被打开的文件句柄
    DataSize:   Integer;    //数据大小
    Data: array [0..8912-1] of Char;        //数据
  end;
  PHookData = ^THookData;


  Tfm_MainForm = class(TForm)
    redt_Data: TRichEdit;
    cbb_Com: TComboBox;
    btn_Start: TButton;
    chk_asc: TCheckBox;
    tmr1: TTimer;
    Label1: TLabel;
    Label2: TLabel;
    img1: TImage;
    btn1: TBitBtn;
    stat1: TStatusBar;
    mm1: TMainMenu;
    N1: TMenuItem;
    N2: TMenuItem;
    N3: TMenuItem;
    N4: TMenuItem;
    N5: TMenuItem;
    N6: TMenuItem;
    N7: TMenuItem;
    btnClear: TButton;
    actlst1: TActionList;
    act_start: TAction;
    act_Clear: TAction;
    act_Exit: TAction;
    dlgSave1: TSaveDialog;
    N8: TMenuItem;
    N9: TMenuItem;
    act_Top: TAction;
    act_Help: TAction;
    act_About: TAction;
    act_SoftWare: TAction;
    act_Home: TAction;
    N10: TMenuItem;
    N11: TMenuItem;
    N12: TMenuItem;
    N13: TMenuItem;
    jfyes1: TMenuItem;
    N14: TMenuItem;
    cbb_PID: TComboBox;
    procedure FormCreate(Sender: TObject);
    procedure tmr1Timer(Sender: TObject);
    procedure img1MouseDown(Sender: TObject; Button: TMouseButton;
      Shift: TShiftState; X, Y: Integer);
    procedure img1MouseMove(Sender: TObject; Shift: TShiftState; X,
      Y: Integer);
    procedure N7Click(Sender: TObject);
    procedure act_startExecute(Sender: TObject);
    procedure N4Click(Sender: TObject);
    procedure act_ClearExecute(Sender: TObject);
    procedure act_HelpExecute(Sender: TObject);
    procedure act_AboutExecute(Sender: TObject);
    procedure act_SoftWareExecute(Sender: TObject);
    procedure act_TopExecute(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
  private
    { Private declarations }
    FReadTotal, FWriteTotal: Cardinal;
    procedure AddLog(const Log: string; const AColor: TColor = clBlack);
    function GetPidFormWindow: string;

    function OnData(P: PHookData): Integer;       
  public
    { Public declarations }
  end;

type
  //回调func
  TOnData = function(lParam: Integer): Integer; stdcall;

//启动串口监视
//Pid要监视的进程ID
//ComIndex: 要监视的COM号
//lpCallFunc: 回调func, 传递一个TOnData过程指针
function MonitorComm(Pid, ComIndex: DWORD; lpCallFunc: TOnData): BOOL; stdcall external 'PMonitorComm.dll';
//关闭串口监视, 返BOOL类型
function UnMonitorComm(): BOOL; stdcall; external 'PMonitorComm.dll';
//取得全部进程ID
procedure GetAllProcess(hComBox: HWND); stdcall  external 'PMonitorComm.dll';


var
  fm_MainForm: Tfm_MainForm;

  
implementation

uses ShellAPI;

 

{$R *.dfm}

function AOnData(lParam: Integer): Integer; stdcall;
begin
  Result := fm_MainForm.OnData(PHookData(lParam));
end;


procedure Tfm_MainForm.FormCreate(Sender: TObject);
var
  I: Integer;
begin
  self.Caption := IDS_APPTITLE;
  Application.Title := self.Caption;
  
  self.redt_Data.Clear;
  self.cbb_Com.Items.Add('所有COM口');

  for I := 1 to 255 do
  begin
    self.cbb_Com.Items.Add(Format('COM%d', [I]));
  end;
  cbb_Com.ItemIndex := 0;
  GetAllProcess(self.cbb_PId.Handle);

  FReadTotal := 0;
  FWriteTotal := 0;

end;

procedure Tfm_MainForm.AddLog(const Log: string; const AColor: TColor = clBlack);
begin
  self.redt_Data.SelAttributes.Color := AColor;
  self.redt_Data.SelStart := MaxInt;
  try
    if self.redt_Data.Text = '' then
    self.redt_Data.SelText := Log
    else
    self.redt_Data.SelText := #13#10+Log;
  except
  end;
end;


function Tfm_MainForm.OnData(P: PHookData): Integer;
var
  s: string;
  I: Integer;
begin
  Result := IDOK;
  if p = nil then Exit;
  case p.CommState of
    COMM_OPEN:   AddLog(Format('打开 COM%d', [p.ComPort]));
    COMM_CLOSE:  AddLog(Format('关闭 COM%d', [p.ComPort]));
    COMM_READ,
    COMM_WRITE:  begin

                   if p.CommState = COMM_READ then
                   begin
                     s := Format('COM%d Read: %d(Byte)', [P.ComPort, P.DataSize]);
                     Inc(FReadTotal, P.DataSize);
                   end
                   else begin
                     s := Format('COM%d Write: %d(Byte)', [P.ComPort, P.DataSize]);
                     Inc(FWriteTotal, P.DataSize);
                   end;

                   self.stat1.Panels[0].Text := Format('进程: %s, %s, Write: %u(Bytes) |  Read: %u(Bytes)',
                                               [self.cbb_PID.Text, self.cbb_Com.Text, FWriteTotal, FReadTotal]);

                   if chk_asc.Checked then
                   begin
                     for I := 0 to P.DataSize -1 do
                       s := Format('%s%.2x ', [s, Ord(P.Data[I]) ]);
                   end else
                   begin
                     s := s + P.Data;
                   end;

                   if p.CommState = COMM_READ then  AddLog(s, clBlue)
                   else AddLog(s, clGreen);
                 end;
  end;
end;

procedure Tfm_MainForm.tmr1Timer(Sender: TObject);
begin
  GetAllProcess(self.cbb_PId.Handle);

  if self.act_start.Caption = SS_STOP then
  if  cbb_PID.Items.IndexOf(cbb_PID.Text) < 0 then
  begin
    cbb_PID.Text := '';
    act_start.Caption := SS_START;
    act_start.Checked := False;
    AddLog('目标进程已关闭');
  end;
end;



function Tfm_MainForm.GetPidFormWindow(): string;
  function FindItem(AStr: string): Integer;
  var I: Integer;
  begin
    for I := 0 to cbb_PID.Items.Count -1 do
    if Pos(AStr, cbb_PID.Items[I]) = 1 then
    begin
      Result := I;
      Exit;
    end;
    Result := -1;
  end;
var
  H: HWND;
  s: string;
  PID: DWORD;
  pt: TPoint;
  AIndex: Integer;
begin
  if not GetCursorPos(pt) then Exit;
  H := Windows.WindowFromPoint(pt);
  if H = 0 then Exit;
  Windows.GetWindowThreadProcessId(H, @PID);
  if PID > 0 then
  begin
    s := Format('%-8d ', [Pid]);
    AIndex := FindItem(s);
    if AIndex >= 0 then cbb_PID.ItemIndex := AIndex;
  end;

end;

procedure Tfm_MainForm.img1MouseDown(Sender: TObject; Button: TMouseButton;
  Shift: TShiftState; X, Y: Integer);
begin
  windows.SetCursor(self.img1.Picture.Icon.Handle)
end;

procedure Tfm_MainForm.img1MouseMove(Sender: TObject; Shift: TShiftState;
  X, Y: Integer);
begin
  windows.SetCursor(self.img1.Picture.Icon.Handle);
  GetPidFormWindow ();
end;

procedure Tfm_MainForm.N7Click(Sender: TObject);
begin
  Close;
end;

procedure Tfm_MainForm.act_startExecute(Sender: TObject);
var
  PID: DWORD;
begin
  PID := StrToIntDef(Trim(Copy(cbb_PID.Text, 1, Pos(#32, cbb_PID.Text))), 0);

  if PID = 0 then
  begin
    ShowMessage('无效的进程ID');
    self.cbb_PID.SetFocus;
    Exit;
  end;

  act_start.Checked := not act_start.Checked;
  if act_start.Checked then
  begin
    act_start.Checked := MonitorComm(PID, cbb_Com.ItemIndex, AOnData);
    if act_start.Checked then
    begin
      act_start.Caption := SS_STOP;
      AddLog('成功打开监视, '+ cbb_Com.Text);
    end
    else
    if Windows.GetLastError > 0 then
      ShowMessage(SysUtils.SysErrorMessage(Windows.GetLastError));
  end
  else  begin
    UnMonitorComm();
    act_start.Checked := False;
    act_start.Caption := SS_START;
    AddLog('成功停止监视');
    //else ShowMessage()//ShowMessage(SysUtils.SysErrorMessage(Windows.GetLastError));
  end;
end;

procedure Tfm_MainForm.N4Click(Sender: TObject);
begin
  if self.redt_Data.Text = '' then
  begin
    ShowMessage('当前日志为空。');
    Exit;
  end;
  
  if   dlgSave1.Execute then
  begin
    if SysUtils.FileExists(dlgSave1.FileName) then
    if Application.MessageBox(PChar(Format('文件：%s，已经存在，是否要替换它？', [dlgSave1.FileName])),
                              '提示', MB_OKCANCEL or MB_ICONQUESTION) <> IDOK then Exit;

    self.redt_Data.Lines.SaveToFile(dlgSave1.FileName);
  end;
end;

procedure Tfm_MainForm.act_ClearExecute(Sender: TObject);
begin
  redt_Data.Clear;
end;

function GetRelFileName(AFileName: string): string;
var
  buf: string;
  Len: Integer;
begin
  buf := ExtractFileName(AFileName);
  Len := Length(buf);
  while buf[Len] <> '.' do Dec(Len);
  Result := Copy(buf, 1, Len); 
end;

procedure Tfm_MainForm.act_HelpExecute(Sender: TObject);
var
  sFileName: string;
begin
  sFileName := 'CommMonitor.chm';
  if SysUtils.FileExists(sFileName) then
    ShellExecute(self.Handle, 'Open', PChar(sFileName), nil, nil, SW_SHOW)
  else  Windows.MessageBox(Handle, PChar('没有找到帮助文件：'+sFileName), '提示', MB_OK or MB_ICONINFORMATION);
end;


procedure Tfm_MainForm.act_AboutExecute(Sender: TObject);

begin
  Windows.MessageBox(Handle, IDS_APPINFO, '提示', MB_OK or MB_ICONINFORMATION);
  ShellExecute(0, 'open', 'http://www.jfyes.com', nil, nil, SW_SHOW);
end;

procedure Tfm_MainForm.act_SoftWareExecute(Sender: TObject);
var
  s: string;
begin
  if TAction(Sender).Tag  = 1 then
    s :=  'http://www.jfyes.com'
  else s :=  'http://software.jfyes.com';

  ShellExecute(0, 'open', PChar(s), nil, nil, SW_SHOW);
end;

procedure Tfm_MainForm.act_TopExecute(Sender: TObject);
begin
  act_Top.Checked := not act_Top.Checked;
  if act_Top.Checked then
    Windows.SetWindowPos(Handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE or SWP_NOSIZE)
  else
    Windows.SetWindowPos(Handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE or SWP_NOSIZE);
end;

procedure Tfm_MainForm.FormClose(Sender: TObject;
  var Action: TCloseAction);
begin

  if act_start.Checked then
    act_start.Execute;
end;

end.
