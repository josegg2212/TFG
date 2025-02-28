import argparse
import logging
import os
import json
import sys
import importlib.util

from lark import Lark, exceptions

LOGGER = logging.getLogger("NLU_tool")
SCRIPT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))
DEFAULT_FALLBACK_INTENT = "Default Fallback Intent"
FALLBACK_RESULT = {
    "query_result": {
        "intent": {
            "display_name": DEFAULT_FALLBACK_INTENT
        },
        "query_text": "",
        "intent_detection_confidence": 1.0,
        "parameters": {}
    }
}

s_parser = None
s_extractor = None


def build_argparser():
    parser = argparse.ArgumentParser(
        description='Tool to create nlu responses')
    parser.add_argument('-d', '--domain', type=str, required=True, choices={"gsc", "keenon"},
                        help='Grammar domain.')
    parser.add_argument('-c', '--config_file', type=str, required=True,
                        help='File with nlu config.')
    parser.add_argument('-l', '--logging_level', type=lambda x: getattr(logging, x), default=logging.INFO, choices={logging.DEBUG, logging.INFO, logging.ERROR},
                        help="Level of logging messages")
    return parser


def print_query_result(d, indent=0):
    spacing = "    "
    for key, value in d.items():
        if isinstance(value, dict):
            print(f"{spacing * indent}{key}:")
            print_query_result(value, indent + 1)
        else:
            print(f"{spacing * indent}{key}: {value}")


def build_nlu(nlu_config):
    LOGGER.debug("Init")
    is_success = True

    global s_parser
    LOGGER.debug("Loading nlu grammar")
    if "grammar_path" in nlu_config:
        with open(os.path.join(SCRIPT_DIRECTORY, nlu_config["grammar_path"]), "r", encoding="utf-8") as grammar_file:
            grammar = grammar_file.read()
            s_parser = Lark(grammar)
    else:
        LOGGER.error("Error while reading grammar from nlu config")
        is_success = False

    global s_extractor
    LOGGER.debug("Loading extractor")
    if "extractor_path" in nlu_config and "extractor_module_name" in nlu_config:
        module_name = nlu_config["extractor_module_name"]
        intent_mapping = nlu_config["intent_mapping"]

        spec = importlib.util.spec_from_file_location(
            "extractor", os.path.join(SCRIPT_DIRECTORY, nlu_config["extractor_path"]))
        module = importlib.util.module_from_spec(spec)
        sys.modules["domain_callbacks"] = module
        spec.loader.exec_module(module)

        Extractor = getattr(module, module_name)
        s_extractor = Extractor()
        LOGGER.debug("Loading nlu intent mapping in extractor")
        if "intent_mapping" in nlu_config:
            is_success = s_extractor.set_intent_mapping(intent_mapping)
        else:
            LOGGER.error("Error while reading intent mapping from nlu config")
            is_success = False
    else:
        LOGGER.error("Error while loading extractor from nlu config")
        is_success = False

    LOGGER.debug("End")
    return is_success


def main():
    args = build_argparser().parse_args()

    formatter = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    console_handler = logging.StreamHandler()
    console_handler.setLevel(args.logging_level)
    console_handler.setFormatter(formatter)

    LOGGER.setLevel(level=args.logging_level)
    LOGGER.addHandler(console_handler)

    
    json_data = None
    if os.path.exists(args.config_file) and os.path.isfile(args.config_file):
        with open(args.config_file) as f:
            json_data = json.load(f)
    else:
        LOGGER.error("Error while reading config file '%s'", args.config_file)

    nlu_started = False
    if json_data and args.domain in json_data:
        LOGGER.info("Building nlu")
        nlu_started = build_nlu(json_data[args.domain])
    else:
        LOGGER.error("Unable to build nlu for domain '%s'", args.domain)

    if nlu_started:
        print("Available commands (start with #)")
        print(f"\t#exit: Stop")

        global s_extractor, s_parser
        while True:
            user_input = input("\nInsert input -> ")
            if user_input == "#exit":
                LOGGER.info("Exiting")
                break
            else:
                query_result = FALLBACK_RESULT
                try:
                    LOGGER.debug("Generating tree")
                    tree = s_parser.parse(user_input.lower())

                    LOGGER.debug("Extracting nlu response from tree")
                    query_result = s_extractor.transform(tree)
                except exceptions.LarkError as e:
                    LOGGER.info("The grammar does not recognize the input")

                LOGGER.info("Displaying results:")
                print_query_result(query_result)

                LOGGER.debug("Resetting extractor values")
                s_extractor.reset_values()
    else:
        LOGGER.error("Error while reading nlu. Exiting")

if __name__ == "__main__":
    main()
