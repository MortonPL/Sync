#include "Domain/ConflictRule.h"

#include "Utils.h"

ConflictRule ConflictRule::emptyRule;

ConflictRule::ConflictRule()
{
}

ConflictRule::~ConflictRule()
{
}

ConflictRule::ConflictRule(std::string name, std::string rule, std::string command)
{
    this->name = name;
    this->rule = rule;
    this->command = command;
    try
    {
        Utils::Replace(rule, ".", "\\.");
        Utils::Replace(rule, "*", ".*");
        this->regex = std::regex(rule);
    }
    catch(const std::exception& e)
    {
        this->badRule = true;
    }
}

ConflictRule::ConflictRule(int id, int order, std::string name, std::string rule, std::string command)
{
    this->id = id;
    this->order = order;
    this->name = name;
    this->rule = rule;
    this->command = command;
    try
    {
        Utils::Replace(rule, ".", "\\.");
        Utils::Replace(rule, "*", ".*");
        this->regex = std::regex(rule);
    }
    catch(const std::exception& e)
    {
        this->badRule = true;
    }
}

ConflictRule& ConflictRule::Match(std::string path, std::vector<ConflictRule>& rules)
{
    for (auto& rule: rules)
    {
        if (rule.badRule)
            continue;
        
        if (std::regex_match(path, rule.regex))
            return rule;
    }

    return emptyRule;
}
