#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

#---------------------------------------------------------------------------------
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
# DATA is a list of directories containing binary files
# all directories are relative to this makefile
#---------------------------------------------------------------------------------
BUILD		:=	build
SOURCES		:=	source source/glib source/libs source/gba_nds_fat source/libelm/source source/libelm/source/unicode\
			source/resource source/plugin source/plugin/Image source/plugin/Sound source/zlib\
			source/plugin/Video source/plugin/Video/libmpeg2 \
			source/plugin/Image/libgif source/plugin/Image/libjpeg source/plugin/Image/libpng \
			source/plugin/Sound/libgme source/plugin/Sound/libmidi source/plugin/Sound/libmp3 source/plugin/Sound/libogg \
			source/procbody source/procbody/proc_AudioPlay source/procbody/Proc_BinView source/procbody/proc_DPGPlay \
			source/procbody/proc_BootROM source/procbody/proc_FileList source/procbody/proc_ImageView \
			source/procbody/proc_Memo source/procbody/proc_MemoList source/procbody/proc_TextView \
			source/plugin/Sound/libgme/disabled source/plugin/Sound/libgme/format_Ay source/plugin/Sound/libgme/format_Gb \
			source/plugin/Sound/libgme/format_Hes source/plugin/Sound/libgme/format_Kss \
			source/plugin/Sound/libgme/format_Nes source/plugin/Sound/libgme/format_Sap \
			source/plugin/Sound/libgme/format_Sms 

INCLUDES	:=	source source/glib source/libs source/gba_nds_fat source/libelm/source \
			source/resource source/plugin source/plugin/Image source/plugin/Sound \
			source/plugin/Video source/libelm/include source/zlib \
			source/plugin/Sound/libgme \
			source/plugin/Sound/libgme/disabled source/plugin/Sound/libgme/documents \
			source/plugin/Sound/libgme/format_Ay source/plugin/Sound/libgme/format_Gb \
			source/plugin/Sound/libgme/format_Hes source/plugin/Sound/libgme/format_Kss \
			source/plugin/Sound/libgme/format_Nes source/plugin/Sound/libgme/format_Sap \
			source/plugin/Sound/libgme/format_Sms \
			source/plugin/Sound/libmidi \
			source/procbody source/procbody/proc_AudioPlay source/procbody/Proc_BinView source/procbody/proc_DPGPlay \
			source/procbody/proc_BootROM source/procbody/proc_FileList source/procbody/proc_ImageView \
			source/procbody/proc_Memo source/procbody/proc_MemoList source/procbody/proc_TextView \
			source/plugin/Sound/libmp3
DATA		:=


#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	#-mthumb -mthumb-interwork	#This may cause some problem when there are some cross-calls between C&Assembly.

CFLAGS	:=	-Wall -O2 -DNO_VIZ\
 			-march=armv5te -mtune=arm946e-s -fomit-frame-pointer\
			-ffast-math \
			$(ARCH)

CFLAGS	+=	$(INCLUDE) -DARM9 -DPluginMode_Sound -DPluginMode_Image -DUSEDPG -DUSELIBMD5
CXXFLAGS	:=	$(CFLAGS) -fno-rtti -fno-exceptions

ASFLAGS	:=	$(ARCH) -march=armv5te -mtune=arm946e-s $(CFLAGS)

LDFLAGS	=	-specs=fish_arm9.specs $(ARCH) -Wl,-Map,$(notdir $*.map)

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:=	-lnds9
 
#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=	$(LIBNDS) $(CURDIR)
 
#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
 
export ARM9ELF	:=	$(CURDIR)/$(TARGET).elf
export DEPSDIR	:=	$(CURDIR)/$(BUILD)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))
 
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
 
#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
 
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)
 
export LIBPATHS	:=	-L$(TOPDIR)/nifi $(foreach dir,$(LIBDIRS),-L$(dir)/lib)
 
.PHONY: $(BUILD) clean
 
#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	cp $(CURDIR)/fish_arm9.ld $(DEVKITARM)/arm-eabi/lib/
	cp $(CURDIR)/fish_arm9.specs $(DEVKITARM)/arm-eabi/lib/
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
 
#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -rf $(DEVKITARM)/arm-eabi/lib/fish_arm9.specs
	@rm -rf $(DEVKITARM)/arm-eabi/lib/fish_arm9.ld
	@rm -fr $(BUILD) *.elf *.nds* *.bin 
 
 
#---------------------------------------------------------------------------------
else
 
#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(ARM9ELF)	:	$(OFILES)
	@echo linking $(notdir $@)
	@$(LD)  $(LDFLAGS) $(OFILES) $(LIBPATHS) $(LIBS) -o $@

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data 
#---------------------------------------------------------------------------------
%.bin.o	:	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPSDIR)/*.d
 
#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
