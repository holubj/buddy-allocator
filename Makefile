all:
	g++ -Wall -pedantic -std=c++11 -o buddy src/buddy.cpp

clean:
	rm buddy
