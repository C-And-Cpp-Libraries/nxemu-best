@ECHO OFF
SETLOCAL

for /f "delims=" %%a in ('WHERE 7z 2^>nul') do set "zip=%%a"

if "%zip%" == "" (
	if exist "C:\Program Files\7-Zip\7z.exe" (
		set "zip=C:\Program Files\7-Zip\7z.exe"
	) else (
		echo can not find 7z.exe
		goto :EndErr
	)
)

set ZipFileName=nxemu
set VSPlatform=x64
if not "%1" == "" set ZipFileName=%1
if not "%2" == "" set VSPlatform=%2

SET current_dir=%cd%
cd /d %~dp0..\..\
SET base_dir=%cd%
cd /d %current_dir%

IF EXIST "%base_dir%\package" rmdir /S /Q "%base_dir%\package"
IF %ERRORLEVEL% NEQ 0 GOTO EndErr
IF NOT EXIST "%base_dir%\package" mkdir "%base_dir%\package"
IF %ERRORLEVEL% NEQ 0 GOTO EndErr

rd "%base_dir%\bin\package" /Q /S > NUL 2>&1
md "%base_dir%\bin\package\"
md "%base_dir%\bin\package\lang\"
md "%base_dir%\bin\package\modules\"
md "%base_dir%\bin\package\modules\cpu\"
md "%base_dir%\bin\package\modules\operating_system\"
md "%base_dir%\bin\package\modules\video\"
)

copy "%base_dir%\bin\%VSPlatform%\Release\sciter.dll" "%base_dir%\bin\package"
copy "%base_dir%\bin\%VSPlatform%\Release\nxemu.exe" "%base_dir%\bin\package"
copy "%base_dir%\modules\%VSPlatform%\cpu\nxemu-cpu.dll" "%base_dir%\bin\package\modules\cpu"
copy "%base_dir%\modules\%VSPlatform%\operating_system\nxemu-os.dll" "%base_dir%\bin\package\modules\operating_system"
copy "%base_dir%\modules\%VSPlatform%\video\nxemu-video.dll" "%base_dir%\bin\package\modules\video"

:: Language
echo "D:\Dev\nxemu-public\bin\x64\Release\resource_packer.exe" "D:\Dev\nxemu-public\lang\english" "%base_dir%\bin\package\lang\english.lang"

cd %base_dir%\bin\package
"%zip%" a -tzip -r "%base_dir%\package\%ZipFileName%" *
cd /d %current_dir%

echo package %ZipFileName% created

goto :end


:EndErr
ENDLOCAL
echo Build failed
exit /B 1

:End
ENDLOCAL
exit /B 0
