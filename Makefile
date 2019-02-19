objects = product.o combination.o util.o array.o
flags = -O3 -lgsl -lgslcblas -lm -Wno-unused-result -fopenmp
OBJDIR := obj
BINDIR := bin
SRCDIR := src/cpp
CC := g++
OBJS := $(addprefix $(OBJDIR)/,$(objects))


all: dir pml chow pml_cv pml_graph

pml: $(OBJS) $(OBJDIR)/pml.o 
	$(CC) -o $(BINDIR)/pml $^ $(flags)

chow: $(OBJS) $(OBJDIR)/chow_liu.o
	$(CC) -o $(BINDIR)/chow $^ $(flags)

pml_cv: $(OBJS) $(OBJDIR)/pml_cv.o
	$(CC) -o $(BINDIR)/pml_cv $^ $(flags)

pml_graph: $(OBJS) $(OBJDIR)/pml_graph.o
	$(CC) -o $(BINDIR)/pml_graph $^ $(flags)

$(OBJDIR)/chow_liu.o: $(SRCDIR)/chow_liu.cpp $(SRCDIR)/combination.h $(SRCDIR)/array.h $(SRCDIR)/product.h
	$(CC) -c $(OUTPUT_OPTION) $< $(flags)

$(OBJDIR)/util.o: $(SRCDIR)/util.cpp $(SRCDIR)/util.h
	$(CC) -c $(OUTPUT_OPTION) $< $(flags)

$(OBJDIR)/array.o: $(SRCDIR)/array.cpp $(SRCDIR)/array.h $(SRCDIR)/util.h
	$(CC) -c $(OUTPUT_OPTION) $< $(flags)

$(OBJDIR)/pml.o: $(SRCDIR)/pml.cpp $(SRCDIR)/product.h $(SRCDIR)/combination.h $(SRCDIR)/pml.h $(SRCDIR)/util.h
	$(CC) -c $(OUTPUT_OPTION) $< $(flags)

$(OBJDIR)/pml_cv.o: $(SRCDIR)/pml_cv.cpp $(SRCDIR)/product.h $(SRCDIR)/util.h
	$(CC) -c $(OUTPUT_OPTION) $< $(flags)

$(OBJDIR)/pml_graph.o: $(SRCDIR)/pml_graph.c $(SRCDIR)/product.h $(SRCDIR)/util.h
	$(CC) -c $(OUTPUT_OPTION) $< $(flags)

$(OBJDIR)/product.o: $(SRCDIR)/product.cpp $(SRCDIR)/product.h $(SRCDIR)/array.h
	$(CC) -c $(OUTPUT_OPTION) $< $(flags)

$(OBJDIR)/combination.o: $(SRCDIR)/combination.cpp $(SRCDIR)/combination.h $(SRCDIR)/array.h
	$(CC) -c $(OUTPUT_OPTION) $< $(flags)

clean:
	rm -rf $(BINDIR)/pml_cv $(BINDIR)/pml $(BINDIR)/chow $(OBJS) in out $(OBJDIR)/ $(BINDIR)/ tmp/

dir:
	mkdir $(OBJDIR) ; mkdir $(BINDIR) ; mkdir tmp

