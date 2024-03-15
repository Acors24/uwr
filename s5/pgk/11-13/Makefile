default: main

%: %.cpp Obstacle.cpp
	g++ -I. $< AGL3Window.cpp Player.cpp Obstacle.cpp -o $@ -lepoxy -lGL -lglfw 
clean:
	rm -f main
