btree: main.cpp utils.cpp btree.cpp
	g++ -std=c++17 -O3 -o btree main.cpp utils.cpp btree.cpp -fpermissive -pthread
clean:
	rm -f btree