#ifndef SRC_DOMAIN_CONFLICT_RULE_H
#define SRC_DOMAIN_CONFLICT_RULE_H
#include <string>
#include <regex>

class ConflictRule
{
public:
    ConflictRule();
    ~ConflictRule();

    ConflictRule(const std::string name, const std::string rule, const std::string command);
    ConflictRule(const int id, const int order, const std::string name, const std::string rule, const std::string command);

    bool operator==(const ConflictRule& other) const
    {
        return name == other.name
            && id == other.id
            && order == other.order
            && rule == other.rule
            && command == other.command;
    }

    static const ConflictRule& Match(const std::string& path, const std::vector<ConflictRule>& rules);

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
