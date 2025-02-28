// Libraries
#include <iostream> 
#include "peglib.h" // For PEGLIB
#include <string>
#include <vector>
#include <map>

// Namespaces
using namespace std;
using namespace peg;



// Define the grammar for parsing user input
const char *grammar = R"(
    start <- command

    command <- COMMAND_KEYWORD command_content

    command_content <- tool / piece / object / measurement

    tool <- TOOL_ID
    piece <- PIECE_ID
    measurement <- MEASUREMENT_ID WS measurement_value?
    object <- OBJECTS

    measurement_value <- single_value / range_value / satisfied
    single_value <- VALUE WS unit?
    range_value <- "mínimo" min_value "máximo" max_value
    satisfied <- SATISFIED_KEYWORDS

    unit <- UNIT
    min_value <- VALUE WS unit?
    max_value <- VALUE WS unit?

    COMMAND_KEYWORD <- "cota" / "pieza" / "herramienta" / "abrir" / "cerrar" / "repetir" / "reiniciar" / "siguiente" / "quitar"
    TOOL_ID <- "sc" [0-9][0-9][0-9][0-9]
    MEASUREMENT_ID <- [A-Za-z] ([A-Za-z] / [0-9])?
    PIECE_ID <- [0-9]+
    OBJECTS <- "orden" / "sesión" / "plano" / "herramienta" / "cota" / "rango"

    VALUE <- [0-9]+ ([.,][0-9]+)?
    UNIT <- "milímetros" / "mm" / "pulgadas" / "in"
    SATISFIED_KEYWORDS <- "conforme" / "no conforme"

    WS <- [ \t]*
    %whitespace <- [ \t]*
    )";



// Structure to represent an intent and its keywords (for intent mappings)
struct IntentMapping {
    vector<string> keywords;
    string intent;
};

// Database of intent mappings
vector<IntentMapping> intent_map = {
    {{"cota", ""}, "userdriven_add-value"},
    {{"pieza", ""}, "userdriven_select-piece"},
    {{"herramienta", ""}, "userdriven_add-tool"},
    {{"abrir", "orden"}, "userdriven_open-wo"},
    {{"abrir", "plano"}, "userdriven_open-blueprint"},
    {{"cerrar", "plano"}, "userdriven_close-blueprint"},
    {{"siguiente", "plano"}, "userdriven_close-blueprint"},
    {{"cerrar", "sesión"}, "userdriven_log-out"},
    {{"cerrar", ""}, "userdriven_close-wo"},
    {{"cerrar", "orden"}, "userdriven_close-wo"},
    {{"repetir", "cota"}, "userdriven_repeat-measurement"},
    {{"reiniciar", "rango"}, "userdriven_reset-range"},
    {{"quitar", "herramienta"}, "userdriven_tool-remove"}
};



// Class that acts as a transformer to extract intents and entities
class CGSCExtractor
{
public:
    string intent;
    vector<string> keywords;
    map<string, string> entities;
    string DEFAULT_FALLBACK_INTENT = "Default Fallback Intent";

    // Class constructor
    CGSCExtractor()
    {
        // Atributes: keywords for intent and entities
        keywords.resize(2, "");

        entities["value"] = "";
        entities["satisfied"] = "";
        entities["unit_size"] = "";
        entities["measure_name"] = "";
        entities["piece_number"] = "";
        entities["tool_code"] = "";
        entities["range_min"] = "";
        entities["range_max"] = "";
    }

    // Transformation methods for each parsing rule
    void on_COMMAND_KEYWORD(const SemanticValues &sv)
    {
        keywords[0] = sv.token(); 
    }

    void on_OBJECTS(const SemanticValues &sv)
    {
        keywords[1] = sv.token(); 
    }
    void on_TOOL_ID(const SemanticValues &sv)
    {
        entities["tool_code"] = sv.token(); 
    }

    void on_PIECE_ID(const SemanticValues &sv)
    {
        entities["piece_number"] = sv.token(); 
    }

    void on_MEASUREMENT_ID(const SemanticValues &sv)
    {
        entities["measure_name"] = sv.token(); 
    }

