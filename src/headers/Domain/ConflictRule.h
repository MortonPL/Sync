#pragma once
#include <string>
#include <regex>

class ConflictRule
{
public:
    ConflictRule();
    ~ConflictRule();

    ConflictRule(std::string name, std::string rule, std::string command);
    ConflictRule(int id, int order, std::string name, std::string rule, std::string command);

    int id;
    int order;
    std::string name;
    std::string rule;
    std::regex regex;
    std::string command;
    bool badRule = false;
};
