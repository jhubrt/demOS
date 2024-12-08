#---------------------------------------------------------------------------
#  RELAPSE	
#---------------------------------------------------------------------------

# packing
Arj2Arjx=$(%DEMOS_PATH)\BIN\TOOLS\arj2arjx.exe
Mod2Modx=$(%DEMOS_PATH)\BIN\TOOLS\mod2modx.exe
Packer=$(%DEMOS_PATH)\BIN\EXTERN\gup.exe a -n2 -e
PackerBoot=$(%DEMOS_PATH)\BIN\EXTERN\gup.exe a -n0 -e

# PackerBoot=$(%DEMOS_PATH)\BIN\EXTERN\arjbeta.exe a -m4 -e

DemosFolder=$(%DEMOS_PATH)\

RelapseZiksSrcFolder=$(%DEMOS_PATH)\RELAPSE\DATA\ZIKS
RelapseZiksDstFolder=$(%DEMOS_PATH)\RELAPSE\DATABIN\ZIKS

RelapseIntroSrcFolder=$(%DEMOS_PATH)\RELAPSE\DATA\INTRO
RelapseIntroDstFolder=$(%DEMOS_PATH)\RELAPSE\DATABIN\INTRO

RelapseLiquidSrcFolder=$(%DEMOS_PATH)\RELAPSE\DATA\LIQUID
RelapseLiquidDstFolder=$(%DEMOS_PATH)\RELAPSE\DATABIN\LIQUID

RelapseEgyptiaSrcFolder=$(%DEMOS_PATH)\RELAPSE\DATA\EGYPTIA
RelapseEgyptiaDstFolder=$(%DEMOS_PATH)\RELAPSE\DATABIN\EGYPTIA

RelapseGrafikSrcFolder=$(%DEMOS_PATH)\RELAPSE\DATA\GRAFIK_S
RelapseGrafikDstFolder=$(%DEMOS_PATH)\RELAPSE\DATABIN\GRAFIK_S

RelapseCascadeSrcFolder=$(%DEMOS_PATH)\RELAPSE\DATA\CASCADE
RelapseCascadeDstFolder=$(%DEMOS_PATH)\RELAPSE\DATABIN\CASCADE

RelapseSpaceFSrcFolder=$(%DEMOS_PATH)\RELAPSE\DATA\SPACEF
RelapseSpaceFDstFolder=$(%DEMOS_PATH)\RELAPSE\DATABIN\SPACEF

RelapseSplashSrcFolder=$(%DEMOS_PATH)\RELAPSE\DATA\SPLASH
RelapseSplashDstFolder=$(%DEMOS_PATH)\RELAPSE\DATABIN\SPLASH

RelapseFastmenuSrcFolder=$(%DEMOS_PATH)\RELAPSE\DATA\FASTMENU
RelapseFastmenuDstFolder=$(%DEMOS_PATH)\RELAPSE\DATABIN\FASTMENU

RelapseEndSrcFolder=$(%DEMOS_PATH)\RELAPSE\DATA\END
RelapseEndDstFolder=$(%DEMOS_PATH)\RELAPSE\DATABIN\END

RelapseInfoSrcFolder=$(%DEMOS_PATH)\RELAPSE\DATA\INFO
RelapseInfoDstFolder=$(%DEMOS_PATH)\RELAPSE\DATABIN\INFO

RelapseInterludSrcFolder=$(%DEMOS_PATH)\RELAPSE\DATA\INTERLUD
RelapseInterludDstFolder=$(%DEMOS_PATH)\RELAPSE\DATABIN\INTERLUD


# binarizer
Binarizer=$(%DEMOS_PATH)\PC\Debug\RelapseBinarize.exe
P2C=$(%DEMOS_PATH)\PC\Debug\P2C_Debug.exe


