#
# Date:      2011/07/11 17:55
# Author:    Jan Faigl
#

OPSYS=$(shell uname)

PLATFORM=$(shell uname -p)
ARCH=.$(PLATFORM)

ifeq ($(OPSYS),FreeBSD)
   BOOST_CFLAGS=-I/usr/local/include
   BOOST_LDFLAGS=-L/usr/local/lib

else
   LOG4CXX_CPPFLAGS=$(shell pkg-config --cflags liblog4cxx)
   LOG4CXX_LDFLAGS=$(shell pkg-config --libs liblog4cxx)

   CAIRO_LDFLAGS:=-L/usr/X11/lib
   CAIRO_CFLAGS:=-I/usr/X11/include
endif


BOOST_LDFLAGS+=-lboost_program_options -lboost_thread -lboost_filesystem -lboost_iostreams -lboost_system
LOG4CXX_LDFLAGS+=-llog4cxx
CAIRO_LDFLAGS+=-lcairo -pthread -lX11

LOCAL_CFLAGS=-I./include
LOCAL_LDFLAGS=-L./lib

CRL-GUI_LDFLAGS=-lcrl-gui
CRL_LDFLAGS=-lcrl
CRL-ALGORITHM=-lcrl-algorithm
