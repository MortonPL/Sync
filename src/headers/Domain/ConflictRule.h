#ifndef SRC_DOMAIN_CONFLICT_RULE_H
#define SRC_DOMAIN_CONFLICT_RULE_H
#include <string>
#include <regex>

class ConflictRule
{
public:
    ConflictRule();
    ~ConflictRule();

    ConflictRule(std::string name, std::string rule, std::string command);
    ConflictRule(int id, int order, std::string name, std::string rule, std::string command);

    bool operator==(const ConflictRule& other) const
    {
        return this->name == other.name
            && this->id == other.id
            && this->order == other.order
            && this->rule == other.rule
            && this->command == other.command;
    }

    static ConflictRule& Match(std::string path, std::vector<ConflictRule>& rules);

    int id;
    int order;
    std::string name;
    std::string rule;
    std::regex regex;
    std::string command;
    bool badRule = false;
    static ConflictRule emptyRule;
};

#endif
