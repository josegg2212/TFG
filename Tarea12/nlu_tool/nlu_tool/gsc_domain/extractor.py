import logging

from lark import Transformer

LOGGER = logging.getLogger("Divivo_lite.nlu_config.CGSCExtractor")
DEFAULT_FALLBACK_INTENT = "Default Fallback Intent"
RULES_NAMES = {
    "measurement_id": "MEASUREMENT_ID",
    "command_keyword": "COMMAND_KEYWORD",
    "unit": "UNIT"
}

class CGSCExtractor(Transformer):
    def __init__(self):
        self.intent_mapping = None
        self.keywords = ["", ""]
        self.entities = {
            "value": "",
            "satisfied": "",
            "unit_size": "",
            "measure_name": "",
            "piece_number": "",
            "tool_code": "",
            "range_min": "",
            "range_max": ""
        }
        self.received_values = []
    
    def reset_values(self):
        LOGGER.debug("Init")
        self.keywords = ["", ""]
        self.entities = {
            "value": "",
            "satisfied": "",
            "unit_size": "",
            "measure_name": "",
            "piece_number": "",
            "tool_code": "",
            "range_min": "",
            "range_max": ""
        }
        self.received_values = []
        LOGGER.debug("End")

    def set_intent_mapping(self, intent_mapping):
        LOGGER.debug("Init")
        is_success = False
        if intent_mapping and len(intent_mapping)>0:
            self.intent_mapping = intent_mapping
            is_success = True
        else:
            LOGGER.error("The selected intent mapping is not valid")
        LOGGER.debug("End")
        return is_success

    def command(self, items):
        self.keywords[0] = next((token.value for token in items if token and token.type == RULES_NAMES["command_keyword"]), "")

    def tool(self, items):
        self.entities["tool_code"] = str(items[0])

    def piece(self, items):
        self.entities["piece_number"] = int(items[0])

    def measurement(self, items):
        self.entities["measure_name"] = next((token.value for token in items if token and token.type == RULES_NAMES["measurement_id"]), "")
        
    def single_value(self, items):
        self.entities["value"] = self.received_values[0]

    def range_value(self, items):
        self.entities["range_min"] = self.received_values[0]
        self.entities["range_max"] = self.received_values[1]

    def value(self, items):
        received_value = []
        for item in items:
            if item.type != RULES_NAMES["unit"]:
                received_value.append(item)
        self.received_values.append(float("".join(received_value)))
        self.entities["unit_size"] = next((token.value for token in items if token and token.type == RULES_NAMES["unit"]), "")

    def satisfied(self, items):
        self.entities["satisfied"] = str(items[0])

    def object(self, items):
        self.keywords[1] = str(items[0])

    def start(self, items):
        LOGGER.debug("Init")

        intent = DEFAULT_FALLBACK_INTENT
        for intent_map in self.intent_mapping:
            if intent_map["keywords"] == self.keywords:
                intent = intent_map["intent"]

        output = {
            "query_result": {
                "intent": {
                    "display_name": intent
                },
                "query_text": "",
                "intent_detection_confidence": 1.0,
                "parameters": self.entities
            }
        }

        self.reset_values()
        LOGGER.debug("End")
        return output