    void on_single_value(const SemanticValues &sv)
    {
        entities["value"] = any_cast<string>(sv[0]); 
    }

    void on_min_value(const SemanticValues &sv)
    {
        entities["range_min"] = any_cast<string>(sv[0]);
    }

    void on_max_value(const SemanticValues &sv)
    {
        entities["range_max"] = any_cast<string>(sv[0]);
    }

    void on_UNIT(const SemanticValues &sv)
    {
        entities["unit_size"] = sv.token(); 
    }

    void on_SATISFIED_KEYWORDS(const SemanticValues &sv)
    {
        entities["satisfied"] = sv.token(); 
    }

    // Method to determine final intent based on extracted information (output information)
    map<string, string> get_output()
    {
        map<string, string> output;

        // Search intent in intent map thanks to keywords extracted
        intent = DEFAULT_FALLBACK_INTENT;       
        if (keywords.size() > 0){
            for (const auto& mapping : intent_map) {
                if (mapping.keywords[0] == keywords[0] && (mapping.keywords[1] == "" || mapping.keywords[1] == keywords[1])) {
                    intent = mapping.intent;
                    break;                   
                }
            }
            // Add keywords and intent to output
            output["keyword1"] = keywords[0];
            output["keyword2"] = keywords[1];
            output["intent"] = intent;      
        }

        // Add entities to output
        for (const auto &pair : entities)
        {
            output[pair.first] = pair.second;
        }

        // Reset values to prepare for next parsing
        reset_values();
        return output;
    }

    // Method to reset keywors and entities for the next parsing
    void reset_values()
    {
        fill(keywords.begin(), keywords.end(), "");
        entities["value"] = "";
        entities["satisfied"] = "";
        entities["unit_size"] = "";
        entities["measure_name"] = "";
        entities["piece_number"] = "";
        entities["tool_code"] = "";
        entities["range_min"] = "";
        entities["range_max"] = "";
    }
};



// Main 
int main()
{
    // Grammar parser
    parser p(grammar);

    // Extractor class
    CGSCExtractor extractor;

    // Assign parsing rules to callback functions
    p["COMMAND_KEYWORD"] = [&extractor](const SemanticValues &sv)
    {
        extractor.on_COMMAND_KEYWORD(sv);
    };
    p["OBJECTS"] = [&extractor](const SemanticValues &sv)
    {
        extractor.on_OBJECTS(sv);
    };
    p["TOOL_ID"] = [&extractor](const SemanticValues &sv)
    {
        extractor.on_TOOL_ID(sv);
    };
    p["PIECE_ID"] = [&extractor](const SemanticValues &sv)
    {
        extractor.on_PIECE_ID(sv);
    };
    p["MEASUREMENT_ID"] = [&extractor](const SemanticValues &sv)
    {
        extractor.on_MEASUREMENT_ID(sv);
    };
    p["single_value"] = [&extractor](const SemanticValues &sv)
    {
        extractor.on_single_value(sv);
    };

    p["min_value"] = [&extractor](const SemanticValues &sv)
    {
        extractor.on_min_value(sv);
    };

    p["max_value"] = [&extractor](const SemanticValues &sv)
    {
        extractor.on_max_value(sv);
    };
    p["VALUE"] = [&extractor](const SemanticValues &sv)
    {
        return sv.token_to_string();
    };
    p["UNIT"] = [&extractor](const SemanticValues &sv)
    {
        extractor.on_UNIT(sv);
    };
    p["SATISFIED_KEYWORDS"] = [&extractor](const SemanticValues &sv)
    {
        extractor.on_SATISFIED_KEYWORDS(sv);
    };

    string input;

    // Run a loop to continuously read inputs and parse them
    while (1)
    {
        // Read input from user and parse it
        cout << "Insert an input: ";
        getline(cin, input);

        if (p.parse(input))
        {
            // Get output (keywords, intent and entities) from the extractor and print it
            auto output = extractor.get_output();
            for (const auto &pair : output)
            {
                cout << pair.first << ": " << pair.second << endl;
            }
        }
        else
        {
            cout << "Parsing failed." << endl;
        }
    }
    return 0;
}
