OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)

MEMORY {
	rom	: ORIGIN = 0x08000000, LENGTH = 32M
	ewram	: ORIGIN = 0x02000000, LENGTH = 4M - 4k
	dtcm	: ORIGIN = 0x0b000000, LENGTH = 16K
	/* I hope this will allow some "overflow" */
	vectors	: ORIGIN = 0x01ff8000, LENGTH = 256
	itcm	: ORIGIN = 0x01ff8100, LENGTH = 32k - 256
}

__vectors_start	=	ORIGIN(vectors);
__itcm_start	=	ORIGIN(itcm);
__ewram_end	=	ORIGIN(ewram) + LENGTH(ewram);
__eheap_end	=	ORIGIN(ewram) + LENGTH(ewram);
__dtcm_start	=	ORIGIN(dtcm);
__dtcm_top	=	ORIGIN(dtcm) + LENGTH(dtcm);
__irq_flags	=	__dtcm_top - 0x08;
__irq_vector	=	__dtcm_top - 0x04;

__sp_svc	=	__dtcm_top - 0x100;
__sp_irq	=	__sp_svc - 0x100;
__sp_usr	=	__sp_irq - 0x100;

SECTIONS
{
	.EOVR_Header	:
	{
		Image_EOVR_Header_Base = . ;
		ProgramID.o (.OverlayHeader)
	}
	.EOVR_Romeo2NCD	:
	{
		Image_EOVR_Romeo2NCD_Base = . ;
		romeo2_ncd.o (.rodata .text)
	}
	.EOVR_AfterSystem	:
	{
		Image_EOVR_AfterSystem_Base = . ;
		* (.OVR_AfterSystem)
		
		/* GLIB staff*/
		cglfont.o (.rodata .text)
		cglstream.o (.rodata .text)
		cgltgf.o (.rodata .text)
		
		/* libs/FileSystem staff*/
		cstream.o (.rodata .text)
		cstream_fs.o (.rodata .text)
		/* shell.o (.rodata .text)*/
		shell_SystemAfter.o (.rodata .text)
		
		/* libs/DLL staff*/
		dll.o (.rodata .text)
		dllsound.o (.rodata .text)
		playlist.o (.rodata .text)
		plug_lyric.o (.rodata .text)
		fpga_helper.o (.rodata .text)
		
		/* libs/Image staff*/
		BMPReader.o (.rodata .text)
		cfont.o (.rodata .text)
		component.o (.rodata .text)
		ErrorDialog.o (.rodata .text)
		/*ExifCache.o (.rodata .text)*/
		msgwin.o (.rodata .text)
		particle.o (.rodata .text)
		rect.o (.rodata .text)
		skin.o (.rodata .text)
		splash.o (.rodata .text)
		VideoResume.o (.rodata .text)
		vramcache.o (.rodata .text)
		
		/* libs/Memory staff*/
		extmem.o (.rodata .text)
		extmem_DSBM.o (.rodata .text)
		extmem_EZ3in1.o (.rodata .text)
		extmem_M3ExtPack.o (.rodata .text)
		extmem_RawMem.o (.rodata .text)
		extmem_SuperCard.o (.rodata .text)
		/* memtool.o (.rodata .text)*/
		
		/* libs/ndsrom*/
		BootROM.o (.rodata .text)
		extlink.o (.rodata .text)
		NDSROMIcon.o (.rodata .text)
		
		/* libs/SaveState staff*/
		launchstate.o (.rodata .text)
		procstate.o (.rodata .text)
		resume.o (.rodata .text)
		
		/* libs/Sound staff*/
		sndeff.o (.rodata .text)
		strpcm.o (.rodata .text)
		snd_click_long_c_bin.o (.rodata .text)
		snd_click_short_c_bin.o (.rodata .text)
		
		/* libs/System staff*/
		datetime.o (.rodata .text)
		/* OverlayManager.o (.rodata .text)*/
		/* zlibhelp.o (.rodata .text)*/
		/* _console.o (.rodata .text)*/
		/* _consoleWriteLog.o (.rodata .text)*/
		md5c.o (.rodata .text)
		fastlzss16decpalasm_body.o (.rodata .text)
		fastlzss16decpalasm.o (.rodata .text)
		
		/*; libs/Text staff*/
		euc2unicode.o (.rodata .text)
		lang.o (.rodata .text)
		/*; strtool.o (.rodata .text)*/
		/*; unicode.o (.rodata .text)*/
	}
	.EOVR_proc_MainPass	:
	{
		Image_EOVR_proc_MainPass_Base = . ;
		* (.IWRAM_proc_MainPass)
		inifile.o (.rodata .text)
	}
	.EOVR_proc_ChkDsk	:
	{
		Image_EOVR_proc_ChkDsk_Base = . ;
		proc_chkdsk.o (.rodata .text)
		* (.IWRAM_proc_ChkDsk)
	}
	.EOVR_proc_Setup	:
	{
		Image_EOVR_proc_Setup_Base = . ;
		proc_setup.o (.rodata .text)
		* (.IWRAM_proc_Setup)
	}
	.EOVR_proc_FileList	:
	{
		Image_EOVR_proc_FileList_Base = . ;
		proc_FileList.o (.rodata .text)
		* (.IWRAM_proc_FileList)
		NDSFiles.o (.rodata .text)
		hiddenpaths.o (.rodata .text)
		/*;MemPool.o (.rodata .text)*/
		ThumbDPG.o (.rodata .text)
		/*;ThumbExif.o (.rodata .text)*/
		ThumbIPK.o (.rodata .text)
		/*;ThumbM4A.o (.rodata .text)*/
		cipk_simple.o (.rodata .text)
		* (.IWRAM_proc_AudioPlay)
	}
	.EOVR_proc_SysMenu	:
	{
		Image_EOVR_proc_SysMenu_Base = . ;
		proc_sysmenu.o (.rodata .text)
		* (.IWRAM_proc_SysMenu)
	}
	.EOVR_proc_DPGCustom	:
	{
		Image_EOVR_proc_DPGCustom_Base = . ;
		proc_DPGCustom.o (.rodata .text)
		* (.IWRAM_proc_DPGCustom)
	}
	.EOVR_proc_DPGPlay	:
	{
		Image_EOVR_proc_DPGPlay_Base = . ;
		proc_DPGPlay.o (.rodata .text)
		* (.IWRAM_proc_DPGPlay)
	}
	.EOVR_proc_ImageCustom	:
	{
		Image_EOVR_proc_ImageCustom_Base = . ;
		proc_ImageCustom.o (.rodata .text)
		* (.IWRAM_proc_ImageCustom)
	}
	.EOVR_proc_ImageView	:
	{
		Image_EOVR_proc_ImageView_Base = . ;
		proc_ImageView.o (.rodata .text)
		* (.IWRAM_proc_ImageView)
	}
	.EOVR_proc_TextCustom	:
	{
		Image_EOVR_proc_TextCustom_Base = . ;
		proc_TextCustom.o (.rodata .text)
		* (.IWRAM_proc_TextCustom)
	}
	.EOVR_proc_TextMenu	:
	{
		Image_EOVR_proc_TextMenu_Base = . ;
		proc_TextMenu.o (.rodata .text)
		* (.IWRAM_proc_TextMenu)
	}
	.EOVR_proc_TextView	:
	{
		Image_EOVR_proc_TextView_Base = . ;
		proc_TextView.o (.rodata .text)
		* (.IWRAM_proc_TextView)
		cctf.o (.rodata .text)
		cctf_dfs.o (.rodata .text)
	}
	.EOVR_proc_BinView	:
	{
		Image_EOVR_proc_BinView_Base = . ;
		proc_BinView.o (.rodata .text)
		* (.IWRAM_proc_BinView)
	}
	.EOVR_proc_Launch	:
	{
		Image_EOVR_proc_Launch_Base = . ;
		proc_Launch.o (.rodata .text)
		* (.IWRAM_proc_Launch)
	}
	.EOVR_proc_Custom	:
	{
		Image_EOVR_proc_Custom_Base = . ;
		proc_Custom.o (.rodata .text)
		* (.IWRAM_proc_Custom)
	}
	.EOVR_proc_BootROM	:
	{
		Image_EOVR_proc_BootROM_Base = . ;
		proc_BootROM.o (.rodata .text)
		* (.IWRAM_proc_BootROM)
	}
	.EOVR_proc_MemoEdit	:
	{
		Image_EOVR_proc_MemoEdit_Base = . ;
		proc_MemoEdit.o (.rodata .text)
		* (.IWRAM_proc_MemoEdit)
	}
	.EOVR_proc_MemoList	:
	{
		Image_EOVR_proc_MemoList_Base = . ;
		proc_MemoList.o (.rodata .text)
		* (.IWRAM_proc_MemoList)
	}
	.EOVR_proc_AudioCustom	:
	{
		Image_EOVR_proc_AudioCustom_Base = . ;
		proc_AudioCustom.o (.rodata .text)
		* (IWRAM_proc_AudioCustom)
	}
	.EOVR_libsnd_mp3	:
	{
		Image_EOVR_libsnd_mp3_Base = . ;
		plug_mp3.o (.rodata .text)
		bit.o (.rodata .text)
		decoder.o (.rodata .text)
		fixed.o (.rodata .text)
		frame.o (.rodata .text)
		huffman.o (.rodata .text)
		layer12.o (.rodata .text)
		layer3.o (.rodata .text)
		stream.o (.rodata .text)
		synth.o (.rodata .text)
		timer.o (.rodata .text)
		version.o (.rodata .text)
	}
	.EOVR_libsnd_midi	:
	{
		Image_EOVR_libsnd_midi_Base = . ;
		plug_midi.o (.rodata .text)
		mtrk.o (.rodata .text)
		pch.o (.rodata .text)
		rcplib.o (.rodata .text)
		smidlib.o (.rodata .text)
		smidlib_sm.o (.rodata .text)
		sndfont.o (.rodata .text)
	}
	.EOVR_libsnd_gme	:
	{
		Image_EOVR_libsnd_gme_Base = . ;
		plug_gme.o (.rodata .text)
		Ay_Apu.o (.rodata .text)
		Ay_Cpu.o (.rodata .text)
		Ay_Emu.o (.rodata .text)
		Gbs_Emu.o (.rodata .text)
		Gb_Apu.o (.rodata .text)
		Gb_Cpu.o (.rodata .text)
		Gb_Oscs.o (.rodata .text)
		gme.o (.rodata .text)
		Hes_Apu.o (.rodata .text)
		Hes_Cpu.o (.rodata .text)
		Hes_Emu.o (.rodata .text)
		Kss_Cpu.o (.rodata .text)
		Kss_Emu.o (.rodata .text)
		Kss_Scc_Apu.o (.rodata .text)
		Nes_Apu.o (.rodata .text)
		Nes_Cpu.o (.rodata .text)
		Nes_Fme7_Apu.o (.rodata .text)
		Nes_Namco_Apu.o (.rodata .text)
		Nes_Oscs.o (.rodata .text)
		Nes_Vrc6_Apu.o (.rodata .text)
		Nsf_Emu.o (.rodata .text)
		Sap_Apu.o (.rodata .text)
		Sap_Cpu.o (.rodata .text)
		Sap_Emu.o (.rodata .text)
		Sms_Apu.o (.rodata .text)
	}
	.EOVR_libsnd_ogg	:
	{
		Image_EOVR_libsnd_ogg_Base = . ;
		plug_ogg.o (.rodata .text)
		bitwise.o (.rodata .text)
		codebook.o (.rodata .text)
		floor0.o (.rodata .text)
		floor1.o (.rodata .text)
		framing.o (.rodata .text)
		info.o (.rodata .text)
		mapping0.o (.rodata .text)
		_libogg_misc.o (.rodata .text)
		res012.o (.rodata .text)
		vorbisfile.o (.rodata .text)
		_libogg_dsp.o (.rodata .text)
		_libogg_mdct.o (.rodata .text)
	}
	.EOVR_libsnd_wave	:
	{
		Image_EOVR_libsnd_wave_Base = . ;
		plug_wav.o (.rodata .text)
	}
	.EOVR_libimg_jpeg	:
	{
		Image_EOVR_libimg_jpeg_Base = . ;
		plug_jpeg.o (.rodata .text)
		jcomapi.o (.rodata .text)
		jdapimin.o (.rodata .text)
		jdapistd.o (.rodata .text)
		jdatasrc.o (.rodata .text)
		jdcoefct.o (.rodata .text)
		jdcolor.o (.rodata .text)
		jddctmgr.o (.rodata .text)
		jdhuff.o (.rodata .text)
		jdinput.o (.rodata .text)
		jdmainct.o (.rodata .text)
		jdmarker.o (.rodata .text)
		jdmaster.o (.rodata .text)
		jdmerge.o (.rodata .text)
		jdphuff.o (.rodata .text)
		jdpostct.o (.rodata .text)
		jdsample.o (.rodata .text)
		
		jerror.o (.rodata .text)
		jidctfst.o (.rodata .text)
		jidctred.o (.rodata .text)
		
		jmemansi.o (.rodata .text)
		jmemmgr.o (.rodata .text)
		jquant1.o (.rodata .text)
		jutils.o (.rodata .text)
	}
	.EOVR_libimg_png	:
	{
		Image_EOVR_libimg_png_Base = . ;
		plug_png.o (.rodata .text)
		png.o (.rodata .text)
		pngerror.o (.rodata .text)
		pngget.o (.rodata .text)
		pngmem.o (.rodata .text)
		pngread.o (.rodata .text)
		pngrio.o (.rodata .text)
		pngrtran.o (.rodata .text)
		pngrutil.o (.rodata .text)
		pngset.o (.rodata .text)
		pngtrans.o (.rodata .text)
	}
	.EOVR_libimg_bmp	:
	{
		Image_EOVR_libimg_bmp_Base = . ;
		plug_bmp.o (.rodata .text)
	}
	.EOVR_libimg_gif	:
	{
		Image_EOVR_libimg_gif_Base = . ;
		plug_gif.o (.rodata .text)
		dgif_lib.o (.rodata .text)
		gif_err.o (.rodata .text)
		gifalloc.o (.rodata .text)
	}
	.EOVR_libimg_psd	:
	{
		Image_EOVR_libimg_psd_Base = . ;
		plug_psd.o (.rodata .text)
	}
	.EOVR_libglobal_dpg	:
	{
		Image_EOVR_libglobal_dpg_Base = . ;
		alloc.o (.rodata .text)
		clibdpg.o (.rodata .text)
		clibmpg.o (.rodata .text)
		decode.o (.rodata .text)
		header.o (.rodata .text)
		idct.o (.rodata .text)
		motion_comp.o (.rodata .text)
		motion_comp_arm.o (.rodata .text)
		plug_dpg.o (.rodata .text)
		plug_mp2.o (.rodata .text)
		slice.o (.rodata .text)
		_dpgfs.o (.rodata .text)
		plug_srttitle.o (.rodata .text)
	}
	.EOVR_libglobal_ndsrom	:
	{
		Image_EOVR_libglobal_ndsrom_Base = . ;
		*(.EOVR_libglobal_ndsrom)
		plug_ndsrom.o (.rodata .text)
	}
	.EOVR_AfterSystem2	:
	{
		Image_EOVR_AfterSystem2_Base = . ;
		* (.OVR_AfterSystem2)
	}
	.EOVR_ITCM_libglobal_dpg	:
	{
		Image_EOVR_ITCM_libglobal_dpg_Base = . ;
		* (.ITCM_libglobal_dpg)
	}
	.EOVR_ITCM_libglobal_ndsrom	:
	{
		Image_EOVR_ITCM_libglobal_ndsrom_Base = . ;
		* (.ITCM_libglobal_ndsrom)
	}
	.EOVR_ITCM_libimg_jpeg	:
	{
		Image_EOVR_ITCM_libimg_jpeg_Base = . ;
		* (.ITCM_libimg_jpeg)
	}
	.EOVR_ITCM_libimg_png	:
	{
		Image_EOVR_ITCM_libimg_png_Base = . ;
		* (.ITCM_libimg_png)
	}
	.EOVR_ITCM_libimg_bmp	:
	{
		Image_EOVR_ITCM_libimg_bmp_Base = . ;
		* (.ITCM_libimg_bmp)
	}
	.EOVR_ITCM_libimg_gif	:
	{
		Image_EOVR_ITCM_libimg_gif_Base = . ;
		* (.ITCM_libimg_gif)
	}
	.EOVR_ITCM_libimg_psd	:
	{
		Image_EOVR_ITCM_libimg_psd_Base = . ;
		* (.ITCM_libimg_psd)
	}
	.EOVR_ITCM_libsnd_mp3	:
	{
		Image_EOVR_ITCM_libsnd_mp3_Base = . ;
		* (.ITCM_libsnd_mp3)
	}
	.EOVR_ITCM_libsnd_ogg	:
	{
		Image_EOVR_ITCM_libsnd_ogg_Base = . ;
		* (.ITCM_libsnd_ogg)
	}
	.EOVR_ITCM_libsnd_gme	:
	{
		Image_EOVR_ITCM_libsnd_gme_Base = . ;
		Blip_Buffer.o (.rodata .text)
		Classic_Emu.o (.rodata .text)
		Data_Reader.o (.rodata .text)
		Effects_Buffer.o (.rodata .text)
		Gme_File.o (.rodata .text)
		Multi_Buffer.o (.rodata .text)
		Music_Emu.o (.rodata .text)
		* (.ITCM_libsnd_gme)
	}
	.EOVR_ITCM_libsnd_midi	:
	{
		Image_EOVR_ITCM_libsnd_midi_Base = . ;
		* (.ITCM_libsnd_midi)
	}
	.EOVR_ITCM_libsnd_wave	:
	{
		Image_EOVR_ITCM_libsnd_wave_Base = . ;
		* (.ITCM_libsnd_wave)
	}
	.EOVR_MTCM	:
	{
		Image_EOVR_MTCM_Base = . ;
		header.o (.rodata)
		slice.o (.rodata)
		clibmpg.o (.rodata)
		* (.mtcmset)
		* (.mtcmvar)
	}
	.EOVR_libglobal_RebootLoader	:
	{
		Image_EOVR_libglobal_RebootLoader_Base = . ;
		* (.rebootloader)
	}

	.init	:
	{
		__text_start = . ;
		KEEP (*(.init))
		. = ALIGN(4);  /* REQUIRED. LD is flaky without it. */
	} >ewram = 0xff

	.plt : { *(.plt) } >ewram = 0xff

	.text :   /* ALIGN (4): */
	{
		*(EXCLUDE_FILE (*.itcm*) .text)

		*(.text.*)
		*(.stub)
		/* .gnu.warning sections are handled specially by elf32.em.  */
		*(.gnu.warning)
		*(.gnu.linkonce.t*)
		*(.glue_7)
		*(.glue_7t)
		. = ALIGN(4);  /* REQUIRED. LD is flaky without it. */
	} >ewram = 0xff

	.fini           :
	{
		KEEP (*(.fini))
	} >ewram =0xff

	__text_end = . ;

	.rodata :
	{
		*(.rodata)
		*all.rodata*(*)
		*(.roda)
		*(.rodata.*)
		*(.gnu.linkonce.r*)
		SORT(CONSTRUCTORS)
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >ewram = 0xff

	.ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >ewram
 	__exidx_start = .;
	ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } >ewram
 	__exidx_end = .;

	/*	Ensure the __preinit_array_start label is properly aligned.  We
		could instead move the label definition inside the section, but
		the linker would then create the section even if it turns out to
		be empty, which isn't pretty.  */

	. = ALIGN(32 / 8);

	PROVIDE (__preinit_array_start = .);
	.preinit_array     : { KEEP (*(.preinit_array)) } >ewram = 0xff
	PROVIDE (__preinit_array_end = .);
	PROVIDE (__init_array_start = .);
	.init_array     :
	{
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
	} >ewram = 0xff
	PROVIDE (__init_array_end = .);
	PROVIDE (__fini_array_start = .);
	.fini_array     :
	{
		KEEP (*(.fini_array))
		KEEP (*(SORT(.fini_array.*)))
	} >ewram = 0xff

	PROVIDE (__fini_array_end = .);

	.ctors :
	{
	/*	gcc uses crtbegin.o to find the start of the constructors, so
		we make sure it is first.  Because this is a wildcard, it
		doesn't matter if the user does not actually link against
		crtbegin.o; the linker won't look for a file to match a
		wildcard.  The wildcard also means that it doesn't matter which
		directory crtbegin.o is in.  */
		KEEP (*crtbegin.o(.ctors))
		KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
		KEEP (*(SORT(.ctors.*)))
		KEEP (*(.ctors))
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >ewram = 0xff

	.dtors :
	{
		KEEP (*crtbegin.o(.dtors))
		KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
		KEEP (*(SORT(.dtors.*)))
		KEEP (*(.dtors))
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >ewram = 0xff

	.eh_frame :
	{
		KEEP (*(.eh_frame))
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >ewram = 0xff

	.gcc_except_table :
	{
		*(.gcc_except_table)
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >ewram = 0xff
	.jcr            : { KEEP (*(.jcr)) } >ewram = 0
	.got            : { *(.got.plt) *(.got) *(.rel.got) } >ewram = 0

	.ewram ALIGN(4) : 
	{
		__ewram_start = ABSOLUTE(.) ;
		*(.ewram)
		*ewram.*(.text)
		. = ALIGN(4);   /* REQUIRED. LD is flaky without it. */
	} >ewram = 0xff


	.data ALIGN(4) :
	{
		__data_start = ABSOLUTE(.);
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
		*(.ITCM_libsnd_gme_DATA .IWRAM_proc_AudioCustom)
		CONSTRUCTORS
		. = ALIGN(4);
		__data_end = ABSOLUTE(.) ;
	} >ewram = 0xff


	__dtcm_lma = . ;
	__bss_vma = . ;

	.dtcm __dtcm_start : AT (__dtcm_lma)
	{
		Image_ER_DTCM_Base = . ;
		Load_ER_DTCM_Base = __dtcm_lma;
		*(.dtcm)
		*(.dtcm.*)
		. = ALIGN(4);
		__dtcm_end = ABSOLUTE(.);
	} >dtcm  = 0xff
	Image_ER_DTCM_Length = SIZEOF(.dtcm);


	__itcm_lma = __dtcm_lma + SIZEOF(.dtcm);

	.itcm __itcm_start : AT (__itcm_lma)
	{
		*(.ITCM)
		*(.ITCM_Global)
		*(.ITCM_MemTool)
		*(.ITCM_GLIB)
		
		*(*.itcm *.itcm.*)
		. = ALIGN(4);
		__itcm_end = ABSOLUTE(.);
	} >itcm = 0xff
	
	__vectors_lma = __itcm_lma + SIZEOF(.itcm);

	.vectors __vectors_start : AT (__vectors_lma)
	{
		*(.vectors)
		*vectors.*(.text)
		. = ALIGN(4);
		__vectors_end = ABSOLUTE(.);
	} >vectors = 0xff
	
	.sbss __dtcm_end (NOLOAD): 
	{
		__sbss_start = ABSOLUTE(.);
		__sbss_start__ = ABSOLUTE(.);
		*(.sbss)
		. = ALIGN(4);    /* REQUIRED. LD is flaky without it. */
		__sbss_end = ABSOLUTE(.);
	} >dtcm 

	.bss __bss_vma (NOLOAD): 
	{
		__bss_start = ABSOLUTE(.);
		__bss_start__ = ABSOLUTE(.);
		*(.dynbss)
		*(.gnu.linkonce.b*)
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);    /* REQUIRED. LD is flaky without it. */
		__bss_end__ = ABSOLUTE(.) ;
		__end__ = ABSOLUTE(.) ;
	} AT>ewram 

	Load_EOVR_Header_Base = __vectors_lma + SIZEOF(.vectors);
	.EOVR_Header 0x00000000 : AT (Load_EOVR_Header_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_Header_Length = SIZEOF(.EOVR_Header);	
	
	Load_EOVR_Romeo2NCD_Base = Load_EOVR_Header_Base + SIZEOF(.EOVR_Header);
	.EOVR_Romeo2NCD 0x00000000 : AT (Load_EOVR_Romeo2NCD_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_Romeo2NCD_Length = SIZEOF(.EOVR_Romeo2NCD);

	Load_EOVR_AfterSystem_Base = Load_EOVR_Romeo2NCD_Base + SIZEOF(.EOVR_Romeo2NCD);
	.EOVR_AfterSystem 0x023ff000-0x23e00 : AT (Load_EOVR_AfterSystem_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_AfterSystem_Length = SIZEOF(.EOVR_AfterSystem);

	Load_EOVR_proc_MainPass_Base = Load_EOVR_AfterSystem_Base + SIZEOF(.EOVR_AfterSystem);
	.EOVR_proc_MainPass 0x023ff000-0x23e00-0x13e00 : AT (Load_EOVR_proc_MainPass_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_proc_MainPass_Length = SIZEOF(.EOVR_proc_MainPass);
	
	Load_EOVR_proc_ChkDsk_Base = Load_EOVR_proc_MainPass_Base + SIZEOF(.EOVR_proc_MainPass);
	.EOVR_proc_ChkDsk 0x023ff000-0x23e00-0x13e00 : AT (Load_EOVR_proc_ChkDsk_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_proc_ChkDsk_Length = SIZEOF(.EOVR_proc_ChkDsk);
	
	Load_EOVR_proc_Setup_Base = Load_EOVR_proc_ChkDsk_Base + SIZEOF(.EOVR_proc_ChkDsk);
	.EOVR_proc_Setup 0x023ff000-0x23e00-0x13e00 : AT (Load_EOVR_proc_Setup_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_proc_Setup_Length = SIZEOF(.EOVR_proc_Setup);
	
	Load_EOVR_proc_FileList_Base = Load_EOVR_proc_Setup_Base + SIZEOF(.EOVR_proc_Setup);
	.EOVR_proc_FileList 0x023ff000-0x23e00-0x13e00 : AT (Load_EOVR_proc_FileList_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_proc_FileList_Length = SIZEOF(.EOVR_proc_FileList);
	
	Load_EOVR_proc_SysMenu_Base = Load_EOVR_proc_FileList_Base + SIZEOF(.EOVR_proc_FileList);
	.EOVR_proc_SysMenu 0x023ff000-0x23e00-0x13e00 : AT (Load_EOVR_proc_SysMenu_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_proc_SysMenu_Length = SIZEOF(.EOVR_proc_SysMenu);
	
	Load_EOVR_proc_DPGCustom_Base = Load_EOVR_proc_SysMenu_Base + SIZEOF(.EOVR_proc_SysMenu);
	.EOVR_proc_DPGCustom 0x023ff000-0x23e00-0x13e00 : AT (Load_EOVR_proc_DPGCustom_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_proc_DPGCustom_Length = SIZEOF(.EOVR_proc_DPGCustom);
	
	Load_EOVR_proc_DPGPlay_Base = Load_EOVR_proc_DPGCustom_Base + SIZEOF(.EOVR_proc_DPGCustom);
	.EOVR_proc_DPGPlay 0x023ff000-0x23e00-0x13e00 : AT (Load_EOVR_proc_DPGPlay_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_proc_DPGPlay_Length = SIZEOF(.EOVR_proc_DPGPlay);
	
	Load_EOVR_proc_ImageCustom_Base = Load_EOVR_proc_DPGPlay_Base + SIZEOF(.EOVR_proc_DPGPlay);
	.EOVR_proc_ImageCustom 0x023ff000-0x23e00-0x13e00 : AT (Load_EOVR_proc_ImageCustom_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_proc_ImageCustom_Length = SIZEOF(.EOVR_proc_ImageCustom);
	
	Load_EOVR_proc_ImageView_Base = Load_EOVR_proc_ImageCustom_Base + SIZEOF(.EOVR_proc_ImageCustom);
	.EOVR_proc_ImageView 0x023ff000-0x23e00-0x13e00 : AT (Load_EOVR_proc_ImageView_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_proc_ImageView_Length = SIZEOF(.EOVR_proc_ImageView);
	
	Load_EOVR_proc_TextCustom_Base = Load_EOVR_proc_ImageView_Base + SIZEOF(.EOVR_proc_ImageView);
	.EOVR_proc_TextCustom 0x023ff000-0x23e00-0x13e00 : AT (Load_EOVR_proc_TextCustom_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_proc_TextCustom_Length = SIZEOF(.EOVR_proc_TextCustom);
	
	Load_EOVR_proc_TextMenu_Base = Load_EOVR_proc_TextCustom_Base + SIZEOF(.EOVR_proc_TextCustom);
	.EOVR_proc_TextMenu 0x023ff000-0x23e00-0x13e00 : AT (Load_EOVR_proc_TextMenu_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_proc_TextMenu_Length = SIZEOF(.EOVR_proc_TextMenu);
	
	Load_EOVR_proc_TextView_Base = Load_EOVR_proc_TextMenu_Base + SIZEOF(.EOVR_proc_TextMenu);
	.EOVR_proc_TextView 0x023ff000-0x23e00-0x13e00 : AT (Load_EOVR_proc_TextView_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_proc_TextView_Length = SIZEOF(.EOVR_proc_TextView);
	
	Load_EOVR_proc_BinView_Base = Load_EOVR_proc_TextView_Base + SIZEOF(.EOVR_proc_TextView);
	.EOVR_proc_BinView 0x023ff000-0x23e00-0x13e00 : AT (Load_EOVR_proc_BinView_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_proc_BinView_Length = SIZEOF(.EOVR_proc_BinView);
	
	Load_EOVR_proc_Launch_Base = Load_EOVR_proc_BinView_Base + SIZEOF(.EOVR_proc_BinView);
	.EOVR_proc_Launch 0x023ff000-0x23e00-0x13e00 : AT (Load_EOVR_proc_Launch_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_proc_Launch_Length = SIZEOF(.EOVR_proc_Launch);
	
	Load_EOVR_proc_Custom_Base = Load_EOVR_proc_Launch_Base + SIZEOF(.EOVR_proc_Launch);
	.EOVR_proc_Custom 0x023ff000-0x23e00-0x13e00 : AT (Load_EOVR_proc_Custom_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_proc_Custom_Length = SIZEOF(.EOVR_proc_Custom);
	
	Load_EOVR_proc_BootROM_Base = Load_EOVR_proc_Custom_Base + SIZEOF(.EOVR_proc_Custom);
	.EOVR_proc_BootROM 0x023ff000-0x23e00-0x13e00 : AT (Load_EOVR_proc_BootROM_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_proc_BootROM_Length = SIZEOF(.EOVR_proc_BootROM);
	
	Load_EOVR_proc_MemoEdit_Base = Load_EOVR_proc_BootROM_Base + SIZEOF(.EOVR_proc_BootROM);
	.EOVR_proc_MemoEdit 0x023ff000-0x23e00-0x13e00 : AT (Load_EOVR_proc_MemoEdit_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_proc_MemoEdit_Length = SIZEOF(.EOVR_proc_MemoEdit);
	
	Load_EOVR_proc_MemoList_Base = Load_EOVR_proc_MemoEdit_Base + SIZEOF(.EOVR_proc_MemoEdit);
	.EOVR_proc_MemoList 0x023ff000-0x23e00-0x13e00 : AT (Load_EOVR_proc_MemoList_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_proc_MemoList_Length = SIZEOF(.EOVR_proc_MemoList);
	
	Load_EOVR_proc_AudioCustom_Base = Load_EOVR_proc_MemoList_Base + SIZEOF(.EOVR_proc_MemoList);
	.EOVR_proc_AudioCustom 0x023ff000-0x23e00-0x13e00  : AT (Load_EOVR_proc_AudioCustom_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_proc_AudioCustom_Length = SIZEOF(.EOVR_proc_AudioCustom);
	
	/*--------------------------------------------------------------------------------------*/
	Load_EOVR_libsnd_mp3_Base = Load_EOVR_proc_AudioCustom_Base + SIZEOF(.EOVR_proc_AudioCustom);
	.EOVR_libsnd_mp3 0x023ff000-0x23e00-0x13e00-0x21800  : AT (Load_EOVR_libsnd_mp3_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_libsnd_mp3_Length = SIZEOF(.EOVR_libsnd_mp3);
	
	Load_EOVR_libsnd_midi_Base = Load_EOVR_libsnd_mp3_Base + SIZEOF(.EOVR_libsnd_mp3);
	.EOVR_libsnd_midi 0x023ff000-0x23e00-0x13e00-0x21800  : AT (Load_EOVR_libsnd_midi_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_libsnd_midi_Length = SIZEOF(.EOVR_libsnd_midi);
	
	Load_EOVR_libsnd_gme_Base = Load_EOVR_libsnd_midi_Base + SIZEOF(.EOVR_libsnd_midi);
	.EOVR_libsnd_gme 0x023ff000-0x23e00-0x13e00-0x21800  : AT (Load_EOVR_libsnd_gme_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_libsnd_gme_Length = SIZEOF(.EOVR_libsnd_gme);
	
	Load_EOVR_libsnd_ogg_Base = Load_EOVR_libsnd_gme_Base + SIZEOF(.EOVR_libsnd_gme);
	.EOVR_libsnd_ogg 0x023ff000-0x23e00-0x13e00-0x21800  : AT (Load_EOVR_libsnd_ogg_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_libsnd_ogg_Length = SIZEOF(.EOVR_libsnd_ogg);
	
	Load_EOVR_libsnd_wave_Base = Load_EOVR_libsnd_ogg_Base + SIZEOF(.EOVR_libsnd_ogg);
	.EOVR_libsnd_wave 0x023ff000-0x23e00-0x13e00-0x21800  : AT (Load_EOVR_libsnd_wave_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_libsnd_wave_Length = SIZEOF(.EOVR_libsnd_wave);


	Load_EOVR_libimg_jpeg_Base = Load_EOVR_libsnd_wave_Base + SIZEOF(.EOVR_libsnd_wave);
	.EOVR_libimg_jpeg 0x023ff000-0x23e00-0x13e00-0x21800-0x10000  : AT (Load_EOVR_libimg_jpeg_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_libimg_jpeg_Length = SIZEOF(.EOVR_libimg_jpeg);
	
	Load_EOVR_libimg_png_Base = Load_EOVR_libimg_jpeg_Base + SIZEOF(.EOVR_libimg_jpeg);
	.EOVR_libimg_png 0x023ff000-0x23e00-0x13e00-0x21800-0x10000  : AT (Load_EOVR_libimg_png_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_libimg_png_Length = SIZEOF(.EOVR_libimg_png);
	
	Load_EOVR_libimg_bmp_Base = Load_EOVR_libimg_png_Base + SIZEOF(.EOVR_libimg_png);
	.EOVR_libimg_bmp 0x023ff000-0x23e00-0x13e00-0x21800-0x10000  : AT (Load_EOVR_libimg_bmp_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_libimg_bmp_Length = SIZEOF(.EOVR_libimg_bmp);
	
	Load_EOVR_libimg_gif_Base = Load_EOVR_libimg_bmp_Base + SIZEOF(.EOVR_libimg_bmp);
	.EOVR_libimg_gif 0x023ff000-0x23e00-0x13e00-0x21800-0x10000  : AT (Load_EOVR_libimg_gif_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_libimg_gif_Length = SIZEOF(.EOVR_libimg_gif);
	
	Load_EOVR_libimg_psd_Base = Load_EOVR_libimg_gif_Base + SIZEOF(.EOVR_libimg_gif);
	.EOVR_libimg_psd 0x023ff000-0x23e00-0x13e00-0x21800-0x10000  : AT (Load_EOVR_libimg_psd_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_libimg_psd_Length = SIZEOF(.EOVR_libimg_psd);
	
	/*; --------------------------------------------------------------------------------------*/
	
	Load_EOVR_libglobal_dpg_Base = Load_EOVR_libimg_psd_Base + SIZEOF(.EOVR_libimg_psd);
	.EOVR_libglobal_dpg 0x023ff000-0x23e00-0x13e00-0x21800-0x10000  : AT (Load_EOVR_libglobal_dpg_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_libglobal_dpg_Length = SIZEOF(.EOVR_libglobal_dpg);
	
	Load_EOVR_libglobal_ndsrom_Base = Load_EOVR_libglobal_dpg_Base + SIZEOF(.EOVR_libglobal_dpg);
	.EOVR_libglobal_ndsrom 0x023ff000-0x23e00-0x13e00-0x21800-0x10000  : AT (Load_EOVR_libglobal_ndsrom_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_libglobal_ndsrom_Length = SIZEOF(.EOVR_libglobal_ndsrom);
	
	/*; ----------------------------------------------------------------------------*/
	
	Load_EOVR_AfterSystem2_Base = Load_EOVR_libglobal_ndsrom_Base + SIZEOF(.EOVR_libglobal_ndsrom);
	.EOVR_AfterSystem2 0x023ff000-0x23e00-0x13e00-0x21800-0x10000-0xf00  : AT (Load_EOVR_AfterSystem2_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_AfterSystem2_Length = SIZEOF(.EOVR_AfterSystem2);
	
	/* old values from rvds */
	/* ----------------------------------------------------------------------------*/
	/* 0x01ff8a00		, 0x7600 , EOVR_ITCM_libglobal_**/
	/* 0x01ff8a00		, 0xd00  , EOVR_ITCM_libimg*/
	/* 0x01ff8a00+0xd00	, 0x6900 , EOVR_ITCM_libsnd*/
	/* ----------------------------------------------------------------------------*/
	
	Load_EOVR_ITCM_libglobal_dpg_Base = Load_EOVR_AfterSystem2_Base + SIZEOF(.EOVR_AfterSystem2);
	.EOVR_ITCM_libglobal_dpg 0x01ff8a00  : AT (Load_EOVR_ITCM_libglobal_dpg_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_ITCM_libglobal_dpg_Length = SIZEOF(.EOVR_ITCM_libglobal_dpg);
	
	Load_EOVR_ITCM_libglobal_ndsrom_Base = Load_EOVR_ITCM_libglobal_dpg_Base + SIZEOF(.EOVR_ITCM_libglobal_dpg);
	.EOVR_ITCM_libglobal_ndsrom 0x01ff8a00  : AT (Load_EOVR_ITCM_libglobal_ndsrom_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_ITCM_libglobal_ndsrom_Length = SIZEOF(.EOVR_ITCM_libglobal_ndsrom);
	
	/*; --------------------------------------------------------------------------------------*/
	
	Load_EOVR_ITCM_libimg_jpeg_Base = Load_EOVR_ITCM_libglobal_ndsrom_Base + SIZEOF(.EOVR_ITCM_libglobal_ndsrom);
	.EOVR_ITCM_libimg_jpeg 0x01ff8a00  : AT (Load_EOVR_ITCM_libimg_jpeg_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_ITCM_libimg_jpeg_Length = SIZEOF(.EOVR_ITCM_libimg_jpeg);
	
	Load_EOVR_ITCM_libimg_png_Base = Load_EOVR_ITCM_libimg_jpeg_Base + SIZEOF(.EOVR_ITCM_libimg_jpeg);
	.EOVR_ITCM_libimg_png 0x01ff8a00  : AT (Load_EOVR_ITCM_libimg_png_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_ITCM_libimg_png_Length = SIZEOF(.EOVR_ITCM_libimg_png);
	
	Load_EOVR_ITCM_libimg_bmp_Base = Load_EOVR_ITCM_libimg_png_Base + SIZEOF(.EOVR_ITCM_libimg_png);
	.EOVR_ITCM_libimg_bmp 0x01ff8a00  : AT (Load_EOVR_ITCM_libimg_bmp_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_ITCM_libimg_bmp_Length = SIZEOF(.EOVR_ITCM_libimg_bmp);
	
	Load_EOVR_ITCM_libimg_gif_Base = Load_EOVR_ITCM_libimg_bmp_Base + SIZEOF(.EOVR_ITCM_libimg_bmp);
	.EOVR_ITCM_libimg_gif 0x01ff8a00  : AT (Load_EOVR_ITCM_libimg_gif_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_ITCM_libimg_gif_Length = SIZEOF(.EOVR_ITCM_libimg_gif);
	
	Load_EOVR_ITCM_libimg_psd_Base = Load_EOVR_ITCM_libimg_gif_Base + SIZEOF(.EOVR_ITCM_libimg_gif);
	.EOVR_ITCM_libimg_psd 0x01ff8a00  : AT (Load_EOVR_ITCM_libimg_psd_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_ITCM_libimg_psd_Length = SIZEOF(.EOVR_ITCM_libimg_psd);
	
	/*; --------------------------------------------------------------------------------------*/
	
	Load_EOVR_ITCM_libsnd_mp3_Base = Load_EOVR_ITCM_libimg_psd_Base + SIZEOF(.EOVR_ITCM_libimg_psd);
	.EOVR_ITCM_libsnd_mp3 0x01ff8a00+0xd00  : AT (Load_EOVR_ITCM_libsnd_mp3_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_ITCM_libsnd_mp3_Length = SIZEOF(.EOVR_ITCM_libsnd_mp3);
	
	Load_EOVR_ITCM_libsnd_ogg_Base = Load_EOVR_ITCM_libsnd_mp3_Base + SIZEOF(.EOVR_ITCM_libsnd_mp3);
	.EOVR_ITCM_libsnd_ogg 0x01ff8a00+0xd00 : AT (Load_EOVR_ITCM_libsnd_ogg_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_ITCM_libsnd_ogg_Length = SIZEOF(.EOVR_ITCM_libsnd_ogg);
	
	Load_EOVR_ITCM_libsnd_gme_Base = Load_EOVR_ITCM_libsnd_ogg_Base + SIZEOF(.EOVR_ITCM_libsnd_ogg);
	.EOVR_ITCM_libsnd_gme 0x01ff8a00+0xd00 : AT (Load_EOVR_ITCM_libsnd_gme_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_ITCM_libsnd_gme_Length = SIZEOF(.EOVR_ITCM_libsnd_gme);
	
	Load_EOVR_ITCM_libsnd_midi_Base = Load_EOVR_ITCM_libsnd_gme_Base + SIZEOF(.EOVR_ITCM_libsnd_gme);
	.EOVR_ITCM_libsnd_midi 0x01ff8a00+0xd00 : AT (Load_EOVR_ITCM_libsnd_midi_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_ITCM_libsnd_midi_Length = SIZEOF(.EOVR_ITCM_libsnd_midi);
	
	Load_EOVR_ITCM_libsnd_wave_Base = Load_EOVR_ITCM_libsnd_midi_Base + SIZEOF(.EOVR_ITCM_libsnd_midi);
	.EOVR_ITCM_libsnd_wave 0x01ff8a00+0xd00 : AT (Load_EOVR_ITCM_libsnd_wave_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_ITCM_libsnd_wave_Length = SIZEOF(.EOVR_ITCM_libsnd_wave);
	
	/*; ----------------------------------------------------------------------------*/
	/*; MTCM in DTCM for DPG*/
	
	Load_EOVR_MTCM_Base = Load_EOVR_ITCM_libsnd_wave_Base + SIZEOF(.EOVR_ITCM_libsnd_wave);
	.EOVR_MTCM 0x0b000000+0x500: AT (Load_EOVR_MTCM_Base)
	{
		. = ALIGN(4);
	}
	Image_EOVR_MTCM_Length = SIZEOF(.EOVR_MTCM);
	    
	Load_EOVR_libglobal_RebootLoader_Base = Load_EOVR_MTCM_Base + SIZEOF(.EOVR_MTCM);
	.EOVR_libglobal_RebootLoader 0x023ff200 : AT (Load_EOVR_libglobal_RebootLoader_Base) /* IPC */
	{
		. = ALIGN(4);
	}
	Image_EOVR_libglobal_RebootLoader_Length = SIZEOF(.EOVR_libglobal_RebootLoader);

	/* Stabs debugging sections.  */
	.stab 0 : { *(.stab) }
	.stabstr 0 : { *(.stabstr) }
	.stab.excl 0 : { *(.stab.excl) }
	.stab.exclstr 0 : { *(.stab.exclstr) }
	.stab.index 0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment 0 : { *(.comment) }
	/*	DWARF debug sections.
		Symbols in the DWARF debugging sections are relative to the beginning
		of the section so we begin them at 0.  */
	/* DWARF 1 */
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }
	/* GNU DWARF 1 extensions */
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }
	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	/* DWARF 2 */
	.debug_info     0 : { *(.debug_info) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
	.stack 0x80000 : { _stack = .; *(.stack) }
	/* These must appear regardless of  .  */
}
