#include "Domain/ConflictRule.h"

#include "Utils.h"

ConflictRule ConflictRule::emptyRule;

ConflictRule::ConflictRule()
{
}

ConflictRule::~ConflictRule()
{
}

ConflictRule::ConflictRule(const std::string name, const std::string rule, const std::string command)
{
    this->name = name;
    this->rule = rule;
    this->command = command;
    try
    {
        Utils::Replace(this->rule, ".", "\\.");
        Utils::Replace(this->rule, "*", ".*");
        regex = std::regex(this->rule);
    }
    catch(const std::exception& e)
    {
        badRule = true;
    }
}

ConflictRule::ConflictRule(const int id, const int order, const std::string name, const std::string rule, const std::string command)
{
    this->id = id;
    this->order = order;
    this->name = name;
    this->rule = rule;
    this->command = command;
    try
    {
        Utils::Replace(this->rule, ".", "\\.");
        Utils::Replace(this->rule, "*", ".*");
        regex = std::regex(this->rule);
    }
    catch(const std::exception& e)
    {
        badRule = true;
    }
}

const ConflictRule& ConflictRule::Match(const std::string& path, const std::vector<ConflictRule>& rules)
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
