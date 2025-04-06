from fastapi import FastAPI, Query
from fastapi.staticfiles import StaticFiles
from fastapi.responses import FileResponse
from typing import List
import os
import ctypes
import numpy as np
import matplotlib.pyplot as plt

app = FastAPI()

# Cargar la biblioteca compartida
lib = ctypes.CDLL('./build/libfunctions.so')


# Definir tipos para plot_graph (si existe en C++)
try:
    lib.plot_graph.argtypes = (ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_double), ctypes.c_int)
    lib.plot_graph.restype = None
    cpp_graphics_available = True
except AttributeError:
    cpp_graphics_available = False

# Servir archivos estáticos (CSS, imágenes)
app.mount("/static", StaticFiles(directory="static"), name="static")


# ENDPOINTS
# ------------------------ INDEX -------------------------------------------------------------------------
@app.get("/")
async def serve_index():
    index_path = "static/index.html"
    if os.path.exists(index_path):
        return FileResponse(index_path)
    return {"error": "index.html no encontrado"}



# ------------------------ EXPONENTS -------------------------------------------------------------------------
# Definir tipos de argumentos y retorno para exponents
lib.exponents.argtypes = (ctypes.c_float, ctypes.c_float, ctypes.c_float, ctypes.c_float)
lib.exponents.restype = ctypes.c_float

@app.get("/exponents")
async def exponents(
    M: float = Query(1, description="Modulation"),  # Asegurar que sea int
    SNR: float = Query(1.0, description="Signal to Noise Ratio"),  # Asegurar float con decimal
    R: float = Query(1.0, description="Rate"),  # Asegurar float con decimal
    threshold: float = Query(1, description="Threshold")  # Asegurar que sea int
):
    """  
    Calcula l'exponent `E` i la seva `rho òptima`.
    """
    result = lib.exponents(ctypes.c_float(M), ctypes.c_float(SNR), ctypes.c_float(R), ctypes.c_float(threshold))
    return {"result": result}


# ------------------------ GRAPHICS -------------------------------------------------------------------------
@app.get("/plot")
async def plot(x: List[float] = Query(..., description="Valores de X"), 
               y: List[float] = Query(..., description="Valores de Y")):
    """
    Recibe dos listas `x` y `y`, y genera una gráfica.
    - Si `plot_graph` está implementado en C++, la llama.
    - Si no, usa matplotlib como alternativa.
    """
    if len(x) != len(y):
        return {"error": "Los arrays x e y deben tener la misma longitud"}

    if cpp_graphics_available:
        # Convertir a numpy y pasar como punteros a la función C++
        x_array = np.array(x, dtype=np.double)
        y_array = np.array(y, dtype=np.double)

        lib.plot_graph(x_array.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
                       y_array.ctypes.data_as(ctypes.POINTER(ctypes.c_double)),
                       len(x_array))
        return {"message": "Gráfica generada en C++"}

    else:
        # Generar gráfica con matplotlib si C++ no está disponible
        plt.figure()
        plt.plot(x, y, label="Generated Graph")
        plt.legend()
        plt.savefig("static/plot.png")

        return {"message": "Gráfica generada con matplotlib", "image": "/static/plot.png"}
