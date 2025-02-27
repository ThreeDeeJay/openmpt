
CXXFLAGS_WARNINGS += -Wcast-align -Wcast-qual -Wdouble-promotion -Wfloat-conversion -Wmissing-prototypes -Wshift-count-negative -Wshift-count-overflow -Wshift-op-parentheses -Wshift-overflow -Wshift-sign-overflow -Wundef
CFLAGS_WARNINGS   += -Wcast-align -Wcast-qual -Wdouble-promotion -Wfloat-conversion -Wmissing-prototypes -Wshift-count-negative -Wshift-count-overflow -Wshift-op-parentheses -Wshift-overflow -Wshift-sign-overflow -Wundef

CXXFLAGS_WARNINGS += -Wdeprecated -Wextra-semi -Wframe-larger-than=16000 -Wglobal-constructors -Wimplicit-fallthrough -Wmissing-declarations -Wnon-virtual-dtor -Wreserved-id-macro
CFLAGS_WARNINGS   +=                           -Wframe-larger-than=4000

#CXXFLAGS_WARNINGS += -Wfloat-equal
#CXXFLAGS_WARNINGS += -Wdocumentation
#CXXFLAGS_WARNINGS += -Wconversion
#CXXFLAGS_WARNINGS += -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-c++98-c++11-c++14-compat -Wno-padded -Wno-weak-vtables -Wno-sign-conversion -Wno-shadow-field-in-constructor -Wno-conversion -Wno-switch-enum -Wno-old-style-cast

ifeq ($(MODERN),1)
CXXFLAGS_WARNINGS += 
CFLAGS_WARNINGS   += 
LDFLAGS_WARNINGS  += -Wl,-no-undefined -Wl,--detect-odr-violations
endif

CFLAGS_SILENT += -Wno-\#warnings
CFLAGS_SILENT += -Wno-cast-align
CFLAGS_SILENT += -Wno-cast-qual
CFLAGS_SILENT += -Wno-double-promotion
CFLAGS_SILENT += -Wno-float-conversion
CFLAGS_SILENT += -Wno-frame-larger-than
CFLAGS_SILENT += -Wno-missing-prototypes
CFLAGS_SILENT += -Wno-sign-compare
CFLAGS_SILENT += -Wno-unused-function
CFLAGS_SILENT += -Wno-unused-parameter
CFLAGS_SILENT += -Wno-unused-variable

FASTMATH_STYLE=clang
