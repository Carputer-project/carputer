# Build and Deploy Carputer to Live System

**Created**: 2026-04-16
**Updated**: 2026-04-19
**Tested**: Yes - Working

## Overview

This document describes how to build the Carputer Qt application and deploy it to the live system running at `192.168.1.100`.

## Prerequisites

- Qt5 development environment with qmake
- SSH access to the carputer system
- Buildroot build environment (for full OS builds)

## Quick Build and Deploy

### Step 1: Build the Application

```bash
cd /home/devkid/Desktop/carputer/carputer
qmake carputer.pro
make -j$(nproc)
```

### Step 2: Deploy to Live System

```bash
# Deploy binary directly via SSH pipe
cat /home/devkid/Desktop/carputer/carputer/carputer | \
    sshpass -p "12345678" ssh -o StrictHostKeyChecking=no root@192.168.1.100 \
    "cat > /usr/bin/carputer && chmod +x /usr/bin/carputer"

# Start the service
sshpass -p "12345678" ssh -o StrictHostKeyChecking=no root@192.168.1.100 \
    "systemctl restart carputer"
```

## Important Notes

1. **NEVER use `make clean`** - This forces a full rebuild and wastes time
2. **Use incremental builds** - Only changed components are recompiled
3. **The update package method is broken** - Use direct SSH pipe instead

## Verification Commands

On the live system:

```bash
# Check carputer status
systemctl status carputer

# Check version
cat /etc/carputer/version.txt

# View logs
journalctl -u carputer -f

# Check if binary is running
ps aux | grep carputer
```

## Troubleshooting

### Binary not found after deploy
```bash
ls -la /usr/bin/carputer
```

### Service won't start
```bash
journalctl -u carputer -n 50
```

### Permission denied
```bash
chmod +x /usr/bin/carputer
```