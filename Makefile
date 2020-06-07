#
# Date:      2020/01/31 10:33
# Author:    Petra Stefanikova, Petr Vana, Jan Faigl
#

CXX:=ccache $(CXX)

include Mk/libs.mk

CPPFLAGS+=$(LOCAL_CFLAGS)
LDFLAGS+=$(LOCAL_LDFLAGS)

CPPFLAGS+=$(CRL_CFLAGS) $(BOOST_CFLAGS) $(CAIRO_CFLAGS) $(LOG4CXX_CPPFLAGS)
LDFLAGS+=$(CRL-ALGORITHM) $(CRL-GUI_LDFLAGS) $(CRL_LDFLAGS) $(CAIRO_LDFLAGS) $(BOOST_LDFLAGS) $(LOG4CXX_LDFLAGS)

CXXFLAGS+=-std=c++11
CXXFLAGS+=-O2 -march=native
CXXFLAGS+=-g

OBJS=\
     src/grasp_ceop.o\
     src/grasp.o\
     src/target.o\
     src/coords.o\
     src/path.o\

TARGET=grasp_ceop

include Mk/comrob.mk

