#---------------------------------------------------------------------------
#  BLITZWAV	
#---------------------------------------------------------------------------

# packing
Arj2Arjx=$(%DEMOS_PATH)\BIN\TOOLS\arj2arjx.exe
Packer=$(%DEMOS_PATH)\BIN\EXTERN\arjbeta.exe a -m4 -e

# menus
BlsConverter=$(%DEMOS_PATH)\BIN\BLS\BlsConvert$(%TARGET).exe
MenusSrcFolder=$(%DEMOS_PATH)\BLITZIK\DATA\MENUS
MenusDstFolder=$(%DEMOS_PATH)\BLITZIK\DATABIN\MENUS

# muzik
ZiksSrcFolder=$(%DEMOS_PATH)\BLITZIK\DATA\ZIKS
ZiksDstFolder=$(%DEMOS_PATH)\BLITZIK\DATABIN\ZIKS

# layerz
PicsSrcFolder=$(%DEMOS_PATH)\BLITZIK\DATA\PICS
PicsDstFolder=$(%DEMOS_PATH)\BLITZIK\DATABIN\PICS
LayerZSrcFolder=$(%DEMOS_PATH)\BLITZIK\DATA\LAYERZ
LayerZDstFolder=$(%DEMOS_PATH)\BLITZIK\DATABIN\LAYERZ

# info
InfoSrcFolder=$(%DEMOS_PATH)\BLITZIK\DATA\INFO
InfoDstFolder=$(%DEMOS_PATH)\BLITZIK\DATABIN\INFO

# intro
IntroSrcFolder=$(%DEMOS_PATH)\BLITZIK\DATA\POLYZOOM
IntroDstFolder=$(%DEMOS_PATH)\BLITZIK\DATABIN\POLYZOOM

# binarizer
Binarizer=$(%DEMOS_PATH)\PC\Debug\BlitZikBinarize.exe

ziks: 	$(ZiksDstFolder)\1STBLIT.BLZ  &
		$(ZiksDstFolder)\1ST-KIT.BLZ  &
		$(ZiksDstFolder)\BEETHOV7.BLZ &
		$(ZiksDstFolder)\CODERS2.BLZ  &
		$(ZiksDstFolder)\LOADER3.BLZ  &
		$(ZiksDstFolder)\MEETTHEF.BLZ &
		$(ZiksDstFolder)\NEWSTEP5.BLZ &
		$(ZiksDstFolder)\NUTEK10.BLZ
		  
menus:	$(MenusDstFolder)\ECOCONC0.BIN &
        $(MenusDstFolder)\FRENCHT0.BIN &
        $(MenusDstFolder)\GWARM0.BIN   &
        $(MenusDstFolder)\LASER0.BIN   &
		$(MenusDstFolder)\GEOTECH0.BIN &
		$(MenusDstFolder)\PINKBLU0.BIN &
		$(MenusDstFolder)\ICONS.BIN	   &
		$(MenusDstFolder)\INFOS.BIN	   &
		$(MenusDstFolder)\MENUZIK0.BIN &
		$(MenusDstFolder)\AUTORUN.BIN  &
		$(MenusDstFolder)\COMPO.BIN


pics:	$(PicsDstFolder)\BLITZ.BIN	   &
		$(PicsDstFolder)\CYBER0.BIN	   &
		$(PicsDstFolder)\CYBER1.BIN	   &
		$(PicsDstFolder)\CYBER2.BIN	   &
		$(PicsDstFolder)\CYBER3.BIN	   &
		$(PicsDstFolder)\CYBER4.BIN	   &
		$(PicsDstFolder)\CYBER5.BIN	   &
		$(PicsDstFolder)\CYBER6.BIN	   &
		$(LayerZDstFolder)\SPRITE1.BIN &
		$(IntroDstFolder)\PARTY.BIN

info:	$(InfoDstFolder)\INFO.BIN	   &
		$(InfoDstFolder)\INFOCOMP.BIN  &
		$(InfoDstFolder)\VOLMASKS.BIN

exe:	$(%DEMOS_PATH)\BLITZWAO.ARJ    &
		$(%DEMOS_PATH)\OUTPUT\BLITZIK\MENU.ARJ &
		$(%DEMOS_PATH)\OUTPUT\BLITZIK\INFO.ARJ

