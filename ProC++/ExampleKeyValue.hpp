#include <iostream>
#include <string>
#include <string_view>
#include <format>

using namespace std;

class KeyValue
{
public:
	KeyValue(string_view key, int value) : m_key{ key }, m_value{ value } {}

	const string& getKey() const { return m_key; }
	int getValue() const { return m_value; }

private:
	string m_key;
	int m_value;
};

namespace std {
    template<>
    class formatter<KeyValue>
    {
    public:
        constexpr auto parse(auto& context)
        {
            auto iter{ context.begin() };
            const auto end{ context.end() };
            if (iter == end || *iter == '}') {  // {} format specifier
                m_outputType = OutputType::KeyAndValue;
                return iter;
            }
            
            switch (*iter) {
                case 'a':  // {:a} format specifier
                    m_outputType = OutputType::KeyOnly;
                    break;
                case 'b':  // {:b} format specifier
                    m_outputType = OutputType::ValueOnly;
                    break;
                case 'c':  // {:c} format specifier
                    m_outputType = OutputType::KeyAndValue;
                    break;
                default:
                    m_outputType = OutputType::Invalid;
                    break;
                    //throw format_error{ "Invalid KeyValue format specifier." };
            }

            ++iter;
            if (iter != end && *iter != '}') {
                m_outputType = OutputType::Invalid;
                //throw format_error{ "Invalid KeyValue format specifier." };
            }

            return iter;
        }

        auto format(const KeyValue& kv, auto& context) const
        {
            if (m_outputType == OutputType::Invalid) {
                throw format_error{ "Invalid KeyValue format specifier." };
                //return format_to(context.out(), "Invalid format specifier");
            }
            switch (m_outputType) {
                using enum OutputType;

                case KeyOnly:
                    return format_to(context.out(), "{}", kv.getKey());
                case ValueOnly:
                    return format_to(context.out(), "{}", kv.getValue());
                default:
                    return format_to(context.out(), "{} - {}", kv.getKey(), kv.getValue());
            }
        }

    private:
        enum class OutputType
        {
            KeyOnly,
            ValueOnly,
            KeyAndValue,
            Invalid
        };

        OutputType m_outputType{ OutputType::KeyAndValue };
    };
}
