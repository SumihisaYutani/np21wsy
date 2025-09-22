@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
cd /d "J:\project\np21wsy\win9x"
msbuild np21vs2019.vcxproj /p:Configuration=Release /p:Platform=x64