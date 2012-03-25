#ifndef MOUS_OPTION_H
#define MOUS_OPTION_H

#include <inttypes.h>
#include <string>
#include <vector>
#include <utility>

namespace mous {

namespace OptionType {
enum e {
    None = 0,

    Int,            // int32_t
    Float,          // double
    String,

    EnumedInt,
    EnumedFloat,
    EnumedString,

    RangedInt,
    RangedFloat
};
}
typedef OptionType::e EmOptionType;

inline const char* ToString(EmOptionType type)
{
    switch (type) {
        case OptionType::None:
            return "None";

        case OptionType::Int:
            return "Int";

        case OptionType::Float:
            return "Float";

        case OptionType::String:
            return "String";

        case OptionType::EnumedInt:
            return "EnumedInt";

        case OptionType::EnumedFloat:
            return "EnumedFloat";

        case OptionType::EnumedString:
            return "EnumedString";

        case OptionType::RangedInt:
            return "RangedInt";

        case OptionType::RangedFloat:
            return "RangedFloat";
    }
    return "";
}

struct CommonOption
{
    std::string desc;
};

struct IntOption
{
    CommonOption common;
    int32_t defaultVal;
    mutable int32_t userVal; 
};

struct FloatOption
{
    CommonOption common;
    double defaultVal;
    mutable double userVal; 
};

struct StringOption
{
    CommonOption common;
    std::string defaultVal;
    mutable std::string userVal; 
};

struct EnumedIntOption
{
    CommonOption common;
    std::vector<int32_t> enumedVal;
    size_t defaultChoice;
    mutable size_t userChoice; 
};

struct EnumedFloatOption
{
    CommonOption common;
    std::vector<double> enumedtVal;
    size_t defaultChoice;
    mutable size_t userChoice; 
};

struct EnumedStringOption
{
    CommonOption common;
    std::vector<std::string> enumedVal;
    size_t defaultChoice;
    mutable size_t userChoice; 
};

struct RangedIntOption
{
    CommonOption common;
    int32_t min;
    int32_t max;
    int32_t defaultVal;
    mutable int32_t userVal; 
};

struct RangedFloatOption
{
    CommonOption common;
    double min;
    double max;
    double defaultVal;
    mutable double userVal; 
};

typedef std::pair<const void*, EmOptionType> ConstOptionPair;

}

#endif
