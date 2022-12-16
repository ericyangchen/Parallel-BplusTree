btree.exe: main.cpp utils.cpp btree.cpp
	g++ -std=c++17 -O3 -o btree.exe main.cpp utils.cpp btree.cpp -fpermissive -pthread
clean:
	powershell.exe rm btree.exe