#---------------------------------------------------------------------------
# rules ziks
#---------------------------------------------------------------------------
$(ZiksDstFolder)\1STBLIT.BLZ: 	$(ZiksSrcFolder)\1STBLIT.XM  &
								$(ZiksSrcFolder)\1STBLIT.INI &
								$(ZiksSrcFolder)\1STBLIT.SEQ &
								$(ZiksSrcFolder)\FXMACROS.TXT
	$BlsConverter $(ZiksSrcFolder)\$^&.XM -blitz -seq
	copy $(ZiksSrcFolder)\$^&.BLZ $(ZiksDstFolder)\
	copy $(ZiksSrcFolder)\$^&.BSQ $(ZiksDstFolder)\
	del $(ZiksDstFolder)\$^&.ARJ
	$(Packer) $(ZiksDstFolder)\$^&.ARJ $(ZiksDstFolder)\$^&.BLZ
	$(Arj2Arjx) $(ZiksDstFolder)\$^&.ARJ

$(ZiksDstFolder)\1ST-KIT.BLZ: 	$(ZiksSrcFolder)\1ST-KIT.XM  &
								$(ZiksSrcFolder)\1ST-KIT.INI &
								$(ZiksSrcFolder)\1ST-KIT.SEQ &
								$(ZiksSrcFolder)\FXMACROS.TXT
	$BlsConverter $(ZiksSrcFolder)\$^&.XM -blitz -seq
	copy $(ZiksSrcFolder)\$^&.BLZ $(ZiksDstFolder)\
	copy $(ZiksSrcFolder)\$^&.BSQ $(ZiksDstFolder)\
	del $(ZiksDstFolder)\$^&.ARJ
	$(Packer) $(ZiksDstFolder)\$^&.ARJ $(ZiksDstFolder)\$^&.BLZ
	$(Arj2Arjx) $(ZiksDstFolder)\$^&.ARJ
	
$(ZiksDstFolder)\BEETHOV7.BLZ: 	$(ZiksSrcFolder)\BEETHOV7.XM  &
								$(ZiksSrcFolder)\BEETHOV7.INI &
								$(ZiksSrcFolder)\BEETHOV7.SEQ &
								$(ZiksSrcFolder)\FXMACROS.TXT
	$BlsConverter $(ZiksSrcFolder)\$^&.XM -blitz -seq
	copy $(ZiksSrcFolder)\$^&.BLZ $(ZiksDstFolder)\
	copy $(ZiksSrcFolder)\$^&.BSQ $(ZiksDstFolder)\
	del $(ZiksDstFolder)\$^&.ARJ
	$(Packer) $(ZiksDstFolder)\$^&.ARJ $(ZiksDstFolder)\$^&.BLZ
	$(Arj2Arjx) $(ZiksDstFolder)\$^&.ARJ

$(ZiksDstFolder)\CODERS2.BLZ: 	$(ZiksSrcFolder)\CODERS2.XM  &
								$(ZiksSrcFolder)\CODERS2.INI &
								$(ZiksSrcFolder)\CODERS2.SEQ &
								$(ZiksSrcFolder)\FXMACROS.TXT
	$BlsConverter $(ZiksSrcFolder)\$^&.XM -blitz -seq
	copy $(ZiksSrcFolder)\$^&.BLZ $(ZiksDstFolder)\
	copy $(ZiksSrcFolder)\$^&.BSQ $(ZiksDstFolder)\
	del $(ZiksDstFolder)\$^&.ARJ
	$(Packer) $(ZiksDstFolder)\$^&.ARJ $(ZiksDstFolder)\$^&.BLZ
	$(Arj2Arjx) $(ZiksDstFolder)\$^&.ARJ
	
$(ZiksDstFolder)\LOADER3.BLZ:   $(ZiksSrcFolder)\LOADER3.XM  &
								$(ZiksSrcFolder)\LOADER3.INI &
								$(ZiksSrcFolder)\LOADER3.SEQ &
								$(ZiksSrcFolder)\FXMACROS.TXT
	$BlsConverter $(ZiksSrcFolder)\$^&.XM -blitz -seq
	copy $(ZiksSrcFolder)\$^&.BLZ $(ZiksDstFolder)\
	copy $(ZiksSrcFolder)\$^&.BSQ $(ZiksDstFolder)\	
	del $(ZiksDstFolder)\$^&.ARJ
	$(Packer) $(ZiksDstFolder)\$^&.ARJ $(ZiksDstFolder)\$^&.BLZ
	$(Arj2Arjx) $(ZiksDstFolder)\$^&.ARJ
	
