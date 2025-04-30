from exponents.utils import call_exponents

def respond(text: str) -> str:
    # Parser simple
    if "calculate" in text.lower():
        # Exemple fix
        M = 4
        typeM = "QAM"
        SNR = 10.0
        R = 1.0
        N = 2

        values = call_exponents(M, typeM, SNR, R, N)
        return (f"Results:\n"
                f"• Error probability: {values[0]:.4f}\n"
                f"• Exponent: {values[1]:.4f}\n"
                f"• Optimal rho: {values[2]:.4f}")
    
    return "I don't understand you. I'm still a bit dumb. Try with 'Calculate exponents for M=4, QAM, SNR=10, R=1, N=2'."
