// Libraries
#include <iostream> 
#include "peglib.h" // For PEGLIB
#include <string>
#include <vector>
#include <map>

#include <fstream>
#include <sstream>
#include <string>

#include "json.hpp" // For JSON

#include "CGSCExtractor.h" // For CGSCExtractor

// Namespaces
using json = nlohmann::json;
using namespace std;
using namespace peg;



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

vector<IntentMapping> intent_map;
// Load database of intent mappings
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
    // Load grammar from file
    string grammar_path = "grammar_gsc.peg";
    string grammar = load_grammar(grammar_path);

    if (grammar.empty()) {
        cerr << "No se pudo cargar la gramática. Saliendo..." << endl;
        return 1;
    }

    // Load intent map
    string intent_map_path = "intent_map.json"; 
    vector<IntentMapping> intent_map = load_intent_map(intent_map_path);

    if (intent_map.empty()) {
        cerr << "Error al cargar el intent_map. Saliendo..." << endl;
        return 1;
    }

    // Grammar parser
    parser p(grammar);

    // Extractor class
    CGSCExtractor extractor;
    extractor.on_extractor(p);

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
