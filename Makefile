CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall

EXE = djk
SRC = djk.cpp

P1 = pruebas/prueba1_aleatorio.txt
P2 = pruebas/prueba2_conexo.txt
P3 = pruebas/prueba3_ids_dispersos.txt
P4 = pruebas/prueba4_cadena.txt
P5 = pruebas/prueba5_sin_camino.txt
P6 = pruebas/prueba6_pesos_grandes.txt

.PHONY: all grafo prueba1 prueba2 prueba3 prueba4 prueba5 prueba6 todas clean

# Compila solo si djk.cpp cambió
all: $(EXE)

$(EXE): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(EXE)

# Archivo de muestra del profesor
grafo: $(EXE)
	./$(EXE) grafo.txt

prueba1: $(EXE)
	./$(EXE) $(P1)

prueba2: $(EXE)
	./$(EXE) $(P2)

prueba3: $(EXE)
	./$(EXE) $(P3)

prueba4: $(EXE)
	./$(EXE) $(P4)

prueba5: $(EXE)
	./$(EXE) $(P5)

prueba6: $(EXE)
	./$(EXE) $(P6)

# Corre la muestra y las 6 pruebas, una tras otra
todas: $(EXE)
	@for f in grafo.txt $(P1) $(P2) $(P3) $(P4) $(P5) $(P6); do \
		echo "========== $$f"; \
		./$(EXE) $$f; \
		echo; \
	done

clean:
	rm -f $(EXE)
