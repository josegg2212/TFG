// Libraries
#include <iostream> 
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

#include "peglib.h" // For PEGLIB

#include "json.hpp" // For JSON

#include "extractor.h" // For extractor

#include "config.h" // For config

// Namespaces
using json = nlohmann::json;
using namespace std;
using namespace peg;

// Main 
int main()
{
    // Load configuration
    config config;

    // Request choice from user
    string userChoice;
    cout << "Select an option (gsc or keenon): " << endl;
    getline(cin, userChoice);

    // Load configuration paths
    string configFilePath = "config_paths.json";
    ConfigPaths paths = config.loadConfigPaths(configFilePath, userChoice);

    // Load grammar from file
    string grammar_path = paths.grammar_path;
    string grammar = config.load_grammar(grammar_path);

    if (grammar.empty()) {
        cerr << "No se pudo cargar la gramática. Saliendo..." << endl;
        return 1;
    }

    // Load json configuration (keywords and entities to capture)
    string rule_config_path = paths.config_path;
    vector<RuleConfig> rule_config = config.load_config(rule_config_path);

    // Load intent map
    string intent_map_path = paths.intent_map_path; 
    vector<IntentMapping> intent_map = config.load_intent_map(intent_map_path);

    if (intent_map.empty()) {
        cerr << "Error al cargar el intent_map. Saliendo..." << endl;
        return 1;
    }

    // Grammar parser
    parser p(grammar);

    // Extractor class
    extractor extractor(p, rule_config);

    // Run a loop to continuously read inputs and parse them
    string input;
    while (1)
    {
        // Read input from user and parse it
        cout << "Insert an input: " << endl;
        getline(cin, input);

        // Convert input to lowercase
        input = toLower(input);

        // Parse the input
        if (p.parse(input))
        {
            // Get output (keywords, intent and entities) from the extractor and print it
            auto output = extractor.get_output(intent_map);
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

