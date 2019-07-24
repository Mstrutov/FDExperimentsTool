#include "CSVParser.h"
#include <cassert>
#include <fstream>
#include <string>
#include <experimental/filesystem>
#include <vector>

using namespace std;

CSVParser::CSVParser(fs::path &path): CSVParser(path, ',') {}

CSVParser::CSVParser(fs::path& path, char separator) : source(path), separator(separator), hasNext(true), nextLine() {
    // TODO: Настроить Exception
    if (separator == '\0'){
        assert(0);
    }
    getNext();
}

/*
bool CSVParser::isSameChar(char separator, char escape) {
    return separator != '\0' && separator == escape;
}
*/

void CSVParser::getNext(){
    nextLine = "";
    getline(source, nextLine);
    if (nextLine.empty()){
        hasNext = false;
    }
}

vector<string> CSVParser::parseNext() {
    vector<string> result = vector<string>();
    if (!hasNext){
        return result;
    }
    auto nextTokenBegin = nextLine.begin();
    auto nextTokenEnd = nextLine.begin();
    while (nextTokenEnd != nextLine.end()){
        if (*nextTokenEnd == separator){
            result.emplace_back(nextTokenBegin, nextTokenEnd);
            nextTokenBegin = nextTokenEnd + 1;
            nextTokenEnd = nextTokenBegin;
        } else {
            nextTokenEnd++;
        }
    }
    result.emplace_back(nextTokenBegin, nextTokenEnd);
    getNext();
    return result;
}

bool CSVParser::getHasNext() { return hasNext;}
char CSVParser::getSeparator() { return separator;}