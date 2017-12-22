rem https://www.codeproject.com/tips/551782/how-to-be-your-own-certificate

"C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Bin\makecert" -n "CN=Trill.net" -cy authority -a sha1 -sv "TillNet.pvk" -r "TrillNet.cer"
