rm -f example-ctr.3dsx
cd ../src
make PLATFORM=PLATFORM_CTR RAYLIB_MODULE_AUDIO=FALSE
cd ../example-ctr
make
citra example-ctr.3dsx 1>/dev/null 2>/dev/null