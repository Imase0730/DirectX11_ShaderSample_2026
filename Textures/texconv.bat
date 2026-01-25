@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

REM ============================
REM 出力フォルダ設定
REM ============================
set "OUTDIR=%~dp0..\Resources\Models"

REM フォルダがなければ作成
if not exist "%OUTDIR%" (
    mkdir "%OUTDIR%"
)

REM ============================
REM texconvのパス
REM ============================
set "TEXCONV=%~dp0texconv.exe"

if not exist "%TEXCONV%" (
    echo ERROR: texconv.exe が見つかりません
    pause
    exit /b 1
)

REM ============================
REM ドロップされた全ファイルを処理
REM ============================
for %%F in (%*) do (
    echo Converting: %%~nxF
    start "" /B "%TEXCONV%" -y -f BC7_UNORM -o "%OUTDIR%" "%%F"
)

echo.
echo 変換完了！
pause
