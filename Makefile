CXX = g++
CXXFLAGS = -O2 -std=c++17

.PHONY: demo clean

flight_planner: flight_planner.cpp
	$(CXX) $(CXXFLAGS) -o flight_planner flight_planner.cpp

demo: flight_planner
	./flight_planner --demo

clean:
	rm -f flight_planner
