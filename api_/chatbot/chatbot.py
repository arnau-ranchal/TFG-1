from exponents.utils import call_exponents

def respond(text: str) -> str:
    # Muy simple parser (esto lo puedes reemplazar por NLP o regex más elaborada)
    if "calcula" in text.lower():
        # Ejemplo fijo — puedes extraer esto de texto dinámicamente
        M = 4
        typeM = "QAM"
        SNR = 10.0
        R = 1.0
        N = 2

        values = call_exponents(M, typeM, SNR, R, N)
        return (f"Resultados:\n"
                f"• Probabilidad de error: {values[0]:.4f}\n"
                f"• Exponent: {values[1]:.4f}\n"
                f"• Rho óptima: {values[2]:.4f}")
    
    return "🤖 No entendí la pregunta. Intenta con: 'Calcula exponents para M=4, QAM, SNR=10, R=1, N=2'"
