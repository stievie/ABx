@echo off

"C:\Program Files (x86)\Windows Kits\8.0\bin\x86\makecert.exe" -n "CN=Trill.net" -ic "TrillNet.cer" -iv "TillNet.pvk" -a sha1 -sky exchange -pe -sr currentuser -ss my "TrillNetSoftware.cer"

rem C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Bin\signtool.exe sign /a /n $qTrill.net$q /t http://timestamp.comodoca.com/authenticode /d $qFW$q $f