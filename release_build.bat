@echo off

set CommonCompilerFlags=-MT -nologo -GR- -EHa -O2 -Oi -WX -W4 -wd4201 -wd4100 -wd4505 -wd4189 -FC 
set CommonLinkerFlags=-incremental:no -opt:ref ole32.lib Xinput.lib user32.lib Gdi32.lib Winmm.lib

cl %CommonCompilerFlags% IGame.cpp /LD /link -incremental:no -opt:ref /EXPORT:GameUpdateAndRender

cl %CommonCompilerFlags% -Fe"Zippy" win32_platform.cpp  /link %CommonLinkerFlags%
