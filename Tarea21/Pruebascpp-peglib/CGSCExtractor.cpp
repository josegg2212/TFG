#include "CGSCExtractor.h"

using namespace std;
using namespace peg;

// Class constructor
CGSCExtractor::CGSCExtractor() {
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

// Method for functions callbacks
void CGSCExtractor::on_extractor(parser &p) {
    p["COMMAND_KEYWORD"] = [this](const SemanticValues &sv) 
    { 
        this->on_COMMAND_KEYWORD(sv); 
    };
    p["OBJECTS"] = [this](const SemanticValues &sv) 
    { 
        this->on_OBJECTS(sv); 
    };
    p["TOOL_ID"] = [this](const SemanticValues &sv) 
    {
        this->on_TOOL_ID(sv); 
    };
    p["PIECE_ID"] = [this](const SemanticValues &sv) 
    { 
        this->on_PIECE_ID(sv); 
    };
    p["MEASUREMENT_ID"] = [this](const SemanticValues &sv) 
    {
        this->on_MEASUREMENT_ID(sv); 
    };
    p["single_value"] = [this](const SemanticValues &sv) 
    { 
        this->on_single_value(sv); 
    };
    p["min_value"] = [this](const SemanticValues &sv) 
    { 
        this->on_min_value(sv); 
    };
    p["max_value"] = [this](const SemanticValues &sv) 
    { 
        this->on_max_value(sv);
    };
    p["VALUE"] = [this](const SemanticValues &sv)
    {
        return sv.token_to_string();
    };
    p["UNIT"] = [this](const SemanticValues &sv) 
    { 
        this->on_UNIT(sv); 
    };
    p["SATISFIED_KEYWORDS"] = [this](const SemanticValues &sv) 
    { 
        this->on_SATISFIED_KEYWORDS(sv); 
    };
}

// Method to get output
map<string, string> CGSCExtractor::get_output(vector<IntentMapping> intent_map) {
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
void CGSCExtractor::reset_values() 
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

// Transformation methods for each parsing rule
void CGSCExtractor::on_COMMAND_KEYWORD(const SemanticValues &sv)
{
    keywords[0] = sv.token(); 
}

void CGSCExtractor::on_OBJECTS(const SemanticValues &sv)
{
    keywords[1] = sv.token(); 
}
void CGSCExtractor::on_TOOL_ID(const SemanticValues &sv)
{
    entities["tool_code"] = sv.token(); 
}

void CGSCExtractor::on_PIECE_ID(const SemanticValues &sv)
{
    entities["piece_number"] = sv.token(); 
}

void CGSCExtractor::on_MEASUREMENT_ID(const SemanticValues &sv)
{
    entities["measure_name"] = sv.token(); 
}

void CGSCExtractor::on_single_value(const SemanticValues &sv)
{
    entities["value"] = any_cast<string>(sv[0]); 
}

void CGSCExtractor::on_min_value(const SemanticValues &sv)
{
    entities["range_min"] = any_cast<string>(sv[0]);
}

void CGSCExtractor::on_max_value(const SemanticValues &sv)
{
    entities["range_max"] = any_cast<string>(sv[0]);
}

void CGSCExtractor::on_UNIT(const SemanticValues &sv)
{
    entities["unit_size"] = sv.token(); 
}

void CGSCExtractor::on_SATISFIED_KEYWORDS(const SemanticValues &sv)
{
        entities["satisfied"] = sv.token(); 
}
