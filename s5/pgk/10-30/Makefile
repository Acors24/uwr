default: AGL3-example

%: %.cpp
	g++ -I. $< -o $@ AGL3Window.cpp  -lepoxy -lGL -lglfw 
clean:
	rm a.out *.o *~ AGL3-example