exe:	$(DemosFolder)\RELAPSEO.ARJ &
		$(DemosFolder)\OUTPUT\RELAPSE\LIQUID.ARJ &
		$(DemosFolder)\OUTPUT\RELAPSE\EGYPTIA.ARJ &
		$(DemosFolder)\OUTPUT\RELAPSE\GRAFIK_S.ARJ &
		$(DemosFolder)\OUTPUT\RELAPSE\INTERLUD.ARJ &
		$(DemosFolder)\OUTPUT\RELAPSE\CASCADE.ARJ &
		$(DemosFolder)\OUTPUT\RELAPSE\SHADE.ARJ &
		$(DemosFolder)\OUTPUT\RELAPSE\SPACEF.ARJ &
		$(DemosFolder)\OUTPUT\RELAPSE\END.ARJ &
		$(DemosFolder)\OUTPUT\RELAPSE\INFO.ARJ &
		$(DemosFolder)\OUTPUT\RELAPSE\LOADER.ARJ &
		$(DemosFolder)\OUTPUT\RELAPSE\FASTMENU.ARJ
		
ziks: 	$(RelapseZiksDstFolder)\HYDRO.ARJ &
		$(RelapseZiksDstFolder)\EGYPTZIK.ARJ &
		$(RelapseZiksDstFolder)\DELOS_04.ARJ &
		$(RelapseZiksDstFolder)\DELOS_05.ARJ &
		$(RelapseZiksDstFolder)\DELOS_0C.ARJ &
		$(RelapseZiksDstFolder)\ENDMOD3.ARJ &
		$(RelapseZiksDstFolder)\NEW_TEC7.ARJ &
		$(RelapseZiksDstFolder)\GRAFIK.ARJ &
		$(RelapseZiksDstFolder)\INFO.ARJ &
		$(RelapseZiksDstFolder)\FIRST6.ARJ &
		$(RelapseZiksDstFolder)\SHA_CASC.ARJ &
		$(RelapseZiksDstFolder)\EAGAN.ARJ 

intro:	$(RelapseIntroDstFolder)\ANIM3D.ARJ &
		$(RelapseIntroDstFolder)\RELAPSE3.ARJ &
		$(RelapseIntroDstFolder)\TITLE.ARJ

liquid:	$(RelapseLiquidDstFolder)\WATER.ARJ &
		$(RelapseLiquidDstFolder)\FONT.ARJ

egyptia: $(RelapseEgyptiaDstFolder)\EGYPTIA.ARJ &
		 $(RelapseEgyptiaDstFolder)\FONT2.ARJ

splash:	 $(RelapseSplashDstFolder)\COLORS.PAL &
		 $(RelapseSplashDstFolder)\SPLASH.ARJ

grafik:	 $(RelapseGrafikDstFolder)\GRAFIKS3.ARJ &
		 $(RelapseGrafikDstFolder)\GRAFIKS.ARJ &
		 $(RelapseGrafikDstFolder)\CIRCURVE.BIN &
		 $(RelapseGrafikDstFolder)\GS1.BIN

cascade: $(RelapseCascadeDstFolder)\CYBERFNT.ARJ &
		 $(RelapseCascadeDstFolder)\WATER1.ARJ &
		 $(RelapseCascadeDstFolder)\DEST.PAL

interlud: $(RelapseInterludDstFolder)\ART.ARJ &
		 $(RelapseInterludDstFolder)\BRIK.ARJ &
		 $(RelapseInterludDstFolder)\SIN.BIN &
		 $(RelapseInterludDstFolder)\FONT.ARJ &
		 $(RelapseInterludDstFolder)\YMSOUND0.YM &
		 $(RelapseInterludDstFolder)\YMSOUND1.YM

end:	 $(RelapseEndDstFolder)\INTRO.ARJ &
		 $(RelapseEndDstFolder)\LIQUID.ARJ &
		 $(RelapseEndDstFolder)\EGYPTIA.ARJ &
		 $(RelapseEndDstFolder)\GRAFIKS2.ARJ &
		 $(RelapseEndDstFolder)\INTERLUD.ARJ &
		 $(RelapseEndDstFolder)\CASCADE.ARJ &
		 $(RelapseEndDstFolder)\SHADE.ARJ &
		 $(RelapseEndDstFolder)\SPACE_F.ARJ &
	  	 $(RelapseEndDstFolder)\BARLOAD.ARJ &
	  	 $(RelapseEndDstFolder)\INFO.ARJ &
		 $(RelapseEndDstFolder)\THE_END.ARJ
		
