# Includes
include version.mk
ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

CXX=g++
TARGET=librfidmgr
TARGET_DYN=$(TARGET).so.${MAJOR}.${MINOR}
SDIR=src
ODIR=obj
DIRS=$(SDIR) $(ODIR)

# Detect system OS.
ifeq ($(OS),Windows_NT)
    detected_OS := Windows
else
    detected_OS := $(shell sh -c 'uname -s 2>/dev/null || echo not')
endif

SRCS= cmd_handler.cpp  common.cpp  cpp_if.cpp  rfid_if.cpp  TStringTokenizer.cpp parse_ds.cpp parser.cpp

CXXFLAGS = -std=c++14 -Wall -Wno-unused-function -fPIC

OBJS=$(patsubst %.cpp, $(ODIR)/%.o, $(SRCS))
DEPS=$(patsubst %.cpp, $(ODIR)/%.d, $(SRCS))

LIBS    = -lstdc++ -pthread -lrfidmgr

INCFLAGS= -I./inc -I./inc/asio -I/usr/include

.PHONY:	clean $(TARGET_DYN)
# Objects generation
$(ODIR)/%.o:    $(SDIR)/%.cpp
	@$(call make-dirs)
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -c -MMD $< -o $@

# Generating dependencies
$(ODIR)/%.d:    $(SDIR)/%.cpp
	@$(call make-dirs)
	$(CXX) -M $(CXXFLAGS) $(INCFLAGS) $< > $@


$(TARGET_DYN): makedirs $(OBJS) $(DEPS)
ifeq ($(detected_OS),Darwin)
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -shared -Wl,-install_name,$(TARGET_DYN) -o $(TARGET_DYN) $(OBJS)
else ifeq ($(detected_OS),Linux)
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -shared -Wl,-soname,$(TARGET_DYN) -o $(TARGET_DYN) $(OBJS)
endif

test: $(ODIR)/test.o install
	$(CXX) -Wl,-rpath,$(PREFIX)/lib/ -o  $@ $(ODIR)/test.o $(LIBS)

door: $(ODIR)/door.o install
	$(CXX) -Wl,-rpath,$(PREFIX)/lib/ -o  $@ $(ODIR)/door.o $(LIBS)

unit_test: $(ODIR)/unit_test.o install
	$(CXX) -Wl,-rpath,$(PREFIX)/lib/ -o  $@ $(ODIR)/unit_test.o $(LIBS)

install: $(TARGET_DYN)
	sudo install  $(TARGET_DYN) $(PREFIX)/lib/
	sudo ln -s -f $(PREFIX)/lib/$(TARGET_DYN) $(PREFIX)/lib/$(TARGET).so
	sudo cp inc/*.hpp $(PREFIX)/include
	sudo cp inc/TStringTokenizer.h $(PREFIX)/include
	sudo cp inc/TString.h $(PREFIX)/include
	sudo cp inc/rfid_err.h $(PREFIX)/include

clean:
	find ./ -name "*~" -exec rm -rf {} \;
	find ./ -iname "*.[o|d]" -exec rm -rf {} \;
	rm -f $(TARGET_DYN) test unit_test
	sudo rm -f $(PREFIX)/lib/$(TARGET).so $(PREFIX)/lib/$(TARGET_DYN) \
		$(PRFIX)/include/aixlog.hpp \
		$(PRFIX)/include/cmd_handler.hpp \
		$(PRFIX)/include/cpp_if.hpp \
		$(PRFIX)/include/packet_content.hpp \
		$(PRFIX)/include/parser.hpp \
		$(PRFIX)/include/rfid_if.hpp \
		$(PREFIX)/include/TStringTokenizer.h \
		$(PREFIX)/include/common.hpp \
		$(PREFIX)/include/observer.hpp \
		$(PREFIX)/include/packet_queue.hpp \
		$(PREFIX)/include/send_sync_observer.hpp \
		$(PREFIX)/include/conn_queue.hpp \
		$(PREFIX)/include/PacketCommunication.hpp \
		$(PREFIX)/include/parse_ds.hpp \
		$(PREFIX)/include/rfid_err.h \
		$(PREFIX)/include/TString.h

# Create directories
makedirs:
	@$(call make-dirs)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif

define make-dirs
	for dir in $(ODIR);\
	do\
		mkdir -p $$dir;\
	done
endef
