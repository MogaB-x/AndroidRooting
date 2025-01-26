#!/bin/sh

mv /android/system/bin/app_process64 /android/system/bin/app_process64_original
cp /android/system/su_ota/system/app_daemon /android/system/bin/app_process64
cp /android/system/su_ota/system/mysu /android/system/bin/mysu
cp /android/system/su_ota/system/mydaemon /android/system/bin/mydaemon

chmod 755 /android/system/bin/mysu
chmod 755 /android/system/bin/mydaemon
chmod 755 /android/system/bin/app_process64

exit 0