spacef:	 $(RelapseSpaceFDstFolder)\IMAGE0.ARJ &
		 $(RelapseSpaceFDstFolder)\IMAGE1.ARJ &
		 $(RelapseSpaceFDstFolder)\IMAGE2.ARJ &
		 $(RelapseSpaceFDstFolder)\IMAGE3.ARJ &
		 $(RelapseSpaceFDstFolder)\IMAGE4.ARJ &
		 $(RelapseSpaceFDstFolder)\IMAGE5.ARJ &
		 $(RelapseSpaceFDstFolder)\IMAGE6.ARJ &
		 $(RelapseSpaceFDstFolder)\IMAGE7.ARJ &
		 $(RelapseSpaceFDstFolder)\IMAGE8.ARJ &
		 $(RelapseSpaceFDstFolder)\IMAGE9.ARJ &
		 $(RelapseSpaceFDstFolder)\IMAGE10.ARJ &
		 $(RelapseSpaceFDstFolder)\IMAGE11.ARJ &
		 $(RelapseSpaceFDstFolder)\NUAGES2.ARJ

info:	 $(RelapseInfoDstFolder)\INFO.ARJ &
		 $(RelapseInfoDstFolder)\FONT.ARJ &
		 $(RelapseInfoDstFolder)\COLORS.PAL

#---------------------------------------------------------------------------
# exe
#---------------------------------------------------------------------------
$(DemosFolder)\RELAPSEO.ARJ:	$(DemosFolder)\RELAPSEO.PRG
	$(PackerBoot) $^@ $^*.PRG
	$(Arj2Arjx) $^@
	
$(DemosFolder)\OUTPUT\RELAPSE\LOADER.ARJ:	$(DemosFolder)\OUTPUT\RELAPSE\LOADER.PRX
	$(Packer) $^@ $^*.PRX
	$(Arj2Arjx) $(DemosFolder)\OUTPUT\RELAPSE\$^&.ARJ

$(DemosFolder)\OUTPUT\RELAPSE\LIQUID.ARJ:	$(DemosFolder)\OUTPUT\RELAPSE\LIQUID.PRX
	$(Packer) $(DemosFolder)\OUTPUT\RELAPSE\$^&.ARJ $(DemosFolder)\OUTPUT\RELAPSE\$^&.PRX
	$(Arj2Arjx) $^@

$(DemosFolder)\OUTPUT\RELAPSE\EGYPTIA.ARJ:	$(DemosFolder)\OUTPUT\RELAPSE\EGYPTIA.PRX
	$(Packer) $^@ $^*.PRX
	$(Arj2Arjx) $^@

$(DemosFolder)\OUTPUT\RELAPSE\SHADE.ARJ:	$(DemosFolder)\OUTPUT\RELAPSE\SHADE.PRX
	$(Packer) $^@ $^*.PRX
	$(Arj2Arjx) $^@

$(DemosFolder)\OUTPUT\RELAPSE\CASCADE.ARJ:  $(DemosFolder)\OUTPUT\RELAPSE\CASCADE.PRX
	$(Packer) $^@ $^*.PRX
	$(Arj2Arjx) $^@

$(DemosFolder)\OUTPUT\RELAPSE\SPACEF.ARJ:	$(DemosFolder)\OUTPUT\RELAPSE\SPACEF.PRX
	$(Packer) $^@ $^*.PRX
	$(Arj2Arjx) $^@

$(DemosFolder)\OUTPUT\RELAPSE\END.ARJ:	$(DemosFolder)\OUTPUT\RELAPSE\END.PRX
	$(Packer) $^@ $^*.PRX
	$(Arj2Arjx) $^@

