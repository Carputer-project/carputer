#!/bin/sh
export QT_QPA_PLATFORM=eglfs
export QT_QPA_EGLFS_KMS_CONFIG=/etc/kms.json
export EGL_PLATFORM=drm
export LIBVA_DRIVER_NAME=iHD
export LIBVA_DRM_DEVICE=/dev/dri/renderD128
exec /usr/bin/carputer "$@"
