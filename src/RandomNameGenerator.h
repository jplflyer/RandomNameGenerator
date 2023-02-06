#pragma once

#include <string>

#include <showlib/JSONSerializable.h>

/**
 * This serves as a random name generator. It is based on the input file you give,
 * or you can subclass and provide input as C++ arrays.
 *
 * There are three arrays of syllables:
 * 		Prefixes
 * 		Middles
 * 		Suffixes
 *
 * Input files consist of lines according to these rules:
 *
 * 		The line is trimmed at the first # character
 * 		Leading and trailing whitespace is trimmed
 * 		A line beginning with - is a prefix
 * 		A line beginning with + is a suffix
 * 		Otherwise the line represents a middle syllable
 *
 * Rules follow the first non-whitespace text and are as follows:
 *
 * 		-v Previous syllabel must end in a vowel
 * 		-c Previous syllabel must end in a consonant
 * 		+v Next syllabel must start with a vowel
 * 		+c Next syllabel must start with a consanant
 *
 * To use:
 *
 * 		RandomNameGenerator rng(inputFileName);
 *		rng.validate();
 *		std::string newName = rng.compose(numberOfSyllables);
 *
 * The validate() method verifies the input file cannot generate problems. Basically, it
 * verifies that the available choices and the various rules do not lead to impossible
 * situations, such as requiring a preceding syllable ending in a consonant, but there
 * aren't any.
 */
class RandomNameGenerator
{
public:
    enum class SyllableType { Prefix, Middle, Suffix };
    enum class Frequency { Always, Sometimes, Never };

    class Syllable: public ShowLib::JSONSerializable {
    public:
        typedef std::shared_ptr<Syllable> Pointer;
        typedef ShowLib::JSONSerializableVector<Syllable> Vector;

        Syllable() = default;
        Syllable(const std::string & str);
        Syllable(const std::string & str, SyllableType, bool prevVowel, bool preConsonant, bool nextVowel, bool nextConsonant);

        void fromJSON(const JSON &) override;
        JSON toJSON() const override;

        bool endsInVowel() const;
        bool endsInConsonant() const;
        bool beginsWithVowel() const;
        bool beginsWithConsonant() const;

        const std::string & getText()        const { return text; }
        SyllableType getType()               const { return type; }
        bool getPreviousMustEndInVowel()     const { return previousMustEndInVowel; }
        bool getPreviousMustEndInConsonant() const { return previousMustEndInConsonant; }
        bool getNextMustStartWithVowel()     const { return nextMustStartWithVowel; }
        bool getNextMustStartWithConsonant() const { return nextMustStartWithConsonant; }

        Vector makeFollowing(const Vector &);

    protected:
        // Fields
        std::string		text;
        SyllableType	type = SyllableType::Middle;

        // Rules. Each pair is mutually exclusive. You can not
        // have both previousMustEndInVowel and previousMustEndInConsonant.
        bool previousMustEndInVowel = false;
        bool previousMustEndInConsonant = false;
        bool nextMustStartWithVowel = false;
        bool nextMustStartWithConsonant = false;
    };

    /**
     * Our rules are complicated. There is a relationship between the trailing
     * character of one syllable and the leading character of the next syllable.
     * Either can be a consonant or a vowel, and either could have a rule about
     * the other. Thus, we have two sets of rules, one for the relationship to our
     * previous syllable and one for the relationship to the next one.
     */
    class RuleExists {
    public:
        void apply(const Syllable &);

        bool validate(const RuleExists &followingRuleSet) const;

        // For the relationship between us and the preceeding syllable.
        size_t forPrev_Consonant_NoCare = 0;
        size_t forPrev_Vowel_NoCare = 0;

        size_t forPrev_Consonant_ReqVowel = 0;
        size_t forPrev_Consonant_ReqConsonant = 0;

        size_t forPrev_Vowel_ReqVowel = 0;
        size_t forPrev_Vowel_ReqConsonant = 0;

        // For the relationship between us and the next syllable.
        size_t forNext_Consonant_NoCare = 0;
        size_t forNext_Vowel_NoCare = 0;

        size_t forNext_Consonant_ReqVowel = 0;
        size_t forNext_Consonant_ReqConsonant = 0;
        size_t forNext_Vowel_ReqVowel = 0;
        size_t forNext_Vowel_ReqConsonant = 0;
    };

    RandomNameGenerator();
    RandomNameGenerator(const std::string & filename);

    void load(const std::string &filename);

    bool validate();
    std::string compose(int numberOfSyllables);

protected:
    Syllable::Vector prefixes;
    Syllable::Vector middles;
    Syllable::Vector suffixes;

    RuleExists rulesForPrefixes;
    RuleExists rulesForMiddles;
    RuleExists rulesForSuffixes;

    Frequency hyphenAfterPrefix = Frequency::Never;
    Frequency accentAfterPrefix = Frequency::Never;
    Frequency accentAfterSyllable = Frequency::Never;
    Frequency diacriticOnRepeatedVowel = Frequency::Never;
};

std::string syllableTypeToString( RandomNameGenerator::SyllableType );
std::string frequencyToString( RandomNameGenerator::Frequency );

RandomNameGenerator::SyllableType toSyllableType(const std::string &);
RandomNameGenerator::Frequency toFrequency(const std::string &);
