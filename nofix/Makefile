MOD ?= inte

ifeq ($(MOD),inte)
DEPS := KallRecon Type_info
else
DEPS := HooKern Type_info
endif

KDIR := $(KDIR)
MDIR := $(realpath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
ODIR := $(MDIR)/out/$(if $(VER),$(VER),unknown)/$(MOD)

include $(MDIR)/mk/deps.mk

ifeq ($(MOD),inte)
obj-m := inte.o
inte-y := src/inte/nofix/oplus_kernel_security_check.o \
	src/inte/nofix/inte_compat.o \
	$(DEPS_OBJS_ALL)
else
DEPS_OBJS_ALL := $(filter-out deps/HooKern/lib/hk_binder.o deps/HooKern/lib/hk_lsm.o,$(DEPS_OBJS_ALL))
obj-m := $(MOD).o
$(MOD)-y := src/poc/$(MOD)/$(MOD).o src/poc/poc_common.o $(DEPS_OBJS_ALL)
endif

ccflags-y += -std=gnu11
ccflags-y += -Wno-declaration-after-statement
ccflags-y += -Wno-unused-variable
ccflags-y += -Wno-unused-function
ccflags-y += -Wno-strict-prototypes
ccflags-y += -I$(src)/src/poc
ccflags-y += $(addprefix -I$(src)/,$(DEPS_INCS_ALL))

$(info -- KDIR: $(KDIR))
$(info -- MDIR: $(MDIR))
$(info -- ODIR: $(ODIR))

all:
	mkdir -p $(ODIR)
	make -C $(KDIR) M=$(ODIR) src=$(MDIR) srcroot=$(MDIR) modules
clean:
	make -C $(KDIR) M=$(ODIR) src=$(MDIR) srcroot=$(MDIR) clean

$(obj)/%.o: $(src)/%.c $(recordmcount_source) FORCE
	$(call if_changed_rule,cc_o_c)
	$(call cmd,force_checksrc)

$(obj)/%.o: $(src)/%.S FORCE
	$(call if_changed_rule,as_o_S)
