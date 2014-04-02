//  clanExt_JSONReader.cpp :: JSON reader
//  authored by Chu Chin Kuan
//
//  :: LICENSE AND COPYRIGHT ::
//
//  The author disclaims copyright to this source code.
//
//  The author or authors of this code dedicate any and all copyright interest
//  in this code to the public domain. We make this dedication for the benefit
//  of the public at large and to the detriment of our heirs and successors.
//
//  We intend this dedication to be an overt act of relinquishment in perpetuity
//  of all present and future rights to this code under copyright law.

#include "clanExt_JSONReader.hpp"

JSONReader::JSONReader() : mRoot() { }
JSONReader::JSONReader(clan::JsonValue const &root) : mRoot(root) { }

clan::JsonValue& JSONReader::getJsonValue(std::string const &path, clan::JsonValue& root)
{
    if (path.empty())
        return root;

    auto end = path.find_first_of('.');
    std::string part = path.substr(0, end);
    auto node = root.get_members().find(part);
    if (node == root.get_members().cend())
        throw std::out_of_range("Could not find member '" + part + "'");

    // TODO Parse JSON arrays
    // TODO Create JSON nodes for path if it doesn't exist

    if (end != std::string::npos)
        return getJsonValue(path.substr(end + 1, std::string::npos), node->second);
    else
        return node->second;
}

clan::JsonValue const & JSONReader::getJsonValue(std::string const &path, clan::JsonValue const &root)
{
    if (path.empty())
        return root;

    auto end = path.find_first_of('.');
    std::string part = path.substr(0, end);
    auto node = root.get_members().find(part);
    if (node == root.get_members().cend())
        throw std::out_of_range("Could not find member '" + part + "'");

    // TODO Parse JSON arrays

    if (end != std::string::npos)
        return getJsonValue(path.substr(end + 1, std::string::npos), node->second);
    else
        return node->second;
}

clan::Color JSONReader::getColor(std::string const &path) const
{
    clan::JsonValue const &json = getJsonValue(path, mRoot);

    if (json.is_string())
        // First form
        return clan::Color::find_color(json.to_string());

    if (json.is_array())
    {
        auto array = json.get_items();
        switch(array.size())
        {
            case 1:
                if (array[0].is_string())
                    // Second form
                    return clan::Color::find_color(json.to_string());
                break;

            case 2:
                if (array[0].is_string() && array[1].is_number())
                {
                    // Third form
                    clan::Color color = clan::Color::find_color(json.to_string());
                    color.set_alpha(array[1].to_int());
                    return color;
                }
                break;

            case 3:
                // Forth form
                return clan::Color(
                        array[0].to_int(),
                        array[1].to_int(),
                        array[2].to_int()
                        );

            case 4:
                // Forth form
                return clan::Color(
                        array[0].to_int(),
                        array[1].to_int(),
                        array[2].to_int(),
                        array[3].to_int()
                        );
        }
    }

    throw clan::JsonException(
            "Invalid JSON type while parsing color '" + path + "'."
            );

    return clan::Color();
}

clan::Vec2f JSONReader::getVec2f(std::string const &path) const
{
    clan::JsonValue const &json = getJsonValue(path, mRoot);

    if (json.is_array())
    {
        auto array = json.get_items();
        if (array.size() == 2)
            return clan::Vec2f(array[0].to_float(), array[1].to_float());
    }

    throw clan::JsonException(
            "Invalid JSON type while parsing 2D floating-point vector '" + path + "'."
            );

    return clan::Vec2f();
}

clan::Vec2i JSONReader::getVec2i(std::string const &path) const
{
    clan::JsonValue const &json = getJsonValue(path, mRoot);

    if (json.is_array())
    {
        auto array = json.get_items();
        if (array.size() == 2)
            return clan::Vec2i(array[0].to_int(), array[1].to_int());
    }

    throw clan::JsonException(
            "Invalid JSON type while parsing 2D integer vector '" + path + "'."
            );

    return clan::Vec2i();
}

clan::Rectf JSONReader::getRectf(std::string const &path) const
{
    clan::JsonValue const &json = getJsonValue(path, mRoot);

    if (json.is_array())
    {
        auto array = json.get_items();
        if (array.size() == 4)
            return clan::Rectf(
                    array[0].to_float(),
                    array[1].to_float(),
                    array[2].to_float(),
                    array[3].to_float()
                    );
        if (array.size() == 2)
            if (array[0].is_array() && array[1].is_array())
            {
                auto vecTL = array[0].get_items();
                auto vecBR = array[1].get_items();
                if (vecTL.size() && vecBR.size())
                    return clan::Rectf(
                            vecTL[0].to_float(),
                            vecTL[1].to_float(),
                            vecBR[0].to_float(),
                            vecBR[1].to_float()
                            );
            }
    }

    throw clan::JsonException(
            "Invalid JSON type while parsing 2D floating-point rectangle '" + path + "'."
            );

    return clan::Rectf();
}

