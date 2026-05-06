CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -I./src
LDFLAGS = -lws2_32

SOURCES = \
	src/main.cpp \
	src/cli/create_command.cpp \
	src/cli/init_command.cpp \
	src/cli/dev_command.cpp \
	src/cli/build_command.cpp \
	src/cli/deploy_command.cpp \
	src/cli/run_command.cpp \
	src/cli/update_command.cpp \
	src/server/http_server.cpp \
	src/server/live_reload.cpp \
	src/transpiler/ast_to_html.cpp \
	src/transpiler/ast_to_js.cpp \
	src/transpiler/ast_to_css.cpp

TARGET = astra

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
