from fastapi import FastAPI, HTTPException, Query # Web framework - API test
import matlab.engine


app = FastAPI()


# Initialize MATLAB
try:
    eng = matlab.engine.start_matlab()
except Exception as e:
    raise HTTPException(status_code=500, detail=f"Init MATLAB error: {str(e)}")




# ENDPOINTS
# MOD 1
@app.get("/exponents")
def compute_exponents(SNR: str, R: str, M: str, threshold: float):
    try:
        SNR_mat = matlab.double([float(i) for i in SNR.split(",")])
        R_mat = matlab.double([float(i) for i in R.split(",")])
        M_mat = matlab.double([float(i) for i in M.split(",")])
       
        # exponents function (MATLAB)
        exp, rho = eng.exponents(SNR_mat, R_mat, M_mat, threshold)
        return {"Exponent": exp, "With optimal rho": rho}
   
    except ValueError:
        raise HTTPException(status_code=400, detail="Error in data conversion. Verify values.")
    except matlab.engine.MatlabExecutionError as e:
        raise HTTPException(status_code=500, detail=f"MATLAB (engine) error: {str(e)}")
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Unexpected error: {str(e)}")



# MOD 2
@app.get("/graphics")
def generate_graphic(y: str, x: str, range_y: str, range_x: str, color: str, lineType: str, plotType: str):
    try:
        y_mat = matlab.double([float(i) for i in y.split(",")])
        x_mat = matlab.double([float(i) for i in x.split(",")])
        range_y_mat = matlab.double([float(i) for i in range_y.split(",")])
        range_x_mat = matlab.double([float(i) for i in range_x.split(",")])


        # Graphic function (MATLAB)
        eng.graphics(y_mat, x_mat, range_y_mat, range_x_mat, color, lineType, plotType, nargout=0)


        return {"message": "Graph generated successfully"}
    
    except ValueError:
        raise HTTPException(status_code=400, detail="Error in data conversion. Verify values.")
    except matlab.engine.MatlabExecutionError as e:
        raise HTTPException(status_code=500, detail=f"MATLAB (engine) error: {str(e)}")
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Unexpected error: {str(e)}")



@app.on_event("shutdown") # Closes MATLAB when the API stops running
def shutdown():
    eng.quit()

