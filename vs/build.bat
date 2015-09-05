setlocal
set GYP_GENERATORS=ninja,msvs
set GYP_MSVS_VERSION=2010
python.exe generateProject.py --generator-output=. -Ibuild/common.gypi --depth=. %* study.gyp
endlocal