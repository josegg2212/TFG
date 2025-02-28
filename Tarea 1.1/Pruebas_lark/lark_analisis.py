from lark import Lark, Transformer

grammar = """
    start: intent

    intent: action object

    action: "enciende" -> turn_on
          | "apaga"    -> turn_off

    object: "la luz"  -> light
          | "el ventilador" -> fan

    %import common.WS
    %ignore WS
"""

parser = Lark(grammar, parser="lalr")

class IntentExtractor(Transformer):
    def turn_on(self, items):
        return {"intent": "turn_on"}
    
    def turn_off(self, items):
        return {"intent": "turn_off"}

    def light(self, items):
        return {"entity": "object", "value": "light"}
    
    def fan(self, items):
        return {"entity": "object", "value": "fan"}

    def intent(self, items):
        intent = items[0]["intent"]
        entity = items[1]  # El objeto
        return {"intent": intent, "entities": [entity]}

def analyze_sentence(sentence):
    tree = parser.parse(sentence)
    extractor = IntentExtractor()
    result = extractor.transform(tree)
    return result

# Ejemplo de uso
sentence = "enciende la luz"
result = analyze_sentence(sentence)

print(result)  # {'intent': 'turn_on', 'entities': [{'entity': 'object', 'value': 'light'}]}
