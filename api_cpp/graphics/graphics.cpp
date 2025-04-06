// graphics.cpp
#include <iostream>

extern "C" {
    // TODO: Canviar per la funció de veritat + ajustar parametres d'entrada
    void plot_graph(double* x, double* y, int size) {
        std::cout << "Plotting graph with data:" << std::endl;
        for (int i = 0; i < size; i++) {
            std::cout << "x[" << i << "] = " << x[i] 
                      << ", y[" << i << "] = " << y[i] << std::endl;
        }
    }
}
