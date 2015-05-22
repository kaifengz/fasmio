#!/bin/bash

lastbuildnum=$(ls ../backup/ | cut -d. -f3 | sort -n | tail -1)
buildnum=$(($lastbuildnum + 1))
backupfn=$(date +backup.%Y-%m-%d.$buildnum)

cd ..
tar cf backup/$backupfn.tar .hg
cd backup
zip -e $backupfn.tar.zip.tmp $backupfn.tar
mv -f $backupfn.tar.zip.tmp $backupfn.tar.zip
rm -f $backupfn.tar
ls -alF $backupfn.tar.zip

