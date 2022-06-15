#pragma once

#include "json.h"

#include <vector>
#include <optional>

namespace json {

    class DictItemContext;
    class DictKeyContext;
    class ArrayItemContext;

    class Builder {

    public:

        Builder();

        DictKeyContext Key(std::string key);
        Builder& Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        Builder& EndDict();
        Builder& EndArray();

        json::Node Build();

    private:
        Node root_;
        std::vector<Node*> nodes_;
        std::optional<std::string> key_;

        bool IsEmpty();
        bool PrevIsArray();
        bool PrevIsDict();

        Node* AddElement(Node::Value value);

    };

    class DictItemContext {
    public:
        DictItemContext(Builder &builder);

        DictKeyContext Key(std::string key);
        Builder& EndDict();

    private:
        Builder& builder_;
    };

    class DictKeyContext {
    public:
        DictKeyContext(Builder &builder);

        DictItemContext Value (Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();

    private:
        Builder& builder_;
    };

    class ArrayItemContext {
    public:
        ArrayItemContext(Builder &builder);

        ArrayItemContext Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();

        Builder& EndArray();

    private:
        Builder& builder_;
    };

}