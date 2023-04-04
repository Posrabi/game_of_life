all:
	$(CXX) -Wall -Wextra -O3 main.cc -std=c++17 -I /usr/local/include/opencv4/ -lopencv_core -lopencv_videoio -lopencv_imgproc -lopencv_highgui -o main
debug:
	$(CXX) -Wall -Wextra -Og -g main.cc -std=c++17 -I /usr/local/include/opencv4/ -lopencv_core -lopencv_videoio -lopencv_imgproc -lopencv_highgui -o main
	