$(DemosFolder)\OUTPUT\RELAPSE\INFO.ARJ: $(DemosFolder)\OUTPUT\RELAPSE\INFO.PRX
	$(Packer) $^@ $^*.PRX
	$(Arj2Arjx) $^@

$(DemosFolder)\OUTPUT\RELAPSE\FASTMENU.ARJ:	$(DemosFolder)\OUTPUT\RELAPSE\FASTMENU.PRX
	$(Packer) $^@ $^*.PRX
	$(Arj2Arjx) $^@

$(DemosFolder)\OUTPUT\RELAPSE\INTERLUD.ARJ: $(DemosFolder)\OUTPUT\RELAPSE\INTERLUD.PRX
	$(Packer) $^@ $^*.PRX
	$(Arj2Arjx) $^@

$(DemosFolder)\OUTPUT\RELAPSE\GRAFIK_S.ARJ: $(DemosFolder)\OUTPUT\RELAPSE\GRAFIK_S.PRX
	$(Packer) $^@ $^*.PRX
	$(Arj2Arjx) $^@

#---------------------------------------------------------------------------
# rules Grafik Sound
#---------------------------------------------------------------------------

$(RelapseGrafikDstFolder)\GRAFIKS.ARJ: $(RelapseGrafikSrcFolder)\GRAFIKS.PI1
	del $^@
	$(P2C) $(RelapseGrafikSrcFolder)\$^&.PI1 $^*.C4V
	$(Packer) $^@ $(RelapseGrafikDstFolder)\$^&.C4V
	$(Arj2Arjx) $^@

$(RelapseGrafikDstFolder)\CIRCURVE.BIN: $(Binarizer)
	$(Binarizer) 7 _ $(RelapseGrafikDstFolder)\CIRCURVE.BIN

$(RelapseGrafikDstFolder)\GRAFIKS3.ARJ: $(RelapseGrafikSrcFolder)\GRAFIKS3.NEO
	del $^@
	$(P2C) $(RelapseGrafikSrcFolder)\$^&.NEO $^*.C4V
	$(Packer) $^@ $(RelapseGrafikDstFolder)\$^&.C4V
	$(Arj2Arjx) $^@

$(RelapseGrafikDstFolder)\GS1.BIN: $(RelapseGrafikSrcFolder)\GS1_img5.BMP
	$(Binarizer) 8 $(RelapseGrafikSrcFolder)\GS1_img5.BMP $^@

#---------------------------------------------------------------------------
# rules Cascade
#---------------------------------------------------------------------------
$(RelapseCascadeDstFolder)\DEST.PAL: $(RelapseCascadeSrcFolder)\DEST.PAL
	copy $(RelapseCascadeSrcFolder)\$^&.PAL $^@

$(RelapseCascadeDstFolder)\CYBERFNT.ARJ: $(RelapseCascadeSrcFolder)\CYBERFN1.NEO
	del $^@
	$(Binarizer) 5 $(RelapseCascadeSrcFolder)\CYBERFN1.NEO $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseCascadeDstFolder)\WATER1.ARJ: $(RelapseCascadeSrcFolder)\WATER.BMP
	del $(RelapseCascadeDstFolder)\WATER1.ARJ
	del $(RelapseCascadeDstFolder)\WATER2.ARJ
	$(Binarizer) 3 $(RelapseCascadeSrcFolder)\WATER.BMP $(RelapseCascadeDstFolder)\WATER.4B
	$(Packer) $(RelapseCascadeDstFolder)\WATER1.ARJ $(RelapseCascadeDstFolder)\WATER.4B
	$(Packer) $(RelapseCascadeDstFolder)\WATER2.ARJ $(RelapseCascadeDstFolder)\WATER.4B_
	$(Arj2Arjx) $(RelapseCascadeDstFolder)\WATER1.ARJ
	$(Arj2Arjx) $(RelapseCascadeDstFolder)\WATER2.ARJ

