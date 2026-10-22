#!/bin/sh
set -e

TESTDIR=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$TESTDIR/.." && pwd)
TGT=${1:-android16-6.12}
OUT=$ROOT/out/$TGT
DEVICE=$OUT/device
DOCKER=docker.cnb.cool/ylarod/ddk/ddk-min:$TGT

mkdir -p "$DEVICE"

MOD=inte "$ROOT/scripts/build-ddkk.sh" "$TGT"
MOD=poc_hide "$ROOT/scripts/build-ddkk.sh" "$TGT"
docker run --rm -e KDIR="/opt/ddk/kdir/$TGT" -e VER="$TGT" \
	-v "$TESTDIR/badmod":/src -w /src \
	"$DOCKER" make

aarch64-linux-gnu-gcc -static -O2 -o "$DEVICE/inte_demo" "$TESTDIR/demo.c"
cp "$OUT/inte/inte.ko" "$DEVICE/"
cp "$OUT/poc_hide/poc_hide.ko" "$DEVICE/"
cp "$TESTDIR/badmod/out/$TGT/badmod.ko" "$DEVICE/"

echo "device package: $DEVICE"
ls -la "$DEVICE"
