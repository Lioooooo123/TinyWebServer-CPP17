#include "config.h"

#include <getopt.h>

#include <iostream>
#include <exception>
#include <fstream>

namespace
{
int parse_or_default(const char *arg, int fallback, char flag)
{
    if (arg == nullptr)
    {
        return fallback;
    }

    try
    {
        return std::stoi(arg);
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Config] 无效的参数值: -" << flag << "=" << arg
                  << ", 使用默认值 " << fallback << std::endl;
        return fallback;
    }
}

std::string trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}
} // namespace

Config::Config()
    : PORT(9006),
      LOGWrite(0),
      TRIGMode(0),
      LISTENTrigmode(0),
      CONNTrigmode(0),
      OPT_LINGER(0),
      sql_num(8),
      thread_num(8),
      close_log(0),
      actor_model(0)
{
}

void Config::parse_arg(int argc, char *argv[])
{
    int opt = 0;
    constexpr const char *kOptString = "p:l:m:o:s:t:c:a:f:";

    while ((opt = getopt(argc, argv, kOptString)) != -1)
    {
        switch (opt)
        {
        case 'p':
            PORT = parse_or_default(optarg, PORT, 'p');
            break;
        case 'l':
            LOGWrite = parse_or_default(optarg, LOGWrite, 'l');
            break;
        case 'm':
            TRIGMode = parse_or_default(optarg, TRIGMode, 'm');
            break;
        case 'o':
            OPT_LINGER = parse_or_default(optarg, OPT_LINGER, 'o');
            break;
        case 's':
            sql_num = parse_or_default(optarg, sql_num, 's');
            break;
        case 't':
            thread_num = parse_or_default(optarg, thread_num, 't');
            break;
        case 'c':
            close_log = parse_or_default(optarg, close_log, 'c');
            break;
        case 'a':
            actor_model = parse_or_default(optarg, actor_model, 'a');
            break;
        case 'f':
            if (optarg)
            {
                load_from_file(optarg);
            }
            break;
        default:
            break;
        }
    }
}

bool Config::load_from_file(const std::string& config_file)
{
    std::ifstream infile(config_file);
    if (!infile.is_open())
    {
        std::cerr << "[Config] 无法打开配置文件: " << config_file << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(infile, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        size_t eq_pos = line.find('=');
        if (eq_pos == std::string::npos)
            continue;

        std::string key = trim(line.substr(0, eq_pos));
        std::string value = trim(line.substr(eq_pos + 1));

        try
        {
            if (key == "PORT")
                PORT = std::stoi(value);
            else if (key == "LOGWrite")
                LOGWrite = std::stoi(value);
            else if (key == "TRIGMode")
                TRIGMode = std::stoi(value);
            else if (key == "LISTENTrigmode")
                LISTENTrigmode = std::stoi(value);
            else if (key == "CONNTrigmode")
                CONNTrigmode = std::stoi(value);
            else if (key == "OPT_LINGER")
                OPT_LINGER = std::stoi(value);
            else if (key == "sql_num")
                sql_num = std::stoi(value);
            else if (key == "thread_num")
                thread_num = std::stoi(value);
            else if (key == "close_log")
                close_log = std::stoi(value);
            else if (key == "actor_model")
                actor_model = std::stoi(value);
        }
        catch (const std::exception &e)
        {
            std::cerr << "[Config] 无法解析配置项: " << key << "=" << value << std::endl;
        }
    }

    std::cout << "[Config] 成功从文件加载配置: " << config_file << std::endl;
    return true;
}