#---------------------------------------------------------------------------
# rules Interlude
#---------------------------------------------------------------------------
$(RelapseInterludDstFolder)\ART.ARJ:
	copy $(RelapseInterludSrcFolder)\$^&.PCM $(RelapseInterludDstFolder)\
	$(Mod2Modx) $^*.PCM
	del $^@
	$(Packer) $^@ $^*.PCMX
	$(Arj2Arjx) $^@

$(RelapseInterludDstFolder)\BRIK.ARJ:
	$(Binarizer) 6 $(RelapseInterludSrcFolder)\$^&.NEO $^*.BIN
	$(Packer) $^@ $^*.BIN
	$(Arj2Arjx) $^@
	$(Packer) $(RelapseInterludDstFolder)\MOVE.ARJ $(RelapseInterludDstFolder)\MOVE.BIN
	$(Arj2Arjx) $(RelapseInterludDstFolder)\MOVE.ARJ

$(RelapseInterludDstFolder)\FONT.ARJ: $(RelapseInterludSrcFolder)\FONT.NEO
	del $^@
	$(P2C) -size1360 $(RelapseInterludSrcFolder)\$^&.NEO $^*.1P
	$(Packer) $^@ $(RelapseInterludDstFolder)\$^&.1P
	$(Arj2Arjx) $^@
	
$(RelapseInterludDstFolder)\SIN.BIN:
	copy $(RelapseInterludSrcFolder)\$^&.BIN $^@

$(RelapseInterludDstFolder)\YMSOUND0.YM:
	copy $(RelapseInterludSrcFolder)\$^&.YM $^@

$(RelapseInterludDstFolder)\YMSOUND1.YM:
	copy $(RelapseInterludSrcFolder)\$^&.YM $^@

#---------------------------------------------------------------------------
# rules Splash
#---------------------------------------------------------------------------
$(RelapseSplashDstFolder)\COLORS.PAL:	$(RelapseSplashSrcFolder)\COLORS.PAL
	copy $(RelapseSplashSrcFolder)\$^&.PAL $^@

$(RelapseSplashDstFolder)\SPLASH.ARJ:	$(RelapseSplashSrcFolder)\SPLASH.TXT
	$(Packer) $^@ $(RelapseSplashSrcFolder)\$^&.TXT
	$(Arj2Arjx) $^@

#---------------------------------------------------------------------------
# rules Intro
#---------------------------------------------------------------------------
$(RelapseIntroDstFolder)\ANIM3D.ARJ: $(RelapseIntroSrcFolder)\ANIM3D.BIN
	del $^@
	$(Packer) $^@ $(RelapseIntroSrcFolder)\$^&.BIN
	$(Arj2Arjx) $^@

$(RelapseIntroDstFolder)\RELAPSE3.ARJ: $(RelapseIntroSrcFolder)\RELAPSE3.BMP
	del $^@
	$(Binarizer) 4 $(RelapseIntroSrcFolder)\$^&.BMP $^*.1P
	$(Packer) $^@ $^*.1P
	$(Arj2Arjx) $^@

$(RelapseIntroDstFolder)\TITLE.ARJ: $(RelapseIntroSrcFolder)\TITLE.BMP
	del $^@
	$(P2C) $(RelapseIntroSrcFolder)\$^&.BMP $^*.4B
	$(Packer) $^@ $(RelapseIntroDstFolder)\$^&.4B
	$(Arj2Arjx) $^@

#---------------------------------------------------------------------------
# rules Liquid
#---------------------------------------------------------------------------
$(RelapseLiquidDstFolder)\WATER.ARJ: $(RelapseLiquidSrcFolder)\WATER.BIN
	del $^@
	$(P2C) $(RelapseLiquidSrcFolder)\$^&.BIN $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseLiquidDstFolder)\FONT.ARJ: $(RelapseLiquidSrcFolder)\FONT.PI1
	del $^@
	$(Binarizer) 2 $(RelapseLiquidSrcFolder)\$^&.PI1 $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

