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
#DEBUG_EN = -DDEBUG_ENABLE
DEBUG_EN =

# Detect system OS.
ifeq ($(OS),Windows_NT)
    detected_OS := Windows
else
    detected_OS := $(shell sh -c 'uname -s 2>/dev/null || echo not')
endif

SRCS= cmd_handler.cpp  common.cpp  cpp_if.cpp  rfid_if.cpp  TStringTokenizer.cpp parse_ds.cpp parser.cpp handle_manager.cpp rfid_config.cpp

CXXFLAGS = -std=c++17 -Wall -Wno-unused-function -fPIC ${DEBUG_EN} -DMAJOR=${MAJOR} -DMINOR=${MINOR} -DSUB_MINOR=${SUB_MINOR}


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

server: src/async_tcp_echo_server.cpp
	$(CXX) $(INCFLAGS) -o $@ $< $(LIBS)

client: src/blocking_tcp_echo_client.cpp
	$(CXX) $(INCFLAGS) -o $@ $< $(LIBS)

install: $(TARGET_DYN)
	sudo install  $(TARGET_DYN) $(PREFIX)/lib/
	sudo ln -s -f $(PREFIX)/lib/$(TARGET_DYN) $(PREFIX)/lib/$(TARGET).so
	sudo mkdir -p $(PREFIX)/include/rfidmgr
	sudo cp -r inc/* $(PREFIX)/include/rfidmgr
	sudo mkdir -p /etc/rfid_manager
	sudo cp ./rfid_config.json /etc/rfid_manager

clean:
	find ./ -name "*~" -exec rm -rf {} \;
	find ./ -iname "*.[o|d]" -exec rm -rf {} \;
	rm -f $(TARGET_DYN) test unit_test
	sudo rm -rf $(PREFIX)/lib/$(TARGET).so $(PREFIX)/lib/$(TARGET_DYN) \
		$(PREFIX)/include/rfidmgr

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
