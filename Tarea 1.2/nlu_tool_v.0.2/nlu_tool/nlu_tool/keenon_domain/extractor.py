import logging
from lark import Transformer

LOGGER = logging.getLogger("Divivo_lite.nlu_config.CommandExtractor")
DEFAULT_FALLBACK_INTENT = "Default Fallback Intent"

class CCommandExtractor(Transformer):
    def __init__(self):
        self.intent_mapping = None
        self.location_str = None
        self.selected_keywords = ["", ""]
        self.entities = {
            "object": "",
            "table": "",
            "point": "",
            "time": ""
        }
    
    def reset_values(self):
        LOGGER.debug("Init")
        self.location_str = None
        self.selected_keywords = ["", ""]
        self.entities = {
            "object": "",
            "table": "",
            "point": "",
            "time": ""
        }
        LOGGER.debug("End")

    def set_intent_mapping(self, intent_mapping):
        LOGGER.debug("Init")
        is_success = False
        if intent_mapping and len(intent_mapping) > 0:
            self.intent_mapping = intent_mapping
            is_success = True
        else:
            LOGGER.error("The selected intent mapping is not valid")
        LOGGER.debug("End")
        return is_success

    def command(self, items):
        self.selected_keywords[0] = str(items[0])

    def object(self, items):
        self.entities["object"] = " ".join(items)

    def table(self, items):
        self.entities["table"] = " ".join(items)

    def point(self, items):
        self.entities["point"] = " ".join(items)

    def duration(self, items):
        self.entities["time"] = float(items[0])

    def query(self, items):
        self.selected_keywords[0] = next((token.value for token in items if token.type == 'QUERIES'), "")

    def start(self, items):
        LOGGER.debug("Init")
        intent = DEFAULT_FALLBACK_INTENT
        for intent_map in self.intent_mapping:
            if intent_map["keywords"] == self.selected_keywords:
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
        LOGGER.debug("End")
        return output