#---------------------------------------------------------------------------
# rules Egyptia
#---------------------------------------------------------------------------
$(RelapseEgyptiaDstFolder)\EGYPTIA.ARJ:	$(RelapseEgyptiaSrcFolder)\EGYPTIA.NEO
	del $^@
	$(P2C) $(RelapseEgyptiaSrcFolder)\$^&.NEO $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseEgyptiaDstFolder)\FONT2.ARJ:	$(RelapseEgyptiaSrcFolder)\FONT2.NEO
	del $^@
	$(P2C) -nopal $(RelapseEgyptiaSrcFolder)\$^&.NEO $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

#---------------------------------------------------------------------------
# rules SpaceF
#---------------------------------------------------------------------------

$(RelapseSpaceFDstFolder)\IMAGE0.ARJ: $(RelapseSpaceFSrcFolder)\IMAGE0.PI1
	$(P2C) $(RelapseSpaceFSrcFolder)\$^&.PI1 $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseSpaceFDstFolder)\IMAGE1.ARJ: $(RelapseSpaceFSrcFolder)\IMAGE1.PI1
	$(P2C) $(RelapseSpaceFSrcFolder)\$^&.PI1 $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseSpaceFDstFolder)\IMAGE2.ARJ: $(RelapseSpaceFSrcFolder)\IMAGE2.PI1
	$(P2C) $(RelapseSpaceFSrcFolder)\$^&.PI1 $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseSpaceFDstFolder)\IMAGE3.ARJ: $(RelapseSpaceFSrcFolder)\IMAGE3.PI1
	$(P2C) $(RelapseSpaceFSrcFolder)\$^&.PI1 $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseSpaceFDstFolder)\IMAGE4.ARJ: $(RelapseSpaceFSrcFolder)\IMAGE4.PI1
	$(P2C) $(RelapseSpaceFSrcFolder)\$^&.PI1 $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseSpaceFDstFolder)\IMAGE5.ARJ: $(RelapseSpaceFSrcFolder)\IMAGE5.PI1
	$(P2C) $(RelapseSpaceFSrcFolder)\$^&.PI1 $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseSpaceFDstFolder)\IMAGE6.ARJ: $(RelapseSpaceFSrcFolder)\IMAGE6.PI1
	$(P2C) $(RelapseSpaceFSrcFolder)\$^&.PI1 $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseSpaceFDstFolder)\IMAGE7.ARJ: $(RelapseSpaceFSrcFolder)\IMAGE7.PI1
	$(P2C) $(RelapseSpaceFSrcFolder)\$^&.PI1 $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseSpaceFDstFolder)\IMAGE8.ARJ: $(RelapseSpaceFSrcFolder)\IMAGE8.PI1
	$(P2C) $(RelapseSpaceFSrcFolder)\$^&.PI1 $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseSpaceFDstFolder)\IMAGE9.ARJ: $(RelapseSpaceFSrcFolder)\IMAGE9.PI1
	$(P2C) $(RelapseSpaceFSrcFolder)\$^&.PI1 $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseSpaceFDstFolder)\IMAGE10.ARJ: $(RelapseSpaceFSrcFolder)\IMAGE10.PI1
	$(P2C) $(RelapseSpaceFSrcFolder)\$^&.PI1 $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseSpaceFDstFolder)\IMAGE11.ARJ: $(RelapseSpaceFSrcFolder)\IMAGE11.PI1
	$(P2C) $(RelapseSpaceFSrcFolder)\$^&.PI1 $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseSpaceFDstFolder)\NUAGES2.ARJ: $(RelapseSpaceFSrcFolder)\NUAGES2.BIN
	$(P2C) $(RelapseSpaceFSrcFolder)\$^&.BIN $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

#---------------------------------------------------------------------------
# rules End
#---------------------------------------------------------------------------

