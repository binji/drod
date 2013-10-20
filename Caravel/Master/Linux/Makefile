# DROD Makefile, by Gerry Jo Jellestad

MAKEFLAGS += --no-print-directory

default: help
all: all-cus
all-cus: drod-custom drodutil-custom
all-rel all-release: drod-release drodutil-release
all-dbg all-debug: drod-debug drodutil-debug
all-prof all-profile: drod-profile drodutil-profile
cus custom drod drod-cus: drod-custom
drodutil drodutil-cus: drodutil-custom
rel release drod-rel: drod-release
drodutil-rel: drodutil-release
dbg debug drod-dbg: drod-debug
prof profile drod-prof: drod-profile
drodutil-dbg: drodutil-debug
drodutil-prof: drodutil-profile
install install-cus: install-custom
install-rel: install-release
install-dbg: install-debug
install-prof: install-profile

include Config

INST_HELP_LANG = 1

ALLBTYPES = custom release debug profile

ALLMODS = BackEndLib DRODLib FrontEndLib DROD DRODUtil
DRODMOD = BackEndLib DRODLib FrontEndLib DROD
UTILMOD = BackEndLib DRODLib DRODUtil

# Generate all possible ALLMODS-ALLBTYPES combinations
ALLMODS_ALL = $(shell for btype in $(ALLBTYPES); do \
	for mod in $(ALLMODS); do $(ECHO) $$mod-$$btype; done; done)

ALLMODS_CLEAN     = $(ALLMODS:%=clean-%)
ALLMODS_CLEAN_ALL = $(ALLMODS_ALL:%=clean-%)

.PHONY: all all-rel all-release all-debug all-prof all-profile \
	clean custom rel release debug prof profile \
	drod drod-custom drod-rel drod-release drod-debug \
	drod-prof drod-profile \
	drodutil drodutil-custom drodutil-rel drodutil-release \
	drodutil-dbg drodutil-debug \
	drodutil-prof drodutil-profile install $(ALLBTYPES:%=install-%) \
	$(ALLMODS) $(ALLMODS_ALL) $(ALLMODS_CLEAN) $(ALLMODS_CLEAN_ALL) \
	$(ALLBTYPES:%=clean-%) uninstall link

# These must be absolute
OBJDIR = obj
LIBDIR = lib
BINDIR = bin

#EXCLUDE_BackEndLib  =
#EXCLUDE_FrontEndLib =

INCLUDE_BackEndLib  = $(SDL_INCLUDE)
INCLUDE_DRODLib     = $(SDL_INCLUDE)
INCLUDE_FrontEndLib = $(SDL_INCLUDE)
INCLUDE_DROD        = $(SDL_INCLUDE)

TOBJDIR = $(BTYPE)/$(OBJDIR)
TLIBDIR = $(BTYPE)/$(LIBDIR)
TBINDIR = $(BTYPE)/$(BINDIR)
ifneq ($(MODULE),)
MODSDIR = $(SRCDIR)/$(MODULE)
MODODIR = $(TOBJDIR)/$(MODULE)
SOURCES = $(shell $(GREP) "SOURCE=.*\.cpp" $(MODSDIR)/$(MODULE).dsp |\
		$(SED) "s,SOURCE=.\\\\,$(MODSDIR)/,")
#ifneq ($(EXCLUDE_$(MODULE)),)
#SOURCES = $(filter-out $(EXCLUDE_$(MODULE):%=$(MODSDIR)/%), \
#	$(wildcard $(MODSDIR)/*.cpp))
#else
#SOURCES = $(wildcard $(MODSDIR)/*.cpp)
#endif
OBJECTS = $(SOURCES:$(MODSDIR)/%.cpp=$(MODODIR)/%.o)
DEPFILES = $(OBJECTS:%.o=%.dep)
INCLUDE = -I$(MODSDIR) -I$(SRCDIR) $(INCLUDE_$(MODULE))
endif

DRODLIBS = $(TOBJDIR)/DROD/NaClMain.o $(TLIBDIR)/DROD.a \
	   $(TLIBDIR)/FrontEndLib.a $(TLIBDIR)/DRODLib.a \
	   $(TLIBDIR)/BackEndLib.a
UTILLIBS = $(TLIBDIR)/DRODUtil.a $(TLIBDIR)/DRODLib.a $(TLIBDIR)/BackEndLib.a 

help:
	@echo "Help for the DROD build system: First edit the Config file, and follow its"
	@echo "instructions. Then run 'make <TARGET>[-<TYPE>]', where <TARGET> is:"
	@echo
	@echo "  drod         Build and link the drod executable."
	@echo "  drodutil     Build and link the drodutil executable."
	@echo "  all          Build and link all of the above."
	@echo "  BackEndLib   Only build the BackEndLib module."
	@echo "  FrontEndLib  Only build the FrontEndLib module."
	@echo "  DRODLib      Only build the DRODLib module."
	@echo "  DROD         Only build the DROD module (note the big case)."
	@echo "  install      Install the specified build type."
	@echo
	@echo "<TYPE> can be one of (aliases are only available for some targets):"
	@echo
	@echo "  custom (cus)    Custom build: If in doubt, use this one."
	@echo "  release (rel)   Release build: The binary distro was built with this."
	@echo "  debug (dbg)     Debug build: Enables debug information."
	@echo "  profile (prof)  Profile build: Enables profiling."
	@echo
	@echo "If <TARGET> is 'drod', only <TYPE> has to be specified. Also, if <TYPE> is left"
	@echo "out, it defaults to 'custom'. At least one of the two needs to be present."
	@echo "There's also clean targets available; 'make clean[-<TARGET>][-<TYPE>]'."
	@echo
	@echo "Good luck building Deadly Rooms Of Death!"

