#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include "extractor.h" // Para usar RuleConfig e IntentMapping

using namespace std;

// Estructura para almacenar las rutas de archivo
struct ConfigPaths {
    string grammar_path;
    string config_path;
    string intent_map_path;
};

class config {
public:
    // Carga las rutas desde un JSON (por ejemplo, config_paths.json).
    ConfigPaths loadConfigPaths(const string& configFilePath, const string& userChoice);

    // Carga el contenido de un archivo de gramática en un std::string.
    string load_grammar(const string &filepath);

    // Carga la configuración de reglas (rules) desde un JSON.
    vector<RuleConfig> load_config(const string &filepath);

    // Carga la base de datos de mapeo de intenciones.
    vector<IntentMapping> load_intent_map(const string &filepath);
};

#endif // CONFIG_H
