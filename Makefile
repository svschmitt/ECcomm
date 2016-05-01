CFLAGS=	-g
# All object files that must be linked into final executable
OBJ=	main.o ECpacket.o ECcmdmap.o

# Rule for building executable from object files
# $@ is shorthand for the target of the rule
ECcomm: ${OBJ}
	c++ ${CFLAGS} -o $@ ${OBJ}

# Rule for compiling individual sources files into object files
# $< is shorthand for the first prerequisite
${OBJ}: %.o: %.cpp
	c++ ${CFLAGS} -c -O3 $<

# Rule to clean up all output files
clean:
	rm -f ECcomm ${OBJ}