$(ZiksDstFolder)\MEETTHEF.BLZ: 	$(ZiksSrcFolder)\MEETTHEF.XM  &
								$(ZiksSrcFolder)\MEETTHEF.INI &
								$(ZiksSrcFolder)\MEETTHEF.SEQ &
								$(ZiksSrcFolder)\FXMACROS.TXT
	$BlsConverter $(ZiksSrcFolder)\$^&.XM -blitz -seq
	copy $(ZiksSrcFolder)\$^&.BLZ $(ZiksDstFolder)\
	copy $(ZiksSrcFolder)\$^&.BSQ $(ZiksDstFolder)\	
	del $(ZiksDstFolder)\$^&.ARJ
	$(Packer) $(ZiksDstFolder)\$^&.ARJ $(ZiksDstFolder)\$^&.BLZ
	$(Arj2Arjx) $(ZiksDstFolder)\$^&.ARJ

$(ZiksDstFolder)\NEWSTEP5.BLZ: 	$(ZiksSrcFolder)\NEWSTEP5.XM  &
								$(ZiksSrcFolder)\NEWSTEP5.INI &
								$(ZiksSrcFolder)\NEWSTEP5.SEQ &
								$(ZiksSrcFolder)\FXMACROS.TXT
	$BlsConverter $(ZiksSrcFolder)\$^&.XM -blitz -seq
	copy $(ZiksSrcFolder)\$^&.BLZ $(ZiksDstFolder)\
	copy $(ZiksSrcFolder)\$^&.BSQ $(ZiksDstFolder)\	
	del $(ZiksDstFolder)\$^&.ARJ
	$(Packer) $(ZiksDstFolder)\$^&.ARJ $(ZiksDstFolder)\$^&.BLZ
	$(Arj2Arjx) $(ZiksDstFolder)\$^&.ARJ
	
$(ZiksDstFolder)\NUTEK10.BLZ: 	$(ZiksSrcFolder)\NUTEK10.XM  &
								$(ZiksSrcFolder)\NUTEK10.INI &
								$(ZiksSrcFolder)\NUTEK10.SEQ &
								$(ZiksSrcFolder)\FXMACROS.TXT
	$BlsConverter $(ZiksSrcFolder)\$^&.XM -blitz -seq
	copy $(ZiksSrcFolder)\$^&.BLZ $(ZiksDstFolder)\
	copy $(ZiksSrcFolder)\$^&.BSQ $(ZiksDstFolder)\	
	del $(ZiksDstFolder)\$^&.ARJ
	$(Packer) $(ZiksDstFolder)\$^&.ARJ $(ZiksDstFolder)\$^&.BLZ
	$(Arj2Arjx) $(ZiksDstFolder)\$^&.ARJ

#---------------------------------------------------------------------------
# rules menus
#---------------------------------------------------------------------------
$(MenusDstFolder)\MENUZIK0.BIN: $(MenusSrcFolder)\MENU.TXT
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 2 $(MenusSrcFolder)\MENU.TXT $^@
	del $(MenusDstFolder)\MENUZIK0.ARJ
	del $(MenusDstFolder)\MENUZIK1.ARJ
	del $(MenusDstFolder)\MENUZIK2.ARJ
	$(Packer) $(MenusDstFolder)\MENUZIK0.ARJ $(MenusDstFolder)\MENUZIK0.BIN
	$(Packer) $(MenusDstFolder)\MENUZIK1.ARJ $(MenusDstFolder)\MENUZIK1.BIN
	$(Packer) $(MenusDstFolder)\MENUZIK2.ARJ $(MenusDstFolder)\MENUZIK2.BIN
	$(Arj2Arjx) $(MenusDstFolder)\MENUZIK0.ARJ
	$(Arj2Arjx) $(MenusDstFolder)\MENUZIK1.ARJ
	$(Arj2Arjx) $(MenusDstFolder)\MENUZIK2.ARJ

$(MenusDstFolder)\ICONS.BIN: 	$(MenusSrcFolder)\ICONS4.NEO
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 1 $(MenusSrcFolder)\ICONS4.NEO $^@
	del $(MenusDstFolder)\$^&.ARJ
	$(Packer) $(MenusDstFolder)\$^&.ARJ $(MenusDstFolder)\$^&.BIN
	$(Arj2Arjx) $(MenusDstFolder)\$^&.ARJ

