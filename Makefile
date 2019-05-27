v1:
	g++ -pthread version1.cpp -o output && ./output

v2:
	g++ -pthread version2.cpp -o output && ./output

v3:
	g++ -pthread version3.cpp -o output && ./output < input