$(RelapseEndDstFolder)\INTRO.ARJ: $(RelapseEndSrcFolder)\INTRO.NEO
	$(P2C) $(RelapseEndSrcFolder)\$^&.NEO $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseEndDstFolder)\LIQUID.ARJ: $(RelapseEndSrcFolder)\LIQUID.NEO
	$(P2C) $(RelapseEndSrcFolder)\$^&.NEO $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseEndDstFolder)\EGYPTIA.ARJ:	$(RelapseEndSrcFolder)\EGYPTIA.NEO
	$(P2C) $(RelapseEndSrcFolder)\$^&.NEO $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseEndDstFolder)\GRAFIKS2.ARJ: $(RelapseEndSrcFolder)\GRAFIKS2.NEO
	$(P2C) $(RelapseEndSrcFolder)\$^&.NEO $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseEndDstFolder)\INTERLUD.ARJ: $(RelapseEndSrcFolder)\INTERLUD.NEO
	$(P2C) $(RelapseEndSrcFolder)\$^&.NEO $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseEndDstFolder)\CASCADE.ARJ:	$(RelapseEndSrcFolder)\CASCADE.NEO
	$(P2C) $(RelapseEndSrcFolder)\$^&.NEO $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseEndDstFolder)\SHADE.ARJ:	$(RelapseEndSrcFolder)\SHADE.NEO
	$(P2C) $(RelapseEndSrcFolder)\$^&.NEO $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseEndDstFolder)\SPACE_F.ARJ: $(RelapseEndSrcFolder)\SPACE_F.NEO
	$(P2C) $(RelapseEndSrcFolder)\$^&.NEO $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseEndDstFolder)\BARLOAD.ARJ:	$(RelapseEndSrcFolder)\BARLOAD.NEO
	$(P2C) $(RelapseEndSrcFolder)\$^&.NEO $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseEndDstFolder)\INFO.ARJ:	$(RelapseEndSrcFolder)\INFO.NEO
	$(P2C) $(RelapseEndSrcFolder)\$^&.NEO $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseEndDstFolder)\THE_END.ARJ: $(RelapseEndSrcFolder)\THE_END.NEO
	$(P2C) $(RelapseEndSrcFolder)\$^&.NEO $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

#---------------------------------------------------------------------------
# rules info
#---------------------------------------------------------------------------
$(RelapseInfoDstFolder)\INFO.ARJ: $(RelapseInfoSrcFolder)\INFO.TXT
	$(Packer) $^@ $(RelapseInfoSrcFolder)\$^&.TXT
	$(Arj2Arjx) $^@

$(RelapseInfoDstFolder)\FONT.ARJ: $(RelapseLiquidSrcFolder)\FONT.PI1
	del $^@
	$(Binarizer) 2 $(RelapseLiquidSrcFolder)\$^&.PI1 $^*.4B
	$(Packer) $^@ $^*.4B
	$(Arj2Arjx) $^@

$(RelapseInfoDstFolder)\COLORS.PAL: $(RelapseInfoSrcFolder)\COLORS.PAL
	copy $(RelapseInfoSrcFolder)\$^&.PAL $^@

#---------------------------------------------------------------------------
# rules ziks
#---------------------------------------------------------------------------
$(RelapseZiksDstFolder)\HYDRO.ARJ: $(RelapseZiksSrcFolder)\HYDRO.MOD
	copy $(RelapseZiksSrcFolder)\$^&.MOD $(RelapseZiksDstFolder)\
	$(Mod2Modx) $^*.MOD
	del $^@
	$(Packer) $^@ $^*.MODX
	$(Arj2Arjx) $^@

$(RelapseZiksDstFolder)\EGYPTZIK.ARJ: $(RelapseZiksSrcFolder)\EGYPTZIK.MOD
	copy $(RelapseZiksSrcFolder)\$^&.MOD $(RelapseZiksDstFolder)\
	$(Mod2Modx) $^*.MOD
	del $^@
	$(Packer) $^@ $^*.MODX
	$(Arj2Arjx) $^@

