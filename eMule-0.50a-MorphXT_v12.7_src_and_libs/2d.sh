echo running for release in:
export DESTDIR="`muleversion/muleversion.exe|sed \"s/ //g\"`"
echo $DESTDIR
echo continue = enter Break=stop
read conti
./ufind cryptopp srchybrid id3lib png Resizablelib zlib  -type d -print|grep -vi CVS|grep -vi Release|grep -vi Debug|grep -vi BETA |grep -v .svn >dirlist.txt
./ufind cryptopp srchybrid id3lib png Resizablelib zlib  -name "*.cpp" -print -o -name "*.c" -print -o -name "*.rc" -print -o -name "*.jpg" -print -o -name "*fest" -print -o -name "*bin" -print -o -name "*.h" -print -o -name "emule.sln" -print -o -name "emule80.sln"  -print -o -name "LICENCE" -print -o -name "*.rc2" -print -o -name "*.rc" -print -o -name "*.pdf" -print -o -name "*.asm" -print -o -name "*.y" -print -o -name "*.l" -print -o -name "*hpp" -print -o -name "*.htm" -print -o -name "*bmp" -print -o -name "*.avi" -print -o -name "*.gif" -print -o -name "*.ICO" -print -o -name "READ*" -o -name "LICENSE" -print -o -name "THANKS" -o -name "Artistic-License.txt" -print -o -name "License.txt" -print -o -name "Changelog" -o -name "NEWS" -print -o -name "*.ico" -print -o -name "*.inl" -print -o -name "*.vcproj" -print |grep -v BuildLog.htm >srclist.txt
./ufind zlib/contrib/masmx86 -type f -print |grep -v CVS >>srclist.txt
mkdir "$DESTDIR"
mkdir "$DESTDIR/srchybrid"
set -x
cat dirlist.txt|sort |xargs -i echo mkdir --verbose $DESTDIR/{} >mkdir.sh
sh mkdir.sh
cat srclist.txt|xargs -i cp -p {} "$DESTDIR/{}" 
cp srchybrid/ptreads/pthreads/*.* "$DESTDIR/srchybrid/pthreads/pthreads/"
cp emule/Changelog.MorphXT.txt emule/license.txt readme.txt Packag_Licences_Readme.html "$DESTDIR"
cp -Rp eMule "$DESTDIR"
cp -Rp emule/*.*  staging
rm -rf "$DESTDIR/emule/CVS" "$DESTDIR/emule/config/CVS" 
rm -rf "$DESTDIR/emule/webserver/CVS" "$DESTDIR/emule/wapserver/CVS"
rm -rf staging/emule/CVS staging/emule/config/CVS staging/emule/webserver/CVS staging/emule/wapserver/CVS 
mkdir "$DESTDIR/emule/lang"
cp -p srchybrid/lang/dynamic/*.dll "$DESTDIR/emule/lang" 
cp -p srchybrid/lang/dynamic/*.dll staging/lang
cp -p srchybrid/Release/emule.exe "$DESTDIR/emule/eMule.exe"
cp -p srchybrid/Release/emule.exe staging/eMule.exe
cp srchybrid/Release/emule.pdb "$DESTDIR"
cd "$DESTDIR"
./ufind . -name thumbs.db -exec rm {} \;
(echo 'srchybrid/*.*' ;echo 'srchybrid/flag/*.*' ;echo 'srchybrid/flag/res24/*.*' ;echo 'srchybrid/flag/res32/*.*' ;echo 'srchybrid/kademlia/io/*.*' ;echo 'srchybrid/kademlia/kademlia/*.*' ;echo 'srchybrid/kademlia/net/*.*' ;echo 'srchybrid/kademlia/routing/*.*' ;echo 'srchybrid/kademlia/utils/*.*' ;echo 'srchybrid/lang/*.*' ;echo 'srchybrid/lang/mod/*.*' ;echo 'srchybrid/Optimizer/*.*' ;echo 'srchybrid/res/*.*' ;echo 'srchybrid/res/MOD/*.*' ;echo 'srchybrid/ToolTips/*.*' ;echo 'srchybrid/TreeOptsPrefs/*.*' ;echo 'srchybrid/WapServer/*.*' ;echo 'srchybrid/WebCache/*.*' ;echo 'Changelog.MorphXT.txt' ;echo 'license.txt') |zip -9@ "${DESTDIR}-src.zip"
zip -9rm "${DESTDIR}-src_and_libs.zip" srchybrid id3lib png Resizablelib zlib  cryptopp Changelog.MorphXT.txt license.txt README.TXT Packag_Licences_Readme.html
zip -9rm "${DESTDIR}-bin.zip" emule
zip -m emule.pdb.zip emule.pdb
cd ..



