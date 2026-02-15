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
REM ドロップされたファイルを処理
REM ============================
if "%~1"=="" (
    echo 変換するファイルをドラッグ＆ドロップしてください
    pause
    exit /b 1
)

:process_loop
if "%~1"=="" goto done

REM 変換対象のフルパス
set "FILE=%~1"

echo Converting: "!FILE!"
"%TEXCONV%" -y -f BC5_UNORM -inverty -o "%OUTDIR%" "!FILE!"
echo ERRORLEVEL = !errorlevel!
echo.

shift
goto process_loop

:done
echo.
echo すべての変換が完了しました！
pause
