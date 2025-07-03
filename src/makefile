include prorab.mk
include prorab-license.mk
include prorab-clang-format.mk
include prorab-install-dbgsrc.mk

$(eval $(call prorab-config, ../config))

this_src_dir := tcpserver

this_name := $(this_src_dir)$(this_dbg)

this_soname := $(shell cat $(d)soname.txt)

this_srcs := $(call prorab-src-dir, $(this_src_dir))

this_ldlibs += -l nitki$(this_dbg)
this_ldlibs += -l opros$(this_dbg)
this_ldlibs += -l setka$(this_dbg)
this_ldlibs += -l utki$(this_dbg)

$(eval $(prorab-build-lib))

$(eval $(prorab-clang-format))

$(eval $(prorab-install-dbgsrc))

this_license_file := ../LICENSE
$(eval $(prorab-license))
