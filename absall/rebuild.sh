export cbp2make="/home/sa/Dokumente/projects/bin/Release/cbp2make"

$cbp2make -in abs3rd.workspace -unix --keep-outdir
$cbp2make -in absall.workspace -unix --keep-outdir
$cbp2make -in abtests.workspace -unix --keep-outdir

make -f abs3rd.workspace.mak
make -f absall.workspace.mak
make -f abtests.workspace.mak
