from lark import Lark

# Definimos la gramática en formato Lark
grammar = """
    start: intent
    
    intent: saludo | pregunta | comando
    
    saludo: "hola" | "buenos días" | "buenas tardes"
    
    pregunta: "cómo estás" | "qué tal" | "qué haces"
    
    comando: accion objeto
    
    accion: "enciende" | "apaga" | "sube" | "baja"
    
    objeto: "la luz" | "el televisor" | "el volumen"

    %ignore " "  
"""
# Creamos el parser con la gramática
parser = Lark(grammar, parser="earley")

# Frases de prueba
sentences = ["enciende la luz", "baja el volumen", "hola"]

for sentence in sentences:
    try:
        tree = parser.parse(sentence)
        print(f"'{sentence}' -> {tree.pretty()}")
    except Exception as e:
        print(f"Error al parsear '{sentence}': {e}")
