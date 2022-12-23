# Includes
include version.mk
ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

CXX=g++
TARGET=librfidengine
TARGET_DYN=$(TARGET).so.${MAJOR}.${MINOR}
TARGET_STA=$(TARGET).${MAJOR}.${MINOR}.a
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

SRCS= cmd_handler.cpp  common.cpp  c_if.cpp  rfid_if.cpp  TStringTokenizer.cpp parse_ds.cpp parser.cpp handle_manager.cpp rfid_config.cpp curl_stub.cpp ulog_type.cpp

CXXFLAGS = -std=c++17 -Wall -Wno-unused-function -fPIC ${DEBUG_EN} -DMAJOR=${MAJOR} -DMINOR=${MINOR} -DSUB_MINOR=${SUB_MINOR}


OBJS=$(patsubst %.cpp, $(ODIR)/%.o, $(SRCS))
DEPS=$(patsubst %.cpp, $(ODIR)/%.d, $(SRCS))

LIBS    = -lstdc++ -pthread -lcurl
ENGINE_LIB = -lrfidengine

INCFLAGS= -I./inc -I/usr/local/include -I/usr/include

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
	./sync_version.py
ifeq ($(detected_OS),Darwin)
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -shared -Wl,-install_name,$(TARGET_DYN) -o $(TARGET_DYN) $(OBJS) $(LIBS)
else ifeq ($(detected_OS),Linux)
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -shared -Wl,-soname,$(TARGET_DYN) -o $(TARGET_DYN) $(OBJS) $(LIBS)
endif

$(TARGET_STA): makedirs $(OBJS) $(DEPS)
	$(AR) -crs $(TARGET_STA) $(OBJS)


test: $(ODIR)/test.o install
	$(CXX) -Wl,-rpath,$(PREFIX)/lib/ -o  $@ $(ODIR)/test.o $(LIBS) $(ENGINE_LIB)

reset_reader: $(TARGET_STA) $(ODIR)/reset_reader.o
	$(CXX) -o  $@ $(ODIR)/reset_reader.o $(TARGET_STA) -lstdc++ -lcurl -pthread
	sudo mkdir -p /etc/edger/libs
	sudo cp ./rfid_engine.json /etc/edger/libs/rfid_engine

unit_test: $(ODIR)/unit_test.o $(ODIR)/catch_amalgamated.o install
	$(CXX) $(INCFLAGS) -Wl,-rpath,$(PREFIX)/lib/ -o  $@ $(ODIR)/unit_test.o $(ODIR)/catch_amalgamated.o $(LIBS) $(ENGINE_LIB)

cruise_test: $(ODIR)/cruise_test.o $(ODIR)/catch_amalgamated.o install

	$(CXX) $(INCFLAGS) -Wl,-rpath,$(PREFIX)/lib/ -o  $@ $(ODIR)/cruise_test.o $(ODIR)/catch_amalgamated.o $(LIBS) $(ENGINE_LIB)


install: $(TARGET_DYN)
	sudo install  $(TARGET_DYN) $(PREFIX)/lib/
	sudo ln -s -f $(PREFIX)/lib/$(TARGET_DYN) $(PREFIX)/lib/$(TARGET).so
	sudo mkdir -p $(PREFIX)/include/rfidengine
	sudo cp -r inc/* $(PREFIX)/include/rfidengine
	sudo mkdir -p /etc/edger/libs/rfid_engine
	sudo cp ./rfid_engine.json /etc/edger/libs/rfid_engine/rfid_engine.json

clean:
	find ./ -name "*~" -exec rm -rf {} \;
	find ./ -iname "*.[o|d]" -exec rm -rf {} \;
	rm -f $(TARGET_DYN) $(TARGET_STA) test unit_test
	sudo rm -rf $(PREFIX)/lib/$(TARGET).so $(PREFIX)/lib/$(TARGET_DYN) \
		$(PREFIX)/include/rfidengine

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