clan::Rect JSONReader::getRecti(std::string const &path) const
{
    clan::JsonValue const &json = getJsonValue(path, mRoot);

    if (json.is_array())
    {
        auto array = json.get_items();
        if (array.size() == 4)
            return clan::Rect(
                    array[0].to_int(),
                    array[1].to_int(),
                    array[2].to_int(),
                    array[3].to_int()
                    );
        if (array.size() == 2)
            if (array[0].is_array() && array[1].is_array())
            {
                auto vecTL = array[0].get_items();
                auto vecBR = array[1].get_items();
                if (vecTL.size() && vecBR.size())
                    return clan::Rect(
                            vecTL[0].to_int(),
                            vecTL[1].to_int(),
                            vecBR[0].to_int(),
                            vecBR[1].to_int()
                            );
            }
    }

    throw clan::JsonException(
            "Invalid JSON type while parsing 2D integer rectangle '" + path + "'."
            );

    return clan::Rect();
}

clan::FontDescription JSONReader::getFontDesc(std::string const &path) const
{
    clan::FontDescription font_desc = clan::FontDescription();

    font_desc.set_subpixel(false);

    clan::JsonValue const &json = getJsonValue(path, mRoot);
    if (json.is_object())
    {
        for(auto node: json.get_members())
        {
            /****/ if (node.first.compare("typeface") == 0) {
                if (node.second.is_string())
                    font_desc.set_typeface_name(node.second.to_string());
                else
                    throw clan::JsonException(
                            "Invalid JSON type while parsing typeface name '" + path + "'."
                            );
            } else if (node.first.compare("height") == 0) {
                if (node.second.is_number())
                    font_desc.set_height(node.second.to_int());
                else
                    throw clan::JsonException(
                            "Invalid JSON type while parsing typeface height '" + path + "'."
                            );
            } else if (node.first.compare("weight") == 0) {
                if (node.second.is_number())
                    font_desc.set_weight(node.second.to_int());
                else
                    throw clan::JsonException(
                            "Invalid JSON type while parsing typeface weight '" + path + "'."
                            );
            } else if (node.first.compare("italic") == 0) {
                if (node.second.is_boolean())
                    font_desc.set_italic(node.second.to_boolean());
                else
                    throw clan::JsonException(
                            "Invalid JSON type while parsing typeface property '" + path + "'."
                            );
            } else if (node.first.compare("underline") == 0) {
                if (node.second.is_boolean())
                    font_desc.set_underline(node.second.to_boolean());
                else
                    throw clan::JsonException(
                            "Invalid JSON type while parsing typeface property '" + path + "'."
                            );
            } else if (node.first.compare("strikeout") == 0) {
                if (node.second.is_boolean())
                    font_desc.set_strikeout(node.second.to_boolean());
                else
                    throw clan::JsonException(
                            "Invalid JSON type while parsing typeface property '" + path + "'."
                            );
            } else if (node.first.compare("antialias") == 0) {
                if (node.second.is_boolean())
                    font_desc.set_anti_alias(node.second.to_boolean());
                else
                    throw clan::JsonException(
                            "Invalid JSON type while parsing typeface property '" + path + "'."
                            );
            } else if (node.first.compare("subpixel") == 0) {
                if (node.second.is_boolean())
                    font_desc.set_subpixel(node.second.to_boolean());
                else
                    throw clan::JsonException(
                            "Invalid JSON type while parsing typeface property '" + path + "'."
                            );
            } else { /* TODO Generate warning ? */ }
        }
    } else {
        throw clan::JsonException(
                "Invalid JSON type while parsing font descriptor '" + path + "'."
                );
    }

    return font_desc;
}


double JSONReader::getDecimal(std::string const &path) const
{
    return getJsonValue(path, mRoot).to_double();
}

int JSONReader::getInteger(std::string const &path) const
{
    return getJsonValue(path, mRoot).to_int();
}

bool JSONReader::getBoolean(std::string const &path) const
{
    return getJsonValue(path, mRoot).to_boolean();
}

std::string JSONReader::getString(std::string const &path) const
{
    return getJsonValue(path, mRoot).to_string();
}


