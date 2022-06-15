#include "json_builder.h"

namespace json {

    DictKeyContext Builder::Key(std::string key) {
        using namespace std::string_literals;
        if (nodes_.back()->IsMap() && !key_) {
            key_ = std::move(key);
        } else {
            throw std::logic_error("The command \"Key\" in wrong place"s);
        }
        return *this;
    }

    Builder &Builder::Value(Node::Value value) {
        AddElement(value);
        return *this;
    }

    DictItemContext Builder::StartDict() {
        using namespace std::string_literals;
        if (IsEmpty() || key_ || PrevIsArray()) {
            nodes_.emplace_back(std::move(AddElement(Dict())));
            return DictItemContext(*this);
        } else {
            throw std::logic_error("The command \"StartDict\" in wrong place"s);
        }
    }

    ArrayItemContext Builder::StartArray() {
        using namespace std::string_literals;
        if (IsEmpty() || key_ || PrevIsArray()) {
            nodes_.emplace_back(std::move(AddElement(Array())));
            return *this;
        } else {
            throw std::logic_error("The command \"StartArray\" in wrong place"s);
        }
    }

    Builder &Builder::EndDict() {
        using namespace std::string_literals;
        if (PrevIsDict()) {
            nodes_.pop_back();
            return *this;
        } else {
            throw std::logic_error("The command \"EndDict\" in wrong place"s);
        }
    }

    Builder &Builder::EndArray() {
        using namespace std::string_literals;
        if (PrevIsArray()) {
            nodes_.pop_back();
            return *this;
        } else {
            throw std::logic_error("The command \"EndArray\" in wrong place"s);
        }
    }

    json::Node Builder::Build() {
        using namespace std::string_literals;
        if (nodes_.size() == 1 && !nodes_.back()->IsNull()) {
            return root_;
        } else {
            throw std::logic_error("The command \"Build\" in wrong place"s);
        }
    }

    bool Builder::IsEmpty() {
        return nodes_.back()->IsNull();
    }

    bool Builder::PrevIsArray() {
        return nodes_.back()->IsArray();
    }

    bool Builder::PrevIsDict() {
        return nodes_.back()->IsMap();
    }

    Node* Builder::AddElement(Node::Value value) {
        using namespace std::string_literals;
        if (IsEmpty()) {
            nodes_.back()->GetValue() = std::move(value);
            return nodes_.back();
        }
        else if (PrevIsArray()) {
            Array& link_arr = std::get<Array>(nodes_.back()->GetValue());
            return &link_arr.emplace_back(value);
        }
        else if (PrevIsDict() && key_) {
            Dict& link_dict = std::get<Dict>(nodes_.back()->GetValue());
            auto ptr = link_dict.emplace(make_pair(key_.value(), value));
            key_ = std::nullopt;
            return &ptr.first->second;
        }
        else {
            throw std::logic_error("The command \"Value\" in wrong place"s);
        }
    }

    Builder::Builder()
    : root_(nullptr)
    , nodes_{&root_}
    , key_(std::nullopt) {}

    DictItemContext::DictItemContext(Builder &builder)
    : builder_(builder) {}

    DictKeyContext DictItemContext::Key(std::string key) {
        return builder_.Key(key);
    }

    Builder &DictItemContext::EndDict() {
        return builder_.EndDict();
    }

    DictKeyContext::DictKeyContext(Builder &builder)
    : builder_(builder) {}

    DictItemContext DictKeyContext::Value(Node::Value value) {
        return DictItemContext(builder_.Value(value));
    }

    DictItemContext DictKeyContext::StartDict() {
        return builder_.StartDict();
    }

    ArrayItemContext DictKeyContext::StartArray() {
        return builder_.StartArray();
    }

    ArrayItemContext::ArrayItemContext(Builder &builder)
    : builder_(builder) {}

    ArrayItemContext ArrayItemContext::Value(Node::Value value) {
        return ArrayItemContext(builder_.Value(value));
    }

    DictItemContext ArrayItemContext::StartDict() {
        return builder_.StartDict();
    }

    ArrayItemContext ArrayItemContext::StartArray() {
        return builder_.StartArray();
    }

    Builder &ArrayItemContext::EndArray() {
        return builder_.EndArray();
    }
}