$(ALLBTYPES:%=drod-%):
	@$(MAKE) link LINKTARGET="$(@:drod-%=%)/$(BINDIR)/$(DROD)" \
		MODS="$(DRODMOD:%=%$(@:drod%=%))" XTYPE="DROD" BTYPE=$(@:drod-%=%)
$(ALLBTYPES:%=drodutil-%):
	@$(MAKE) link LINKTARGET="$(@:drodutil-%=%)/$(BINDIR)/$(DRODUTIL)" \
		MODS="$(UTILMOD:%=%$(@:drodutil%=%))" XTYPE="UTIL" BTYPE=$(@:drodutil-%=%)
clean:
	$(if $(MODULE), \
		($(if $(BTYPE), \
			$(RM_RF) $(DEPFILE) $(MODODIR) $(TLIBDIR)/$(MODULE).a, \
			$(MAKE) clean-$(MODULE)) ), \
		$(if $(BTYPE), \
			$(RM_RF) $(BTYPE), \
			$(RM_RF) $(ALLBTYPES) ) )

$(ALLMODS_ALL):
	@$(if $(BTYPE), (\
	 $(ECHO) "    Preparing to build $(@:%-$(BTYPE)=%).a, $(BTYPE) build"; \
	 $(ECHO) "    CXXFLAGS=$(CXXFLAGS_common) $(CXXFLAGS_$(BTYPE))"; \
	 $(MKDIR) $(BTYPE) $(TOBJDIR) $(TOBJDIR)/$(@:%-$(BTYPE)=%) $(TLIBDIR) $(TBINDIR); \
	 $(MAKE) $(BTYPE)/$(LIBDIR)/$(@:%-$(BTYPE)=%).a \
		MODULE="$(@:%-$(BTYPE)=%)" CXXFLAGS="$(CXXFLAGS_$(BTYPE))" \
	), \
	 $(MAKE) $@ BTYPE=$(filter $(ALLBTYPES), $(subst -, ,$@)) \
	)

$(ALLMODS_CLEAN_ALL):
	@$(if $(BTYPE), (\
	 $(MAKE) clean MODULE=$(@:clean-%-$(BTYPE)=%) \
	), \
	 $(MAKE) $@ BTYPE=$(filter $(ALLBTYPES), $(subst -, ,$@)) \
	)

$(ALLMODS_CLEAN):
	@$(MAKE) MODULE="$(@:clean-%=%)" BTYPE="*" clean

$(ALLBTYPES:%=clean-%):
	@$(MAKE) clean BTYPE=$(@:clean-%=%)

