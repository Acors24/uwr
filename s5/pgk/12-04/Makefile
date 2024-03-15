CXX    := g++
SRCS   := main.cpp AGL3Window.cpp Camera.cpp Player.cpp Bubble.cpp Sun.cpp Volcano.cpp
HDRS   := AGL3Drawable.hpp AGL3Window.hpp Camera.hpp Player.hpp Bubble.hpp Sun.hpp Volcano.hpp
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