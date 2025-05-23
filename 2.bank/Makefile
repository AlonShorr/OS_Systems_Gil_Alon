CXX = g++
CXXFLAGS = -std=c++11 -g -Wall -Werror -pedantic-errors -DDEBUG -pthread
TARGET = bank

SRCS = account.cpp atm.cpp logger.cpp read_write_lock.cpp bank.cpp
OBJS = account.o atm.o logger.o read_write_lock.o bank.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

account.o: account.cpp account.hpp
	$(CXX) $(CXXFLAGS) -c account.cpp -o account.o

atm.o: atm.cpp atm.hpp
	$(CXX) $(CXXFLAGS) -c atm.cpp -o atm.o

logger.o: logger.cpp logger.hpp
	$(CXX) $(CXXFLAGS) -c logger.cpp -o logger.o

read_write_lock.o: read_write_lock.cpp read_write_lock.hpp
	$(CXX) $(CXXFLAGS) -c read_write_lock.cpp -o read_write_lock.o

bank.o: bank.cpp bank.hpp
	$(CXX) $(CXXFLAGS) -c bank.cpp -o bank.o

clean:
	rm -f $(OBJS) $(TARGET)