$(MenusDstFolder)\INFOS.BIN: 	$(MenusSrcFolder)\INFOC.NEO
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 3 $(MenusSrcFolder)\INFOC.NEO $^@
	del $(MenusDstFolder)\$^&.ARJ
	$(Packer) $(MenusDstFolder)\$^&.ARJ $(MenusDstFolder)\$^&.BIN
	$(Arj2Arjx) $(MenusDstFolder)\$^&.ARJ
	
$(MenusDstFolder)\ECOCONC0.BIN: $(MenusSrcFolder)\TEST3.BMP
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 0 $(MenusSrcFolder)\TEST3.BMP $^@
	
$(MenusDstFolder)\FRENCHT0.BIN: $(MenusSrcFolder)\TEST5.BMP
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 0 $(MenusSrcFolder)\TEST5.BMP $^@

$(MenusDstFolder)\GEOTECH0.BIN: $(MenusSrcFolder)\TEST1.BMP
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 0 $(MenusSrcFolder)\TEST1.BMP $^@

$(MenusDstFolder)\LASER0.BIN: 	$(MenusSrcFolder)\TEST2.BMP
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 0 $(MenusSrcFolder)\TEST2.BMP $^@

$(MenusDstFolder)\GWARM0.BIN: 	$(MenusSrcFolder)\TEST4.BMP
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 0 $(MenusSrcFolder)\TEST4.BMP $^@

$(MenusDstFolder)\PINKBLU0.BIN:	$(MenusSrcFolder)\TEST0.BMP
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 0 $(MenusSrcFolder)\TEST0.BMP $^@

$(MenusDstFolder)\AUTORUN.BIN:	$(MenusSrcFolder)\AUTORUN.TXT
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 11 $(MenusSrcFolder)\AUTORUN.TXT $^@

$(MenusDstFolder)\COMPO.BIN:	$(MenusSrcFolder)\COMPO.TXT
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 11 $(MenusSrcFolder)\COMPO.TXT $^@

#---------------------------------------------------------------------------
# rules pics
#---------------------------------------------------------------------------
$(PicsDstFolder)\BLITZ.BIN: 	$(PicsSrcFolder)\BLITZ.BMP
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 4 $(PicsSrcFolder)\BLITZ.BMP $^@
	del $(PicsDstFolder)\$^&.ARJ
	$(Packer) $(PicsDstFolder)\$^&.ARJ $(PicsDstFolder)\$^&.BIN
	$(Arj2Arjx) $(PicsDstFolder)\$^&.ARJ

$(PicsDstFolder)\CYBER0.BIN: 	$(PicsSrcFolder)\CYBER0.BMP
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 4 $(PicsSrcFolder)\CYBER0.BMP $^@
	del $(PicsDstFolder)\$^&.ARJ
	$(Packer) $(PicsDstFolder)\$^&.ARJ $(PicsDstFolder)\$^&.BIN
	$(Arj2Arjx) $(PicsDstFolder)\$^&.ARJ

$(PicsDstFolder)\CYBER1.BIN: 	$(PicsSrcFolder)\CYBER1.BMP
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 4 $(PicsSrcFolder)\CYBER1.BMP $^@
	del $(PicsDstFolder)\$^&.ARJ
	$(Packer) $(PicsDstFolder)\$^&.ARJ $(PicsDstFolder)\$^&.BIN
	$(Arj2Arjx) $(PicsDstFolder)\$^&.ARJ

$(PicsDstFolder)\CYBER2.BIN: 	$(PicsSrcFolder)\CYBER2.BMP
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 4 $(PicsSrcFolder)\CYBER2.BMP $^@
	del $(PicsDstFolder)\$^&.ARJ
	$(Packer) $(PicsDstFolder)\$^&.ARJ $(PicsDstFolder)\$^&.BIN
	$(Arj2Arjx) $(PicsDstFolder)\$^&.ARJ

$(PicsDstFolder)\CYBER3.BIN: 	$(PicsSrcFolder)\CYBER3.BMP
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 4 $(PicsSrcFolder)\CYBER3.BMP $^@
	del $(PicsDstFolder)\$^&.ARJ
	$(Packer) $(PicsDstFolder)\$^&.ARJ $(PicsDstFolder)\$^&.BIN
	$(Arj2Arjx) $(PicsDstFolder)\$^&.ARJ

