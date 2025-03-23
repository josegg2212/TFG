#ifndef CGSCEXTRACTOR_H
#define CGSCEXTRACTOR_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "peglib.h"

using namespace std;
using namespace peg;

// Structure to represent an intent and its keywords (for intent mappings)
struct IntentMapping {
    vector<string> keywords;
    string intent;
};

// Structure to represent a rule configuration
struct RuleConfig {
    string rule_name;
    string type; 
};

string toLower(string str);

// Class that acts as a transformer to extract intents and entities
class CGSCExtractor {
public:
    string intent;
    vector<string> keywords;
    map<string, string> entities;
    string DEFAULT_FALLBACK_INTENT = "Default Fallback Intent";

    CGSCExtractor(parser &p, const vector<RuleConfig> &rule_config);
    map<string, string> get_output(vector<IntentMapping> intent_map);
    void reset_values();

private:
    void register_keyword(const SemanticValues &sv, int keyword_index);
    void register_entity(const SemanticValues &sv, const string &rule_name);
    void register_node_entity(const SemanticValues &sv, const string &rule_name);
};

#endif
