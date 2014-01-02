//  JSON.hpp :: JSON reader
//  Copyright 2013 Keigen Shu

#ifndef JSON_H
#define JSON_H

#include <map>
#include <ClanLib/core.h>
#include <ClanLib/display.h>

/**
 * Use value from resource <<FNC(RES,VAL)>> if that value
 * PASSES the supplied condition (CMP).
 *
 * @param FNC function to call
 * @param RES resource to read from XML
 * @param CMP comparison
 * @param VAL default to use resource does not exist or if condition FAILS.
 */
#define TAKE_IF_TRUE(FUNCTION,ARG,COMPARE,DEFAULT) \
    [=]() { \
        try { \
            auto VALUE = FUNCTION(ARG); \
            return (VALUE COMPARE) ? VALUE : DEFAULT; \
        } catch (std::exception &e) { /* Can't get the value :/ */ \
            clan::Console::write_line(e.what()); \
            return DEFAULT; \
        } \
    } () /* Call the lambda now */

/**
 * Use value from resource <<FNC(RES,VAL)>> if that value
 * FAILS the supplied condition (CMP).
 *
 * Example  : TAKE_IF_FALSE(conf.getInteger,"video/resh",< 600,600)
 * Expanded : conf.getInteger("video/resh",600) < 600 ? 600 : conf.getInteger("video/resh",600)
 *
 * @param FNC function to call (Config::getSomething)
 * @param RES resource to read from XML ("dir/val")
 * @param CMP comparison operator ("== false", "< SomeValue", ">= 128")
 * @param VAL default to use resource does not exist or if condition PASSES.
 */
#define TAKE_IF_FALSE(FUNCTION,ARG,COMPARE,DEFAULT) \
    [=]() { \
        try { \
            auto VALUE = FUNCTION(ARG); \
            return (VALUE COMPARE) ? DEFAULT : VALUE; \
        } catch (std::exception &e) { /* Can't get the value :/ */ \
            clan::Console::write_line(e.what()); \
            return DEFAULT; \
        } \
    } ()


class JSONReader
{
protected:
    clan::JsonValue   mRoot;

    static clan::JsonValue       & getJsonValue(std::string const &path, clan::JsonValue       & root);
    static clan::JsonValue const & getJsonValue(std::string const &path, clan::JsonValue const & root);

public:
    JSONReader();
    JSONReader(clan::JsonValue const &root);

    inline clan::JsonValue& getRoot() { return mRoot; }

    /** Loads a color object from the JSON file.
     *
     *  Color objects can be written in four (or eleven) forms:
     *
     *   1. a text string that matches any of ClanLib's predefined
     *      colors OR a hex value string of the color,
     *
     *          { "full-white": "white" }
     *          { "full-white": "#FFFFFF" }
     *          { "half-white": "#FFFFFF7F" }
     *
     *   2. a JSON array containing an element matching the above,
     *
     *          { "full-white": [ "white" ] }
     *          { "full-white": [ "#FFFFFF" ] }
     *          { "half-white": [ "#FFFFFF7F" ] }
     *
     *   3. a JSON array like #2 but with an additional number within
     *      the range of 0 .. 255 OVERRIDING its alpha value,
     *
     *          { "full-white": [ "white", 255 ] }
     *          { "half-white": [ "white", 127 ] }
     *          { "full-white": [ "#FFFFFF", 255 ] }
     *          { "half-white": [ "#FFFFFF", 127 ] }
     *          { "full-white": [ "#FFFFFF7F", 255 ] }
     *          { "half-white": [ "#FFFFFF7F", 127 ] }
     *
     *   4. or a JSON array containing 3 (for RGB) OR 4 (for RGBA) numbers
     *      within the range of 0 .. 255.
     *
     *          { "full-white": [ 255, 255, 255 ] }
     *          { "half-white": [ 255, 255, 255, 127 ] }
     *
     */
    clan::Color getColor(std::string const &path) const;

    /** Loads a 2D vector object from the JSON file.
     *
     *  2D vectors are JSON arrays containing 2 numbers with the first
     *  element denoting position in the x-axis and the second element
     *  denoting position in the y-axis.
     *
     *  Valid JSON string:
     *      { "text-pos": [ 16, 32 ] }
     */
    clan::Vec2f getVec2f(std::string const &path) const;
    clan::Vec2i getVec2i(std::string const &path) const;

    /** Loads a 2D rectangle element from the JSON file.
     *
     *  2D rectangles can be represented in two different forms.
     *    - JSON array containing 4 numbers with the first two elements
     *      denoting the top-left 2D vector and the last two elements
     *      denoting the bottom-right 2D vector.
     *          { "box": [ 720, 540, 800, 600 ] }
     *    - JSON array containing 2 2D-vector arrays.
     *          { "box": [ [720, 540], [800, 600] ]}
     */
    clan::Rectf getRectf(std::string const &path) const;
    clan::Rect  getRecti(std::string const &path) const;

    /** Loads a font description element from the JSON file.
     */
    clan::FontDescription getFontDesc(std::string const &path) const;

    /** Loads a number from the JSON file as a `double`. */
    double getDecimal(std::string const &path) const;

    /** Loads a number from the JSON file as an `int`. */
    int getInteger(std::string const &path) const;

    /** Loads a boolean value from the JSON file. */
    bool getBoolean(std::string const &path) const;

    /** Loads a string from the JSON file. */
    std::string getString(std::string const &path) const;


    /** Retrieve if possible or overwrite with default value. */
    template< class ValueType, class ClassFunction >
        ValueType get_or_set(
            ClassFunction       func,
            std::string const & path,
            ValueType   const & default_value)
        {
            ValueType value;

            try {
                value = (this->*func)(path);
            } // TODO save value if fail
            catch ( std::out_of_range ) { value = default_value; }
            catch (clan::JsonException) { value = default_value; }

            return value;
        }

    /** Retrieve if valid or overwrite with default value. */
    template< class ValueType, class ClassFunction, class UnaryPredicate >
        ValueType get_if_else_set(
            ClassFunction       func,
            std::string const & path,
            ValueType   const & default_value,
            UnaryPredicate      p
            )
        {
            ValueType value;

            try {
                value = (this->*func)(path);
                value = p(value) ? value : default_value;
            } // TODO save value if fail
            catch ( std::out_of_range ) { value = default_value; }
            catch (clan::JsonException) { value = default_value; }

            return value;
        }
};

class JSONFile : public JSONReader
{
private:
    std::string const mFilePath;

public:
    JSONFile(std::string const &path);

    inline std::string const& getPath() { return mFilePath; }

    void load();
    void save();
};

#endif
