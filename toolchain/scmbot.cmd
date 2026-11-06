@echo off
rem scmbot on Windows (Docker Desktop), run in the bot's folder: scmbot new | init | build (github.com/scm-js/scmbot).
rem The image is downloaded the first time: the newest toolchain, the one the play page loads. After updating
rem scm-sim, "set SCMBOT_UPDATE=1" once to download it again, or "set SCMBOT_REBUILD=1" to build it from this clone
rem (as after changing the toolchain). When it cannot be downloaded, it is built from this clone.
setlocal
set image=scmbot
set registry=ghcr.io/scm-js/scmbot
docker image inspect %image% >nul 2>&1
if errorlevel 1 set SCMBOT_UPDATE=1
if defined SCMBOT_REBUILD goto build
if not defined SCMBOT_UPDATE goto run
echo scmbot: downloading the toolchain image ^(once; it is a large download^) 1>&2
docker pull -q %registry%:latest >nul 2>&1 && docker tag %registry%:latest %image% && goto run
echo scmbot: it could not be downloaded, so it is built here ^(the first time takes a few minutes^) 1>&2
:build
docker build -q -f "%~dp0Dockerfile" -t %image% "%~dp0.." >nul || exit /b 1
:run
for %%I in (.) do set folder=%%~nxI
docker run --rm -t -e SCMBOT_FOLDER="%folder%" -v "%cd%:/bot" %image% %*
