include core/arch/$(ARCH)/plat-$(PLATFORM)/$(PLATFORM_FLAVOR)/config.mk

CROSS_COMPILE ?= arm-none-linux-gnueabihf-
CROSS_COMPILE64 ?= aarch64-none-linux-gnu-

CFG_WERROR := y

CFG_MMAP_REGIONS ?= 13

$(call force,CFG_SECSTOR_TA,n)

ifeq ($(platform-flavor-armv8),1)
$(call force,CFG_WITH_ARM_TRUSTED_FW,y)
$(call force,CFG_CRYPTO_WITH_CE,y)
endif

$(call force,CFG_SECURE_TIME_SOURCE_CNTPCT,y)
$(call force,CFG_GIC,y)
$(call force,CFG_SCTLR_ALIGNMENT_CHECK,n)
$(call force,CFG_CACHE_API,y)

CFG_TEE_CORE_LOG_LEVEL ?= 3
$(call force,CFG_TEE_TA_LOG_LEVEL,3)
ifeq ($(CFG_TEE_CORE_LOG_LEVEL),0)
$(call force,CFG_UNWIND,n)
endif

CFG_WITH_STATS ?= y

ifeq ($(CFG_RPMB_FS),y)
# If REE FS is erased than the hash in RPMB will be mismatched.
# Enable this option to reset the hash in RPMB.
$(call force,CFG_REE_FS_ALLOW_RESET,y)
endif

# $(call force,CFG_FTRACE_SUPPORT,y)

$(call force,CFG_CORE_HEAP_SIZE,1048576) # 1MB

ifeq ($(CFG_ARM64_core),y)
supported-ta-targets = ta_arm64
else
supported-ta-targets = ta_arm32
endif

ifeq ($(CFG_RPMB_FS),y)
# Android verified boot Trusted Application
CFG_IN_TREE_EARLY_TAS += avb/023f8f1a-292a-432b-8fc4-de8471358067
endif

CFG_ENCRYPT_TA = y

