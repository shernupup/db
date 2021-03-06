#include<IO/ReadHelper.h>
#include<Poco/Exception.h>
#include<CommonUtil/FindSymbol.h>
#include<Common/hex.h>

template <typename Vector>
static void parseComplexEscapeSequence(Vector & s, IO::ReadBuffer & buf)
{
    ++buf.position();
    if (buf.eof())
        throw Poco::Exception("Cannot parse escape sequence");

    if (*buf.position() == 'x')
    {
        ++buf.position();
        /// escape sequence of the form \xAA
        char hex_code[2];
        IO::readPODBinary(hex_code, buf);
        s.push_back( unhex2(hex_code) );
    }
    else if (*buf.position() == 'N')
    {
        /// Support for NULLs: \N sequence must be parsed as empty string.
        ++buf.position();
    }
    else
    {
        /// The usual escape sequence of a single character.
        s.push_back(IO::parseEscapeSequence(*buf.position()));
        ++buf.position();
    }
}

template <char quote, bool enable_sql_style_quoting, typename Vector>
static void readAnyQuotedStringInto(Vector & s, IO::ReadBuffer & buf)
{
    if (buf.eof() || *buf.position() != quote)
        throw Poco::Exception("Cannot parse quoted string: expected opening quote");
    ++buf.position();
    while (!buf.eof())
    {
        const char * next_pos = find_first_symbols<'\\', quote>(buf.position(), buf.buffer().end());
        IO::appendToStringOrVector(s, buf.position(), next_pos);
        buf.position() += next_pos - buf.position();
        if (!buf.hasPendingData())
            continue;

        if (*buf.position() == quote)
        {
            ++buf.position();

            if (enable_sql_style_quoting && !buf.eof() && *buf.position() == quote)
            {
                s.push_back(quote);
                ++buf.position();
                continue;
            }
            return;
        }

        if (*buf.position() == '\\')
            parseComplexEscapeSequence(s, buf);
    }

    throw Poco::Exception("Cannot parse quoted string: expected closing quote");
}




void IO::readBackQuotedStringWithSQLStyle(std::string& s, IO::ReadBuffer& buf)
{
    s.clear();
    readBackQuotedStringInto<true,std::string>(s, buf);
}


template <bool enable_sql_style_quoting, typename Vector>
void IO::readQuotedStringInto(Vector & s, IO::ReadBuffer & buf)
{
    readAnyQuotedStringInto<'\'', enable_sql_style_quoting>(s, buf);
}

template <bool enable_sql_style_quoting, typename Vector>
void IO::readBackQuotedStringInto(Vector & s, IO::ReadBuffer & buf)
{
    readAnyQuotedStringInto<'`', enable_sql_style_quoting,Vector>(s, buf);
}

void IO::readStringUntilEOF(std::string& s, IO::ReadBuffer& buf)
{
    s.clear();
    readStringUntilEOFInto(s, buf);
}


void IO::readDoubleQuotedStringWithSQLStyle(std::string & s, IO::ReadBuffer & buf)
{
    s.clear();
    IO::readDoubleQuotedStringInto<true>(s, buf);
}

template <bool enable_sql_style_quoting, typename Vector>
void IO::readDoubleQuotedStringInto(Vector & s, IO::ReadBuffer & buf)
{
    readAnyQuotedStringInto<'"', enable_sql_style_quoting>(s, buf);
}
