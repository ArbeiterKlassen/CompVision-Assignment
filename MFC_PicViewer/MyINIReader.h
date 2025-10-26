#pragma once
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <sstream>

class INIParser {
public:
	bool readFile(const std::string& filename) {
		std::ifstream file(filename);
		if (!file.is_open()) {
			std::cerr << "无法打开文件: " << filename << std::endl;
			return false;
		}
		std::string line, section;
		while (std::getline(file, line)) {
			trim(line);
			if (line.empty() || line[0] == ';') continue; // 跳过空行和注释
			if (line[0] == '[' && line.back() == ']') {
				section = line.substr(1, line.size() - 2);
				config[section] = {};
			}
			else if (line.find('=') != std::string::npos) {
				size_t pos = line.find('=');
				std::string key = trimCopy(line.substr(0, pos));
				std::string value = trimCopy(line.substr(pos + 1));
				config[section][key] = value;
			}
		}
		return true;
	}

	std::string getValue(const std::string& section, const std::string& key) {
		return config[section][key];
	}
	std::map<std::string, std::map<std::string, std::string>> getAll() {
		return config;
	}

private:
	void trim(std::string& str) {
		if (str.size() >= 3 &&
			(unsigned char)str[0] == 0xEF &&
			(unsigned char)str[1] == 0xBB &&
			(unsigned char)str[2] == 0xBF)
			str.erase(0, 3);
		const char* ws = " \t\r\n\v\f";   // C  locale 空白
		str.erase(0, str.find_first_not_of(ws));
		str.erase(str.find_last_not_of(ws) + 1);
	}
	std::string trimCopy(const std::string& str) {
		std::string copy = str;
		trim(copy);
		return copy;
	}

	std::map<std::string, std::map<std::string, std::string>> config;
};