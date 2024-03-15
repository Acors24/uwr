CXX    := g++
SRCS   := main.cpp AGL3Window.cpp Camera.cpp Model.cpp Util.cpp
HDRS   := AGL3Drawable.hpp AGL3Window.hpp Camera.hpp Model.hpp Util.hpp Material.hpp
OBJS   := $(SRCS:.cpp=.o)
LIBS   := -lepoxy -lGL -lglfw
TARGET := main

default: $(TARGET)

$(TARGET): $(OBJS)
	@$(CXX) -o $(TARGET) $(OBJS) -I. $(LIBS)

%.o: %.cpp %.hpp
	@$(CXX) -c $< -o $@ -I. $(LIBS)

clean:
	@rm -f $(wildcard *.o) main