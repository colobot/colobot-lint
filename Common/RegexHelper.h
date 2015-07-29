#pragma once

#include <llvm/ADT/StringRef.h>

#include <boost/regex.hpp>

using StringRefMatchResults = boost::match_results<llvm::StringRef::const_iterator>;

inline llvm::StringRef GetStringRefResult(const StringRefMatchResults& results,
                                          int number,
                                          llvm::StringRef text)
{
    return text.substr(results.position(number), results.length(number));
}

const char* const LOWER_CAMEL_CASE_PATTERN = "[[:lower:][:digit:]]+([[:upper:][:digit:]]+[[:lower:][:digit:]]*)*";
const char* const UPPER_CAMEL_CASE_PATTERN = "[[:upper:][:digit:]][[:lower:][:digit:]]*([[:upper:][:digit:]]+[[:lower:][:digit:]]*)*";
const char* const ALL_CAPS_UNDERSCORE_PATTERN = "[[:upper:][:digit:]]+(_[[:upper:][:digit:]]+)*";
