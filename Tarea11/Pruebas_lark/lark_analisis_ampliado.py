from lark import Lark, Transformer

grammar = """
    start: comando

    comando: action object modifier? location?

    action: "enciende"  -> turn_on
          | "apaga"     -> turn_off
          | "sube"      -> increase
          | "baja"      -> decrease
          | "ajusta"    -> adjust

    object: "la luz"        -> light
          | "el ventilador" -> fan
          | "el volumen"    -> volume
          | "la temperatura" -> temperature
          | "las persianas"  -> blinds
          | ("el" | "la" | "los" | "las") WORD -> dynamic_object

    modifier: "un poco"      -> slightly
            | "al máximo"    -> max
            | "a la mitad"   -> medium
            | "al" NUMBER "%" -> percent_modifier
            | "a" NUMBER      -> number_modifier
            | "en" NUMBER     -> number_modifier

    location: "en" WORD+ -> dynamic_location

    %import common.WORD  // Captura cualquier palabra como entidad
    %import common.NUMBER
    %import common.WS
    %ignore WS
"""

parser = Lark(grammar, parser="lalr")

class IntentExtractor(Transformer):
    def turn_on(self, items):
        return {"intent": "turn_on"}

    def turn_off(self, items):
        return {"intent": "turn_off"}

    def increase(self, items):
        return {"intent": "increase"}

    def decrease(self, items):
        return {"intent": "decrease"}

    def adjust(self, items):
        return {"intent": "adjust"}

    def light(self, items):
        return {"entity": "object", "value": "light"}

    def fan(self, items):
        return {"entity": "object", "value": "fan"}

    def volume(self, items):
        return {"entity": "object", "value": "volume"}

    def temperature(self, items):
        return {"entity": "object", "value": "temperature"}

    def blinds(self, items):
        return {"entity": "object", "value": "blinds"}

    def dynamic_object(self, items):
        return {"entity": "object", "value": items[0]}
    
    def slightly(self, items):
        return {"entity": "modifier", "value": "slightly"}

    def max(self, items):
        return {"entity": "modifier", "value": "max"}

    def medium(self, items):
        return {"entity": "modifier", "value": "medium"}
    
    def percent_modifier(self, items):
        return {"entity": "modifier", "value": f"{items[0]}%"}

    def number_modifier(self, items):
        return {"entity": "modifier", "value": items[0]}

    def dynamic_location(self, items):
        return {"entity": "location", "value": items[1]}
    
    def comando(self, items):
        intent = items[0]["intent"]
        entity = items[1]  # Objeto
        modifier = items[2] if len(items) > 2 else None  # Modificador opcional
        location = items[3] if len(items) > 3 else None  # Ubicación opcional

        result = {"intent": intent, "entities": [entity]}
        if modifier:
            result["entities"].append(modifier)
        if location:
            result["entities"].append(location)

        return result

def analyze_sentence(sentence):
    tree = parser.parse(sentence)
    extractor = IntentExtractor()
    result = extractor.transform(tree)
    return result

# Ejemplos de uso
sentences = [
    "enciende la luz en la habitacion",
    "apaga la radio",
    "sube el volumen al máximo",
    "baja la temperatura a 22",
    "ajusta las persianas a la mitad"
]

for sentence in sentences:
    print(analyze_sentence(sentence))
