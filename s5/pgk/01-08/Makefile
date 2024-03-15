CXX    := g++
SRCS   := main.cpp AGL3Window.cpp Camera.cpp Map.cpp EarthGrid.cpp Crosshair.cpp
HDRS   := AGL3Drawable.hpp AGL3Window.hpp Camera.hpp Map.hpp EarthGrid.hpp Crosshair.hpp
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