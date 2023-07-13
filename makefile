WebServer:start-main.cc
	g++ -o $@ $^ -std=c++11
.PHONY:clean
clean:
	rm -f WebServer
	bash ./CleanLogs.sh