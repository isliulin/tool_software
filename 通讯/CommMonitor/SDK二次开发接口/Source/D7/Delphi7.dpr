program Delphi7;

uses
  Forms,
  uMainForm in 'uMainForm.pas' {fm_MainForm};

{$R *.res}

begin
  Application.Initialize;
  Application.CreateForm(Tfm_MainForm, fm_MainForm);
  Application.Run;
end.