$(RelapseZiksDstFolder)\DELOS_04.ARJ: $(RelapseZiksSrcFolder)\DELOS_04.MOD
	copy $(RelapseZiksSrcFolder)\$^&.MOD $(RelapseZiksDstFolder)\
	$(Mod2Modx) $^*.MOD
	del $^@
	$(Packer) $^@ $^*.MODX
	$(Arj2Arjx) $^@

$(RelapseZiksDstFolder)\DELOS_05.ARJ: $(RelapseZiksSrcFolder)\DELOS_05.MOD
	copy $(RelapseZiksSrcFolder)\$^&.MOD $(RelapseZiksDstFolder)\
	$(Mod2Modx) $^*.MOD
	del $^@
	$(Packer) $^@ $^*.MODX
	$(Arj2Arjx) $^@

$(RelapseZiksDstFolder)\DELOS_0C.ARJ: $(RelapseZiksSrcFolder)\DELOS_0C.MOD
	copy $(RelapseZiksSrcFolder)\$^&.MOD $(RelapseZiksDstFolder)\
	$(Mod2Modx) $^*.MOD
	del $^@
	$(Packer) $^@ $^*.MODX
	$(Arj2Arjx) $^@

$(RelapseZiksDstFolder)\ENDMOD3.ARJ: $(RelapseZiksSrcFolder)\ENDMOD3.MOD
	copy $(RelapseZiksSrcFolder)\$^&.MOD $(RelapseZiksDstFolder)\
	$(Mod2Modx) $^*.MOD
	del $^@
	$(Packer) $^@ $^*.MODX
	$(Arj2Arjx) $^@

$(RelapseZiksDstFolder)\NEW_TEC7.ARJ: $(RelapseZiksSrcFolder)\NEW_TEC7.MOD
	copy $(RelapseZiksSrcFolder)\$^&.MOD $(RelapseZiksDstFolder)\
	$(Mod2Modx) $^*.MOD
	del $^@
	$(Packer) $^@ $^*.MODX
	$(Arj2Arjx) $^@
	
$(RelapseZiksDstFolder)\GRAFIK.ARJ: $(RelapseZiksSrcFolder)\GRAFIK.MOD
	copy $(RelapseZiksSrcFolder)\$^&.MOD $(RelapseZiksDstFolder)\
	$(Mod2Modx) $^*.MOD
	del $^@
	$(Packer) $^@ $^*.MODX
	$(Arj2Arjx) $^@

$(RelapseZiksDstFolder)\INFO.ARJ: $(RelapseZiksSrcFolder)\INFO.MOD
	copy $(RelapseZiksSrcFolder)\$^&.MOD $(RelapseZiksDstFolder)\
	$(Mod2Modx) $^*.MOD
	del $^@
	$(Packer) $^@ $^*.MODX
	$(Arj2Arjx) $^@
	
$(RelapseZiksDstFolder)\FIRST6.ARJ: $(RelapseZiksSrcFolder)\FIRST6.MOD
	copy $(RelapseZiksSrcFolder)\$^&.MOD $(RelapseZiksDstFolder)\
	$(Mod2Modx) $^*.MOD
	del $^@
	$(Packer) $^@ $^*.MODX
	$(Arj2Arjx) $^@

$(RelapseZiksDstFolder)\SHA_CASC.ARJ: $(RelapseZiksSrcFolder)\SHA_CASC.MOD
	copy $(RelapseZiksSrcFolder)\$^&.MOD $(RelapseZiksDstFolder)\
	$(Mod2Modx) $^*.MOD
	del $^@
	$(Packer) $^@ $^*.MODX
	$(Arj2Arjx) $^@

$(RelapseZiksDstFolder)\EAGAN.ARJ: $(RelapseZiksSrcFolder)\EAGAN.MOD
	copy $(RelapseZiksSrcFolder)\$^&.MOD $(RelapseZiksDstFolder)\
	$(Mod2Modx) $^*.MOD
	del $^@
	$(Packer) $^@ $^*.MODX
	$(Arj2Arjx) $^@