$(ALLBTYPES:%=install-%):
	$(if $(wildcard $(@:install-%=%)/$(BINDIR)/*),, \
		$(error No built targets for "$(@:install-%=%)" build type))
	$(if $(INSTALL_TYPE),, $(error INSTALL_TYPE not set))
	$(if $(BIN_$(INSTALL_TYPE)),, $(error BIN path not set for "$(INSTALL_TYPE)"))
	$(if $(RES_$(INSTALL_TYPE)),, $(error RES path not set for "$(INSTALL_TYPE)"))
	$(if $(DAT_$(INSTALL_TYPE)),, $(error DAT path not set for "$(INSTALL_TYPE)"))
	@$(ECHO) "Installing from \"$(@:install-%=%)\""
	$(INSTALL) $(INST_DAT_DIRS) -d $(DESTDIR)/$(DAT_$(INSTALL_TYPE))
	$(INSTALL) $(INST_RES_DIRS) -d $(DESTDIR)/$(BIN_$(INSTALL_TYPE))
	$(INSTALL) $(INST_RES_DIRS) -d \
		$(DESTDIR)/$(RES_$(INSTALL_TYPE))/Bitmaps \
		$(DESTDIR)/$(RES_$(INSTALL_TYPE))/Fonts \
		$(DESTDIR)/$(RES_$(INSTALL_TYPE))/Music \
		$(DESTDIR)/$(RES_$(INSTALL_TYPE))/Sounds \
		$(INST_HELP_LANG:%=$(DESTDIR)/$(RES_$(INSTALL_TYPE))/Help/%/images)
	$(INSTALL) $(INST_BIN_DIRS) $(@:install-%=%)/$(BINDIR)/* \
		$(DESTDIR)/$(BIN_$(INSTALL_TYPE))
	$(INSTALL) $(INST_RES_FILES) $(SRC_RES_BMP:%=$(SRC_DATADIR)/Bitmaps/%) \
		$(DESTDIR)/$(RES_$(INSTALL_TYPE))/Bitmaps
	$(INSTALL) $(INST_RES_FILES) $(SRC_RES_FONT:%=$(SRC_DATADIR)/Fonts/%) \
		$(DESTDIR)/$(RES_$(INSTALL_TYPE))/Fonts
	$(INSTALL) $(INST_RES_FILES) $(SRC_RES_MSX:%=$(SRC_DATADIR)/Music/%) \
		$(DESTDIR)/$(RES_$(INSTALL_TYPE))/Music
	$(INSTALL) $(INST_RES_FILES) $(SRC_RES_SFX:%=$(SRC_DATADIR)/Sounds/%) \
		$(DESTDIR)/$(RES_$(INSTALL_TYPE))/Sounds
	for lang in $(INST_HELP_LANG); do \
		$(INSTALL) $(INST_RES_FILES) \
			$(SRC_RES_HELP:%=$(SRC_DATADIR)/Help/$$lang/%) \
			$(DESTDIR)/$(RES_$(INSTALL_TYPE))/Help/$$lang; \
		$(INSTALL) $(INST_RES_FILES) \
			$(SRC_RES_HIMG:%=$(SRC_DATADIR)/Help/$$lang/images/%) \
			$(DESTDIR)/$(RES_$(INSTALL_TYPE))/Help/$$lang/images; done
	$(INSTALL) $(INST_DAT_FILES) \
		$(SRC_DAT:%=$(SRC_DATADIR)/%) $(DESTDIR)/$(DAT_$(INSTALL_TYPE))
	@echo "Install done!"

uninstall:
	$(if $(INSTALL_TYPE),, $(error INSTALL_TYPE not set))
	$(if $(BIN_$(INSTALL_TYPE)),, $(error BIN path not set for "$(INSTALL_TYPE)"))
	$(if $(RES_$(INSTALL_TYPE)),, $(error RES path not set for "$(INSTALL_TYPE)"))
	$(if $(DAT_$(INSTALL_TYPE)),, $(error DAT path not set for "$(INSTALL_TYPE)"))
	$(RM_RF) $(DESTDIR)/$(BIN_$(INSTALL_TYPE))/$(DROD) \
		$(DESTDIR)/$(BIN_$(INSTALL_TYPE))/$(DRODUTIL) \
		$(DESTDIR)/$(RES_$(INSTALL_TYPE)) \
		$(DESTDIR)/$(DAT_$(INSTALL_TYPE))

$(MODODIR)/%.o: $(MODSDIR)/%.cpp
	@$(ECHO) "    DepComp $<"
	@$(CXXDEP) $(CXXFLAGS_common) $(CXXFLAGS) $(INCLUDE) -c $< $(DEPOUT) $(@:%.o=%.tmp) -o $@
	@$(SED) "s,\($(MODODIR)/\)*\(.*\).o:,$(MODODIR)/\2.o:," < $(@:%.o=%.tmp) > $(@:%.o=%.dep)
	@$(RM_F) $(@:%.o=%.tmp)

$(TLIBDIR)/$(MODULE).a: $(OBJECTS)
	$(if $(MODULE),, $(error Don't call this target directly))
	@$(ECHO) "    Archive $@"
	$(AR) $@ $(OBJECTS)
	@$(RANLIB) $@
	@$(ECHO) " "

link: $(MODS)
	$(if $(LINKTARGET),, $(error Don't call this target directly))
	@$(MAKE) $(LINKTARGET)
	@$(ECHO) "    Done!"
	@$(ECHO) " "

$(TBINDIR)/$(DROD) $(TBINDIR)/$(DRODUTIL): $($(XTYPE)LIBS)
	@$(ECHO) "    Linking $@:"
	$(LD) $($(XTYPE)LIBS) $(LDFLAGS_common) $(LDFLAGS_$(XTYPE)_$(BTYPE)) -o $@

ifneq ($(DEPFILES),)
ifneq ($(wildcard $(MODODIR)/*.dep),)
include $(wildcard $(MODODIR)/*.dep)
endif
endif

# $Log: Makefile,v $
# Revision 1.6  2005/04/13 16:06:46  gjj
# Some small fixes
#
# Revision 1.5  2004/07/18 13:31:31  gjj
# New Linux data searcher (+ copy data to home if read-only), made some vars
# static, moved stuff to .cpp-files to make the exe smaller, couple of
# bugfixes, etc.
#
# Revision 1.4  2004/01/13 21:46:47  gjj
# Sync with 1.7: improved dependency system
#
# Revision 1.3  2003/10/09 14:53:21  mrimer
# Makefile revisions (committed on behalf of Gerry JJ).
#
# Revision 1.2  2003/10/08 17:30:07  mrimer
# Linux build updates (committed on behalf of Gerry JJ).
#
# Revision 1.1  2003/10/01 15:28:23  mrimer
# Initial check-in (simpler unified makefile process).
#
