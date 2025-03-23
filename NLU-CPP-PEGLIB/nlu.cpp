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



// Namespaces
using json = nlohmann::json;
using namespace std;
using namespace peg;



// Function to load config paths
// Struct to store config paths
struct ConfigPaths {
    string grammar_path;
    string config_path;
    string intent_map_path;
};

ConfigPaths loadConfigPaths(const string& configFilePath, const string& userChoice) {
    ConfigPaths paths = {"", "", ""}; 

    ifstream file(configFilePath);
    if (!file) {
        cerr << "Error: No se pudo abrir el archivo " << configFilePath << endl;
        return paths;
    }

    try {
        json configJson;
        file >> configJson; 

        if (configJson.contains(userChoice)) {
            for (auto& item : configJson[userChoice].items()) {
                string key = item.key();
                string value = item.value().get<string>();

                if (key == "grammar") {
                    paths.grammar_path = value;
                } else if (key == "config") {
                    paths.config_path = value;
                } else if (key == "intent_map") {
                    paths.intent_map_path = value;
                }
            }
        } else {
            cerr << "Error: La opción '" << userChoice << "' no está en el JSON." << endl;
        }
    } catch (const json::parse_error& e) {
        cerr << "Error al parsear el JSON: " << e.what() << endl;
    }

    return paths;
}

// Function to load grammar file
string load_grammar(const string &filepath) {
    ifstream file(filepath);
    if (!file) {
        cerr << "Error: No se pudo abrir el archivo de gramática: " << filepath << endl;
        return "";
    }

    stringstream buffer;
    buffer << file.rdbuf(); 
    return buffer.str();    
}

// Load json configuration (keywords and entities to capture)
vector<RuleConfig> load_config(const string &filepath) {
    vector<RuleConfig> config;
    
    ifstream file(filepath);
    if (!file) {
        cerr << "Error al abrir JSON de config: " << filepath << endl;
        return {};
    }
    json j;
    file >> j;

    for (auto &item : j["rules"]) {
        RuleConfig rc;
        rc.rule_name = item["rule_name"].get<string>();
        rc.type      = item["type"].get<string>();
        config.push_back(rc);
    }
    return config;
}

// Load database of intent mappings
vector<IntentMapping> intent_map;
vector<IntentMapping> load_intent_map(const string &filepath) {
    ifstream file(filepath);
    if (!file) {
        cerr << "Error: No se pudo abrir el archivo JSON: " << filepath << endl;
        return {};
    }

    json j;
    file >> j;

    for (const auto &item : j) {
        intent_map.push_back({item["keywords"].get<vector<string>>(), item["intent"].get<string>()});
    }

    return intent_map;
}



// Main 
int main()
{
    
    // Request choice from user
    string userChoice;
    cout << "Select an option (gsc or keenon): " << endl;
    getline(cin, userChoice);

    // Load configuration paths
    string configFilePath = "config_paths.json";
    ConfigPaths paths = loadConfigPaths(configFilePath, userChoice);

    // Load grammar from file
    string grammar_path = paths.grammar_path;
    string grammar = load_grammar(grammar_path);

    if (grammar.empty()) {
        cerr << "No se pudo cargar la gramática. Saliendo..." << endl;
        return 1;
    }

    // Load json configuration (keywords and entities to capture)
    string rule_config_path = paths.config_path;
    vector<RuleConfig> rule_config = load_config(rule_config_path);

    // Load intent map
    string intent_map_path = paths.intent_map_path; 
    vector<IntentMapping> intent_map = load_intent_map(intent_map_path);

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
