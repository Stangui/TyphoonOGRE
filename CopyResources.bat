@echo off
echo Copying media to Bin folder...
call robocopy "%~dp0ThirdParty/OGRE/Media" "%~dp0bin/Media" *.* /s /np /r:1 /xo
if %errorlevel% GEQ 8 exit /b %errorlevel%

echo Copying configs to Bin folders...
call robocopy "%~dp0ThirdParty/OGRE" "%~dp0bin/debug" *.cfg /np /r:1 /xo
if %errorlevel% GEQ 8 exit /b %errorlevel%
call robocopy "%~dp0ThirdParty/OGRE" "%~dp0bin/release" *.cfg /np /r:1 /xo
if %errorlevel% GEQ 8 exit /b %errorlevel%
call robocopy "%~dp0ThirdParty/OGRE" "%~dp0bin/shipping" *.cfg /np /r:1 /xo
if %errorlevel% GEQ 8 exit /b %errorlevel%

echo Copying binaries to Bin folders...
call robocopy "%~dp0ThirdParty/OGRE/Bin/debug" "%~dp0bin/debug" *.dll *.pdb /np /r:1 /xo
if %errorlevel% GEQ 8 exit /b %errorlevel%
call robocopy "%~dp0ThirdParty/SDL2/Bin" "%~dp0bin/debug" SDL2_d.dll /np /r:1 /xo
if %errorlevel% GEQ 8 exit /b %errorlevel%
call robocopy "%~dp0ThirdParty/OGRE/Bin/release" "%~dp0bin/release" *.dll *.pdb /np /r:1 /xo
if %errorlevel% GEQ 8 exit /b %errorlevel%
call robocopy "%~dp0ThirdParty/SDL2/Bin" "%~dp0bin/release" SDL2.dll /np /r:1 /xo
if %errorlevel% GEQ 8 exit /b %errorlevel%
call robocopy "%~dp0ThirdParty/OGRE/Bin/release" "%~dp0bin/shipping" *.dll *.pdb /np /r:1 /xo
if %errorlevel% GEQ 8 exit /b %errorlevel%
call robocopy "%~dp0ThirdParty/SDL2/Bin" "%~dp0bin/shipping" SDL2.dll /np /r:1 /xo
if %errorlevel% GEQ 8 exit /b %errorlevel%
echo Done
