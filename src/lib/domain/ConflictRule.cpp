#include "Domain/ConflictRule.h"

#include "Utils.h"

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
