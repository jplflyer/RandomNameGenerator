//
// A command line version of the Random Name Generator.
//
// Possible actions:
//
//		Parse and validate an input file
//		Parse and validate an input file then dump it in JSON (for testing purposes)
//		Parse an input file and produce a C++ class from it
//		Generate names
//
#include <showlib/CommonUsing.h>
#include <showlib/OptionHandler.h>

#include "RandomNameGenerator.h"

enum class Command {
    Generate,
    Validate,
    JSON,
    CPP_Class
};

/**
 * Entry point.
 */
int main(int argc, char **argv) {
    ShowLib::OptionHandler::ArgumentVector args;
    string filename;
    string outputFileName;
    Command command = Command::Generate;
    int count = 1;

    args.addArg("file",   [&](const char *value) { filename = value; }, "file.txt", "Specify an input file");
    args.addArg("output", [&](const char *value) { outputFileName = value; }, "file.txt", "Specify an output file");

    args.addNoArg("validate", [&](const char *) { command = Command::Validate; }, "Validate input" );

    args.addNoArg("generate", [&](const char *) { command = Command::Generate; }, "Generate names (the default)" );
    args.addArg("count", 'n', [&](const char *value) { count = atoi(value); }, std::to_string(count), "Number of names to generate");

    args.addNoArg("json", [&](const char *) { command = Command::JSON; },      "Output the rules as a JSON file" );
    args.addNoArg("c++",  [&](const char *) { command = Command::CPP_Class; }, "Output a C++ class" );

    if (!ShowLib::OptionHandler::handleOptions(argc, argv, args)) {
        exit(1);
    }

    if (filename.empty()) {
        cerr << "--file filename is required\n";
        exit(1);
    }

    RNG::RandomNameGenerator gen(filename);
    if (!gen.validate()) {
        exit(1);
    }
}
