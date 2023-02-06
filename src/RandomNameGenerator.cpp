#include <magic_enum/magic_enum.hpp>
#include <showlib/CommonUsing.h>
#include <showlib/FileUtilities.h>
#include <showlib/StringUtils.h>
#include <showlib/StringVector.h>

#include "RandomNameGenerator.h"

/**
 * Default constructor. You'll need to have a different way of loading the arrays.
 */
RandomNameGenerator::RandomNameGenerator() {
}

/**
 * Load from this file.
 */
RandomNameGenerator::RandomNameGenerator(const string &fileName) {
    load(fileName);
}

/**
 * Load this file.
 */
void RandomNameGenerator::load(const string &filename) {
    if (filename.empty()) {
        return;
    }
    ShowLib::StringVector lines = ShowLib::FileUtilities::readLines(filename, true);

    if (lines.empty()) {
        return;
    }

    prefixes.clear();
    middles.clear();
    suffixes.clear();

    for (const ShowLib::StringVector::Pointer & lineP: lines) {
        string line = *lineP;
        size_t pos = line.find('#');
        if (pos != string::npos) {
            line = ShowLib::trim(line.substr(0, pos));
        }
        if (line.empty()) {
            continue;
        }

        // The line has data.
        SyllableType type = SyllableType::Middle;
        bool prevVowel = false;
        bool prevConsonant = false;
        bool nextVowel = false;
        bool nextConsonant = false;

        ShowLib::StringVector parts;
        parts.tokenize(line, ' ');

        //----------------------------------------------------------------------
        // We can have special rules. Process any.
        //----------------------------------------------------------------------
        if (*parts[0] == "Rule:") {
            for (size_t index = 1; index < parts.size(); ++index) {
                ShowLib::StringVector ruleParts;

                ruleParts.tokenize(*parts[index], '=');

                string rule = ShowLib::toLower(*ruleParts[0]);
                if (rule == "hyphen-after-prefix") {
                }
                else if (rule == "accent-after-prefix") {
                }
                else if (rule == "accent-after-syllable") {
                }
                else if (rule == "diacritic-on-repeated-vowel") {
                }
            }

            continue;
        }

        //----------------------------------------------------------------------
        // Just a syllabel
        //----------------------------------------------------------------------
        for (size_t index = 1; index < parts.size(); ++index) {
            string rStr = *parts[index];

            if (rStr == "-c") {
                prevConsonant = true;
            }
            else if (rStr == "-v") {
                prevVowel = true;
            }
            else if (rStr == "+c") {
                nextConsonant = true;
            }
            else if (rStr == "+v") {
                nextVowel = true;
            }
        }

        Syllable::Pointer syllableP = std::make_shared<Syllable>(*parts[0], type, prevVowel, prevConsonant, nextVowel, nextConsonant);
        if (syllableP->getText().empty()) {
            continue;
        }

        switch (syllableP->getType()) {
            case SyllableType::Prefix: prefixes.push_back(syllableP); rulesForPrefixes.apply(*syllableP); break;
            case SyllableType::Middle: middles.push_back(syllableP);  rulesForMiddles.apply(*syllableP); break;
            case SyllableType::Suffix: suffixes.push_back(syllableP); rulesForSuffixes.apply(*syllableP); break;
        }
    }
}

/**
 * Is the data safe?
 */
bool RandomNameGenerator::validate() {
    bool retVal = true;

    if (prefixes.empty()) {
        cerr << "No prefixes defined.\n";
        retVal = false;
    }
    else {
        if ( ! middles.empty() ) {
            retVal = retVal && rulesForPrefixes.validate(rulesForMiddles);
        }
        if ( ! suffixes.empty() ) {
            retVal = retVal && rulesForPrefixes.validate(rulesForSuffixes);
        }
    }

    if ( !middles.empty() ) {
        if ( ! suffixes.empty() ) {
            retVal = retVal && rulesForMiddles.validate(rulesForSuffixes);
        }
    }

    return retVal;
}

//======================================================================
// Conversions for our enums.
//======================================================================
std::string syllableTypeToString( RandomNameGenerator::SyllableType st) {
    std::string_view rv = magic_enum::enum_name(st);
    return {rv.data(), rv.size()};
}

std::string frequencyToString( RandomNameGenerator::Frequency type) {
    std::string_view rv = magic_enum::enum_name(type);
    return {rv.data(), rv.size()};
}

RandomNameGenerator::SyllableType toSyllableType(const std::string &str) {
    RandomNameGenerator::SyllableType rv = RandomNameGenerator::SyllableType::Middle;
    auto value = magic_enum::enum_cast<RandomNameGenerator::SyllableType>(str);
    if (value.has_value()) {
        rv = value.value();
    }
    return rv;
}

RandomNameGenerator::Frequency toFrequency(const std::string &str) {
    RandomNameGenerator::Frequency rv = RandomNameGenerator::Frequency::Never;
    auto value = magic_enum::enum_cast<RandomNameGenerator::Frequency>(str);
    if (value.has_value()) {
        rv = value.value();
    }
    return rv;
}

//======================================================================
// Syllables.
//======================================================================

/**
 * Construct with a string.
 */
RandomNameGenerator::Syllable::Syllable(const std::string & str)
    : text(str)
{
}

/**
 * This form is once we're parsed.
 */
RandomNameGenerator::Syllable::Syllable(
    const std::string & str,
    SyllableType typeIn,
    bool prevVowel,
    bool prevConsonant,
    bool nextVowel,
    bool nextConsonant)
