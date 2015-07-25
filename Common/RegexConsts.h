#pragma once

const char* const LOWER_CAMEL_CASE_STRING = "[[:lower:][:digit:]]+([[:upper:][:digit:]]+[[:lower:][:digit:]]*)*";
const char* const UPPER_CAMEL_CASE_STRING = "[[:upper:][:digit:]][[:lower:][:digit:]]*([[:upper:][:digit:]]+[[:lower:][:digit:]]*)*";
const char* const ALL_CAPS_UNDERSCORE_STRING = "[[:upper:][:digit:]]+(_[[:upper:][:digit:]]+)*";
