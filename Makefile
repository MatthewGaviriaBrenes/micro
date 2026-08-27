CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Isrc
SRC = src/scanner.c src/symtab.c src/semantics.c src/codegen.c src/parser.c src/main.c
TARGET = micro

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
	rm -f tests/parser/*.s
	find tests -type f -executable -not -name "*.micro" -not -name "*.c" -delete

test: $(TARGET)
	@echo "=== valid_program.micro ==="
	@echo "7" | ./$(TARGET) tests/parser/valid_program.micro
	@echo "=== missing_semicolon.micro (debe fallar) ==="
	-./$(TARGET) tests/parser/missing_semicolon.micro
	@echo "=== conditional_true.micro ==="
	./$(TARGET) tests/parser/conditional_true.micro
	@echo "=== conditional_false.micro ==="
	./$(TARGET) tests/parser/conditional_false.micro 