$(PicsDstFolder)\CYBER4.BIN: 	$(PicsSrcFolder)\CYBER4.BMP
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 4 $(PicsSrcFolder)\CYBER4.BMP $^@
	del $(PicsDstFolder)\$^&.ARJ
	$(Packer) $(PicsDstFolder)\$^&.ARJ $(PicsDstFolder)\$^&.BIN
	$(Arj2Arjx) $(PicsDstFolder)\$^&.ARJ

$(PicsDstFolder)\CYBER5.BIN: 	$(PicsSrcFolder)\CYBER5.BMP
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 4 $(PicsSrcFolder)\CYBER5.BMP $^@
	del $(PicsDstFolder)\$^&.ARJ
	$(Packer) $(PicsDstFolder)\$^&.ARJ $(PicsDstFolder)\$^&.BIN
	$(Arj2Arjx) $(PicsDstFolder)\$^&.ARJ

$(PicsDstFolder)\CYBER6.BIN: 	$(PicsSrcFolder)\CYBER6.BMP
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 4 $(PicsSrcFolder)\CYBER6.BMP $^@
	del $(PicsDstFolder)\$^&.ARJ
	$(Packer) $(PicsDstFolder)\$^&.ARJ $(PicsDstFolder)\$^&.BIN
	$(Arj2Arjx) $(PicsDstFolder)\$^&.ARJ

$(LayerZDstFolder)\SPRITE1.BIN:	 $(LayerZSrcFolder)\SPRITE.NEO
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 8 $(LayerZSrcFolder)\SPRITE.NEO $^@

$(IntroDstFolder)\PARTY.BIN:	 $(IntroSrcFolder)\PARTY.BMP
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 12 $(IntroSrcFolder)\PARTY.BMP $^@
	del $(IntroDstFolder)\$^&.ARJ
	$(Packer) $(IntroDstFolder)\$^&.ARJ $(IntroDstFolder)\$^&.BIN
	$(Arj2Arjx) $(IntroDstFolder)\$^&.ARJ

#---------------------------------------------------------------------------
# rules info
#---------------------------------------------------------------------------
$(InfoDstFolder)\INFO.BIN:		$(InfoSrcFolder)\INFO.TXT
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 9 $(InfoSrcFolder)\INFO.TXT $^@
	del $(InfoDstFolder)\$^&.ARJ
	$(Packer) $(InfoDstFolder)\$^&.ARJ $(InfoDstFolder)\$^&.BIN
	$(Arj2Arjx) $(InfoDstFolder)\$^&.ARJ

$(InfoDstFolder)\INFOCOMP.BIN:		$(InfoSrcFolder)\INFOCOMP.TXT
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 9 $(InfoSrcFolder)\INFOCOMP.TXT $^@
	del $(InfoDstFolder)\$^&.ARJ
	$(Packer) $(InfoDstFolder)\$^&.ARJ $(InfoDstFolder)\$^&.BIN
	$(Arj2Arjx) $(InfoDstFolder)\$^&.ARJ

$(InfoDstFolder)\VOLMASKS.BIN:  $(InfoSrcFolder)\VOLMASKS.PI1
	$(%DEMOS_PATH)\BLITZIK\TOOLS\BATCH\Binarizer.bat 10 $(InfoSrcFolder)\VOLMASKS.PI1 $^@

#---------------------------------------------------------------------------
# rules exe
#---------------------------------------------------------------------------
$(%DEMOS_PATH)\BLITZWAO.ARJ: $(%DEMOS_PATH)\BLITZWAO.PRG
	del $^@
	$(Packer) $^@ $[@
	$(Arj2Arjx) $^@
	
$(%DEMOS_PATH)\OUTPUT\BLITZIK\MENU.ARJ: $(%DEMOS_PATH)\OUTPUT\BLITZIK\MENU.PRX
	del $^@
	$(Packer) $^@ $[@
	$(Arj2Arjx) $^@

$(%DEMOS_PATH)\OUTPUT\BLITZIK\INFO.ARJ: $(%DEMOS_PATH)\OUTPUT\BLITZIK\INFO.PRX
	del $^@
	$(Packer) $^@ $[@
	$(Arj2Arjx) $^@
