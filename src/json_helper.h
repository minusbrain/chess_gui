#pragma once

#include <initializer_list>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <type_traits>

#include "base/improve_containers.h"

using json = nlohmann::ordered_json;

// For all types except integral types:
template <typename T>
inline std::enable_if_t<!std::is_integral_v<T>, std::optional<T>> json_child_or_nullopt(const json& object, std::string child) {
    if (object.contains(child)) {
        try {
            return object[child].get<T>();
        } catch (const json::exception& ex) {
            return std::nullopt;
        }
    } else {
        return std::nullopt;
    }
}

// For integral types only:
template <typename T>
inline std::enable_if_t<std::is_integral_v<T>, std::optional<T>> json_child_or_nullopt(const json& object, std::string child) {
    if (object.contains(child)) {
        if (object[child].is_string()) {
            try {
                std::string hexstr = object[child].get<std::string>();
                return static_cast<T>(std::stoul(hexstr, nullptr, 0));
            } catch (const json::exception& ex) {
                return std::nullopt;
            }
        }
        try {
            return object[child].get<T>();
        } catch (const json::exception& ex) {
            return std::nullopt;
        }
    } else {
        return std::nullopt;
    }
}

// For all types except integral types:
template <typename T>
inline std::enable_if_t<!std::is_integral_v<T>, T> json_child_or(const json& object, std::string child, T or_value) {
    if (object.contains(child)) {
        try {
            return object[child].get<T>();
        } catch (const json::exception& ex) {
            return or_value;
        }
    } else {
        return or_value;
    }
}

// For integral types only:
template <typename T>
inline std::enable_if_t<std::is_integral_v<T>, T> json_child_or(const json& object, std::string child, T or_value) {
    if (object.contains(child)) {
        if (object[child].is_string()) {
            try {
                std::string hexstr = object[child].get<std::string>();
                return static_cast<T>(std::stoul(hexstr, nullptr, 0));
            } catch (const json::exception& ex) {
                return or_value;
            }
        }
        try {
            return object[child].get<T>();
        } catch (const json::exception& ex) {
            return or_value;
        }
    } else {
        return or_value;
    }
}

// For all types except integral types:
template <typename T>
inline std::enable_if_t<!std::is_integral_v<T>, T> json_child_or_fail(const json& object, std::string child) {
    if (object.contains(child)) {
        try {
            return object[child].get<T>();
        } catch (const json::exception& ex) {
            throw std::invalid_argument("JSON '" + object.dump(1) + "' child '" + child + "' not of expected type");
        }
    } else {
        throw std::invalid_argument("JSON '" + object.dump(1) + "' does not contain mandatory child '" + child + "'");
    }
}

// For integral types only:
template <typename T>
inline std::enable_if_t<std::is_integral_v<T>, T> json_child_or_fail(const json& object, std::string child) {
    if (object.contains(child)) {
        if (object[child].is_string()) {
            try {
                std::string hexstr = object[child].get<std::string>();
                return static_cast<T>(std::stoul(hexstr, nullptr, 0));
            } catch (const json::exception& ex) {
                throw std::invalid_argument("JSON '" + object.dump(1) + "' child '" + child +
                                            "' not of expected integer or hex-string type");
            }
        }
        try {
            return object[child].get<T>();
        } catch (const json::exception& ex) {
            throw std::invalid_argument("JSON '" + object.dump(1) + "' child '" + child + "' not of expected type");
        }
    } else {
        throw std::invalid_argument("JSON '" + object.dump(1) + "' does not contain mandatory child '" + child + "'");
    }
}

template <typename T>
inline std::vector<T> json_array_or_fail(const json& object, std::string child) {
    if (!object.contains(child) || !object[child].is_array()) {
        throw std::invalid_argument("JSON '" + object.dump(1) + "' does not contain mandatory array child '" + child + "'");
    } else {
        try {
            auto& array = object[child];
            std::vector<T> ret;
            for (auto& item : array) {
                ret.push_back(item);
            }
            return ret;
        } catch (const json::exception& ex) {
            throw std::invalid_argument("JSON '" + object.dump(1) + "' child '" + child + "' not of expected type");
        }
    }
}

template <typename T>
inline std::vector<T> json_array_or(const json& object, std::string child, const std::initializer_list<T>& or_value) {
    if (!object.contains(child)) {
        return std::vector(or_value);
    } else if (!object[child].is_array()) {
        throw std::invalid_argument("JSON '" + object.dump(1) + "' child item '" + child + "' is not an array");
    } else {
        try {
            auto& array = object[child];
            std::vector<T> ret;
            for (auto& item : array) {
                ret.push_back(item);
            }
            return ret;
        } catch (const json::exception& ex) {
            throw std::invalid_argument("JSON '" + object.dump(1) + "' child '" + child + "' not of expected type");
        }
    }
}

inline const json& json_find_child_by_name_or_fail(const json& object, const std::string& name) {
    for (const auto& child : object) {
        if (json_child_or<std::string>(child, "name", "") == name) {
            return child;
        }
    }
    throw std::invalid_argument("JSON '" + object.dump(1) + "' does not contain child with name '" + name + "'");
}

inline std::optional<json::value_type> get_element_by_path(json::value_type& root, const std::string& path) {
    json::json_pointer ptr{path};
    if (root.contains(ptr)) {
        json::value_type found = root.at(ptr);
        if (found.empty()) return std::nullopt;
        return found;
    }

    return std::nullopt;
}
