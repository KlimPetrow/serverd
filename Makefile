serverd: serverd.cpp
	g++ serverd.cpp -o serverd -Wall -Wextra -std=c++11

clean:
	rm serverd

.PHONY: clean
