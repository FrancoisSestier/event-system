.PHONY: all
all: format

.PHONY: format
format:
	clang-format include/antity/*.h  include/antity/core/*.h include/antity/utility/*.h -i -style=file 

.PHONY: test
test:
	cmake -Stest -Bbuild/test
	cmake --build build/test
	cmake --build build/test --target test -- ARGS="-V"
