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

// Class that acts as a transformer to extract intents and entities
class CGSCExtractor {
public:
    string intent;
    vector<string> keywords;
    map<string, string> entities;
    string DEFAULT_FALLBACK_INTENT = "Default Fallback Intent";

    CGSCExtractor();
    void on_extractor(parser &p);
    map<string, string> get_output(vector<IntentMapping> intent_map);
    void reset_values();

private:
    void on_COMMAND_KEYWORD(const SemanticValues &sv);
    void on_OBJECTS(const SemanticValues &sv);
    void on_TOOL_ID(const SemanticValues &sv);
    void on_PIECE_ID(const SemanticValues &sv);
    void on_MEASUREMENT_ID(const SemanticValues &sv);
    void on_single_value(const SemanticValues &sv);
    void on_min_value(const SemanticValues &sv);
    void on_max_value(const SemanticValues &sv);
    void on_UNIT(const SemanticValues &sv);
    void on_SATISFIED_KEYWORDS(const SemanticValues &sv);
};

#endif
