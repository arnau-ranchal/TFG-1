from fastapi import FastAPI, Query, HTTPException, Response
from fastapi.staticfiles import StaticFiles
from fastapi.responses import FileResponse
from typing import List, Literal
from pydantic import BaseModel
import os
import ctypes
import numpy as np
import subprocess
import json


app = FastAPI()

# Cargar la biblioteca compartida
lib = ctypes.CDLL('./build/libfunctions.so')


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
# Definir tipus d'arguments i retorn per exponents
lib.exponents.argtypes = (
    ctypes.c_float, 
    ctypes.c_char_p, 
    ctypes.c_float, 
    ctypes.c_float, 
    ctypes.c_float
)
lib.exponents.restype = ctypes.POINTER(ctypes.c_float)

# Funció que crida a la funció exponents
@app.get("/exponents")
async def exponents(
    M: float = Query(1, description="Modulation"),  # Asegurar que sea int
    typeM: str = Query("PAM", description="Tipo de modulación: PAM, QAM, etc."),
    SNR: float = Query(1.0, description="Signal to Noise Ratio"),  # Asegurar float con decimal
    R: float = Query(1.0, description="Rate"),  # Asegurar float con decimal
    N: float = Query(1, description="quadrature")  # Asegurar que sea int
):
    """  
    Calcula l'exponent `Pe`, 'E' i `RHO`.
    """
    # Resultat + Retron
    # Allocate output buffer
    result = (ctypes.c_float * 3)()
    lib.exponents(
        ctypes.c_float(M),
        typeM.encode('utf-8'),
        ctypes.c_float(SNR),
        ctypes.c_float(R),
        ctypes.c_float(N),
        result  # Pass the buffer
    )
    values = list(result)  # Convert to Python list

    return {
        "Probabilidad de error": values[0],
        "Exponents": values[1],
        "rho óptima": values[2]
    }

# ------------------------ GRAPHICS -------------------------------------------------------------------------
def call_exponents(M: float, typeModulation: str, SNR: float, Rate: float, N: float) -> list[float]:
    """
    Wrapper para la función C++ 'exponents' que retorna una lista de 3 valores:
    [Probabilidad de error, Exponent, rho óptima]
    """
    result_ptr = lib.exponents(
        ctypes.c_float(M),
        typeModulation.encode('utf-8'),  # Convertir string a C-style
        ctypes.c_float(SNR),
        ctypes.c_float(Rate),
        ctypes.c_float(N)
    )
    return [result_ptr[i] for i in range(3)]

# Classe per la petició de generar una gràfica respecte una funció ja pre-programada
class FunctionPlotRequest(BaseModel):
    y: str
    x: str
    rang_x: list[float]
    points: int
    typeModulation: str
    M: float
    SNR: float
    Rate: float
    N: float
    color: str = "steelblue"
    lineType: str = "-"
    plotType: str = "linear"

# Funció que crida a clitPlot.js per generar grafics en SVG
@app.post("/plot_function")
async def generate_plot_from_function(plot_data: FunctionPlotRequest):
    try:
        # Genera els punts x
        x_vals = np.linspace(plot_data.rang_x[0], plot_data.rang_x[1], plot_data.points)
        y_vals = []

        for x_point in x_vals:
            # Ajusta els valors segons la variable independent
            args = {
                "M": plot_data.M,
                "SNR": plot_data.SNR,
                "Rate": plot_data.Rate,
                "N": plot_data.N
            }
            args[plot_data.x] = x_point

            # Allocate a new buffer for each call
            result = (ctypes.c_float * 3)()
            lib.exponents(
                ctypes.c_float(args["M"]),
                plot_data.typeModulation.encode('utf-8'),
                ctypes.c_float(args["SNR"]),
                ctypes.c_float(args["Rate"]),
                ctypes.c_float(args["N"]),
                result  # Pass the buffer
            )
            y_map = {
                "ErrorProb": result[0],
                "Exponents": result[1],
                "Rho": result[2]
            }
            y_vals.append(y_map[plot_data.y])

        return {
            "x": x_vals.tolist(),
            "y": y_vals
        }

    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Error generating plot data: {str(e)}")