// Libraries
#include "extractor.h"



// Namespaces
using namespace std;
using namespace peg;



// Function to convert a string to lowercase
string toLower(string str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}



// Class constructor with initializations of keywords and entities, and functions callbacks
extractor::extractor(parser &p, const vector<RuleConfig> &rule_config) {

    // Initialize keywords and entities
    keywords.resize(2, "");
    for (const auto &rc : rule_config) {
        if (rc.type == "entity" || rc.type == "node_entity") {
            entities[toLower(rc.rule_name)] = "";
        }
    }

    // Function callbacks
    int keyword_index = 0;

    for (const auto &rc : rule_config) {
        if (rc.type == "keyword") {
            p[rc.rule_name.c_str()] = [this, keyword_index](const SemanticValues &sv) {
                this->register_keyword(sv,keyword_index);
            };
            keyword_index++;
          

        } else if (rc.type == "token_to_string") {
            p[rc.rule_name.c_str()] = [](const SemanticValues &sv) {
                return sv.token_to_string();
            };

        } else if (rc.type == "entity") {
            p[rc.rule_name.c_str()] = [this, ruleName=toLower(rc.rule_name)](const SemanticValues &sv) {
                this->register_entity(sv, ruleName);
            };

        } else if (rc.type == "node_entity") {
            p[rc.rule_name.c_str()] = [this, ruleName=rc.rule_name](const SemanticValues &sv) {
                this->register_node_entity(sv, ruleName);
            };

        } else {
            std::cerr << "Tipo desconocido en rule_name: " 
                      << rc.rule_name << " -> " << rc.type << std::endl;
        }
    }
}




// Method to get output
map<string, string> extractor::get_output(vector<IntentMapping> intent_map) {
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
void extractor::reset_values() 
{
    fill(keywords.begin(), keywords.end(), "");
    for (auto& pair : entities) {
        pair.second = "";  
    }
}



// Transformation methods for each parsing rule
void extractor::register_keyword(const SemanticValues &sv, int keyword_index)
{
    keywords[keyword_index] = sv.token(); 
}

void extractor::register_entity(const SemanticValues &sv, const string &rule_name)
{
    entities[rule_name] = sv.token();
}
void extractor::register_node_entity(const SemanticValues &sv, const string &rule_name)
{
    entities[rule_name] = any_cast<string>(sv[0]);
}

