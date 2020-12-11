CXX=clang++
TARGET=test
SDIR=src
ODIR=obj

SRCS=test.cpp async_socket.cpp cmd_handler.cpp common.cpp

CXXFLAGS = -std=c++17 -Wall -fPIC -DBOOST_LOG_DYN_LINK

OBJS=$(patsubst %.cpp, $(ODIR)/%.o, $(SRCS))
DEPS=$(patsubst %.cpp, $(ODIR)/%.d, $(SRCS))

LIBS    = -lboost_system -lstdc++ -lboost_thread -pthread -lboost_filesystem

INCFLAGS= -I./inc -I/usr/include

.PHONY:	clean $(TARGET)
# Objects generation
$(ODIR)/%.o:    $(SDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -c -MMD $< -o $@

# Generating dependencies
$(ODIR)/%.d:    $(SDIR)/%.cpp
	$(CXX) -M $(CXXFLAGS) $(INCFLAGS) $< > $@


$(TARGET): $(ODIR)/test.o $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LIBS)

clean:
	find ./ -name "*~" -exec rm -rf {} \;
	find ./ -iname "*.[o|d]" -exec rm -rf {} \;
	rm $(TARGET)

-include $(DEPS)
