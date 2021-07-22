CXX ?= g++
CXXFLAGS += -O3 -Wall -Wextra -Werror -std=c++11 -flto

BUILD_DIR ?= $(CURDIR)/build

SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)
DEPENDS = $(OBJECTS:%.o=%.d)

TARGET = nonogram

.PHONY: default
default: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -c -o $@ $<

-include $(DEPENDS)

.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR) $(TARGET)
