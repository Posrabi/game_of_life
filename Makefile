all:
	$(CXX) -Wall -Wextra -fopenmp -march=native -Ofast main.cc -std=c++17 -I /usr/local/include/opencv4/ -lopencv_core -lopencv_videoio -lopencv_imgproc -lopencv_highgui -o main
profile:
	$(CXX) -Wall -Wextra main.cc -pg -g -gdwarf-4 -std=c++17 -I /usr/local/include/opencv4/ -lopencv_core -lopencv_videoio -lopencv_imgproc -lopencv_highgui -o main
debug:
	$(CXX) -Wall -Wextra -Og -g -march=native main.cc -std=c++17 -I /usr/local/include/opencv4/ -lopencv_core -lopencv_videoio -lopencv_imgproc -lopencv_highgui -o main
	
