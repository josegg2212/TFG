#include "config.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "json.hpp" // Biblioteca de JSON (nlohmann::json)

using json = nlohmann::json;
using namespace std;

// Implementación de loadConfigPaths
ConfigPaths config::loadConfigPaths(const string& configFilePath, const string& userChoice)
{
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

// Implementación de load_grammar
string config::load_grammar(const string &filepath)
{
    ifstream file(filepath);
    if (!file) {
        cerr << "Error: No se pudo abrir el archivo de gramática: " << filepath << endl;
        return "";
    }

    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Implementación de load_config
vector<RuleConfig> config::load_config(const string &filepath)
{
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

// Implementación de load_intent_map
vector<IntentMapping> config::load_intent_map(const string &filepath)
{
    vector<IntentMapping> intent_map;
    ifstream file(filepath);
    if (!file) {
        cerr << "Error: No se pudo abrir el archivo JSON: " << filepath << endl;
        return {};
    }

    json j;
    file >> j;

    for (const auto &item : j) {
        intent_map.push_back({
            item["keywords"].get<vector<string>>(),
            item["intent"].get<string>()
        });
    }

    return intent_map;
}
