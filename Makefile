TARGET = task.cpp
EXECUTABLE = prog

exec : $(EXECUTABLE)
	./$(EXECUTABLE)

prog : $(TARGET)
	g++ -fsanitize=address,undefined $< -o $@

clean:
	rm $(EXECUTABLE)
