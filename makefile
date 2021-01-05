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

SRCS=async_socket.cpp cmd_handler.cpp common.cpp

CXXFLAGS = -std=c++14 -Wall -Wno-unused-function -fPIC

OBJS=$(patsubst %.cpp, $(ODIR)/%.o, $(SRCS))
DEPS=$(patsubst %.cpp, $(ODIR)/%.d, $(SRCS))

LIBS    = -lstdc++ -pthread -lrfidmgr

INCFLAGS= -I./inc -I./inc/asio -I/usr/include

.PHONY:	clean $(TARGET_DYN)
# Objects generation
$(ODIR)/%.o:    $(SDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -c -MMD $< -o $@

# Generating dependencies
$(ODIR)/%.d:    $(SDIR)/%.cpp
	$(CXX) -M $(CXXFLAGS) $(INCFLAGS) $< > $@

$(TARGET_DYN): makedirs $(OBJS) $(DEPS)
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -shared -Wl,-soname,$(TARGET_DYN) -o $(TARGET_DYN) $(OBJS)

test: $(ODIR)/test.o install
	$(CXX) -Wl,-rpath,$(PREFIX)/lib/ -o  $@ $(ODIR)/test.o $(LIBS)

install: $(TARGET_DYN)
	sudo install  $(TARGET_DYN) $(PREFIX)/lib/
	sudo ln -s -f $(PREFIX)/lib/$(TARGET_DYN) $(PREFIX)/lib/$(TARGET).so

clean:
	find ./ -name "*~" -exec rm -rf {} \;
	find ./ -iname "*.[o|d]" -exec rm -rf {} \;
	rm -f $(TARGET_DYN) test
	sudo rm -f $(PREFIX)/lib/$(TARGET).so $(PREFIX)/lib/$(TARGET_DYN)

# Create directories
makedirs:
	@$(call make-dirs)

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif

define make-dirs
	for dir in $(DIRS);\
	do\
		mkdir -p $$dir;\
	done
endef