:	text(str),
    type(typeIn),
    previousMustEndInVowel(prevVowel),
    previousMustEndInConsonant(prevConsonant),
    nextMustStartWithVowel(nextVowel),
    nextMustStartWithConsonant(nextConsonant)
{
}

/**
 * Populate from JSON.
 */
void RandomNameGenerator::Syllable::fromJSON(const JSON &json) {
    text = stringValue(json, "text");
    type = toSyllableType(stringValue(json, "type"));

    previousMustEndInVowel = boolValue(json, "previousMustEndInVowel");
    previousMustEndInConsonant = boolValue(json, "previousMustEndInConsonant");
    nextMustStartWithVowel = boolValue(json, "nextMustStartWithVowel");
    nextMustStartWithConsonant = boolValue(json, "nextMustStartWithConsonant");
}

/**
 * Return our JSON representation.
 */
JSON RandomNameGenerator::Syllable::toJSON() const {
    JSON json = JSON::object();

    json["text"] = text;
    json["type"] = syllableTypeToString(type);
    json["previousMustEndInVowel"] = previousMustEndInVowel;
    json["previousMustEndInConsonant"] = previousMustEndInConsonant;
    json["nextMustStartWithVowel"] = nextMustStartWithVowel;
    json["nextMustStartWithConsonantt"] = nextMustStartWithConsonant;

    return json;
}

/**
 * Return a list of syllables that can follow this one.
 */
RandomNameGenerator::Syllable::Vector RandomNameGenerator::Syllable::makeFollowing(const RandomNameGenerator::Syllable::Vector &vec) {
    Vector retVal;
    bool vowel = endsInVowel();
    bool consonant = endsInConsonant();

    for (const Pointer & possibleSylP: vec) {
        // Eliminate based on rules.
        if (   (vowel && possibleSylP->getPreviousMustEndInConsonant())
            || (consonant && possibleSylP->getPreviousMustEndInVowel())
            || (nextMustStartWithConsonant && possibleSylP->beginsWithVowel())
            || (nextMustStartWithVowel && possibleSylP->beginsWithConsonant()) )
        {
            continue;
        }

        retVal.push_back(possibleSylP);
    }

    return retVal;
}


//======================================================================
// The Rules Engine.
//======================================================================

/**
 * Apply this syllable.
 */
void RandomNameGenerator::RuleExists::apply(const Syllable &syl) {
    //----------------------------------------------------------------------
    // Handle any rules we have for our previous syllable
    //----------------------------------------------------------------------
    if (syl.getPreviousMustEndInVowel()) {
        size_t & forPrev_ReqVowel = syl.beginsWithConsonant() ? forPrev_Consonant_ReqVowel : forPrev_Vowel_ReqVowel;
        ++forPrev_ReqVowel;
    }
    else if (syl.getPreviousMustEndInConsonant()) {
        size_t & forPrev_ReqConsonant = syl.beginsWithConsonant() ? forPrev_Consonant_ReqConsonant : forPrev_Vowel_ReqConsonant;
        ++forPrev_ReqConsonant;
    }
    else {
        size_t & forPrev_NoCare = syl.beginsWithConsonant() ? forPrev_Consonant_NoCare : forPrev_Vowel_NoCare;
        ++forPrev_NoCare;
    }

    //----------------------------------------------------------------------
    // Handle any rules for our following syllable
    //----------------------------------------------------------------------
    if (syl.getNextMustStartWithVowel()) {
        size_t & forNext_Vowel = syl.endsInConsonant() ? forNext_Consonant_ReqVowel : forNext_Vowel_ReqVowel;
        ++forNext_Vowel;
    }
    else if (syl.getNextMustStartWithConsonant()) {
        size_t & forNext_Consonant = syl.endsInConsonant() ? forNext_Consonant_ReqConsonant : forNext_Vowel_ReqConsonant;
        ++forNext_Consonant;
    }
    else {
        size_t & forNext_NoCare = syl.endsInConsonant() ? forNext_Consonant_NoCare : forNext_Vowel_NoCare;
        ++forNext_NoCare;
    }
}

/**
 * We only validate that we can find something to follow us. It may be there are following choices
 * that can't be used because the rules just don't work. We don't care.
 */
bool RandomNameGenerator::RuleExists::validate(const RandomNameGenerator::RuleExists &followingRuleSet) const {
    bool retVal = true;

    if (forNext_Consonant_ReqVowel) {
        // We accept leading vowels that either don't care or accept a preceding consonant
        int count = followingRuleSet.forPrev_Vowel_NoCare + followingRuleSet.forPrev_Vowel_ReqConsonant;

        if (count == 0) {
            cerr << "We have prefixes ending in a consonant that require a vowel, but we may have no satisfying Middles.\n";
            retVal = false;
        }
    }
    if (forNext_Consonant_ReqConsonant) {
        // We accept leading consonants that either don't care or accept a preceding consonant.
        int count = followingRuleSet.forPrev_Consonant_NoCare + followingRuleSet.forPrev_Consonant_ReqConsonant;

        if (count == 0) {
            cerr << "We have prefixes ending in a vowel that require a consonant, but we may have no satisfying Middles.\n";
            retVal = false;
        }
    }

    // And these two are for syllables ending in vowels
    if (forNext_Vowel_ReqVowel) {
    }
    if (forNext_Vowel_ReqConsonant) {
    }

    return retVal;
    }

