CFLAGS=-D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

all: main clean_o

main:
	g++ node.cpp stack.cpp tree.cpp list.cpp logfiles.cpp main.cpp wolfram.cpp $(CFLAGS)

bashkir: main.o logfiles.o tree.o wolfram.o stack.o solver.o
	g++ main.o logfiles.o tree.o wolfram.o stack.o solver.o my_assert.cpp -o bashkir $(CFLAGS)

main.o: main.cpp
	g++ -c main.cpp

wolfram.o: wolfram.cpp
	g++ -c wolfram.cpp

frontend.o: frontend.cpp
	g++ -c frontend.cpp

middlend.o middlend.cpp
	g++ -c middlend.cpp

backend.o backend.cpp
	g++ -c backend.cpp

stack.o: stack.cpp
	g++ -c stack.cpp

tree.o: tree.cpp
	g++ -c tree.cpp

logfiles.o: logfiles.cpp
	g++ -c logfiles.cpp

hash.o: hash.cpp
	g++ -c hash.cpp

clean:
	rm -rf *.o bashkir

clean_o:
	rm -rf *.o
