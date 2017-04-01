

#ifndef UTIL_MINI_HPP
#define UTIL_MINI_HPP

#include <cstring>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <cassert>
#include <exception>
#include <fstream>
#include "Util/util.h"
using namespace std;

namespace ZL {
namespace Util {


template<typename key, typename variant>
class mINI_basic: public map<key, variant> {
	// Public API : existing map<> interface plus following methods
public:
	void parse(const string &text) {
		// reset, split lines and parse
		static auto trim = []( string line ) {
			while( line.size() && ( line.back()=='\t' || line.back()==' ' ) ) line.pop_back();
			while( line.size() && ( line.front()=='\t' || line.front()==' ' ) ) line.erase(0,1);
			return line;
		};
		vector<string> lines = tokenize(text, "\r\n");
		string symbol, tag;
		for (auto &line : lines){
			// trim blanks
			line = trim(line);
			// split line into tokens and parse tokens
			if(line.empty() || line.front() == ';' || line.front() == '#'){
				continue;
			}
			if (line.size() >= 3 && line.front() == '[' && line.back() == ']') {
				tag = trim(line.substr(1, line.size() - 2));
			} else {
				auto at = line.find_first_of('=');
				symbol = trim(tag + "." + line.substr(0, at));
				(*this)[symbol] = (at == string::npos ? string() : trim(line.substr(at + 1)));
			}
		}
	}
	void parseFile(const string &fileName = exePath() + ".ini") {
		ifstream in(fileName, ios::in | ios::binary | ios::ate);
		if (!in.good()) {
			stringstream ss;
			ss << "invalid ini file:" << fileName;
			throw invalid_argument(ss.str());
		}
		long size = in.tellg();
		in.seekg(0, ios::beg);
		string buf;
		buf.resize(size);
		in.read((char *) buf.data(), size);
		parse(buf);
	}

	string dump(const string &header = "; auto-generated by mINI class {",
			const string &footer = "; } ---") const {
		string output(header + (header.empty() ? "" : "\r\n")), tag;
		for (auto &pr : *this) {
			vector<string> kv = tokenize(pr.first, ".");
			if (tag != kv[0]) {
				output += "\r\n[" + (tag = kv[0]) + "]\r\n";
			}
			output += kv[1] + "=" + pr.second + "\r\n";
		}
		return output + "\r\n" + footer + (footer.empty() ? "" : "\r\n");
	}

	void dumpFile(const string &fileName = exePath() + ".ini") {
		ofstream out(fileName, ios::out | ios::binary | ios::trunc);
		auto dmp = dump();
		out.write(dmp.data(),dmp.size());
	}
	static mINI_basic &Instance(){
		static mINI_basic instance;
		return instance;
	}
private:
	vector<string> tokenize(const string &self, const string &chars) const {
		vector<string> tokens(1);
		string map(256, '\0');
		for (const unsigned char &ch : chars) {
			map[ch] = '\1';
		}
		for (const unsigned char &ch : self) {
			if (!map.at(ch)) {
				tokens.back().push_back(char(ch));
			} else if (tokens.back().size()) {
				tokens.push_back(string());
			}
		}
		while (tokens.size() && tokens.back().empty()) {
			tokens.pop_back();
		}
		return tokens;
	}
};

//  handy variant class as key/values
struct variant: public string {
	template<typename T>
	variant(const T &t) :
	string(to_string(t)) {
	}
	template<size_t N>
	variant(const char (&s)[N]) :
	string(s, N) {
	}
	variant(const char *cstr) :
		string(cstr) {
	}
	variant(const string &other = string()) :
		string(other) {
	}
	template<typename T>
	operator T() const {
		T t;
		stringstream ss;
		return ss << *this && ss >> t ? t : T();
	}
	template<typename T> bool operator ==(const T &t) const {
		return 0 == this->compare(variant(t));
	}
	bool operator ==(const char *t) const {
		return this->compare(t) == 0;
	}
	template<typename T>
	T as() const {
		return (T) (*this);
	}
};

using mINI = mINI_basic<string, variant>;

}  // namespace Util
}  // namespace ZL

#endif //UTIL_MINI_HPP

