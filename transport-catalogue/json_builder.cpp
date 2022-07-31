#include "json_builder.h"

namespace json {

    DictKeyContext Builder::Key(std::string key) {
        using namespace std::string_literals;
        if (nodes_stack_.back()->IsDict() && !key_) {
            key_ = std::move(key);
        } else {
            throw std::logic_error("The command \"Key\" was called in wrong place"s);
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
            nodes_stack_.emplace_back(std::move(AddElement(Dict())));
            return DictItemContext(*this);
        } else {
            throw std::logic_error("The command \"StartDict\" was called in wrong place"s);
        }
    }

    ArrayItemContext Builder::StartArray() {
        using namespace std::string_literals;
        if (IsEmpty() || key_ || PrevIsArray()) {
            nodes_stack_.emplace_back(std::move(AddElement(Array())));
            return *this;
        } else {
            throw std::logic_error("The command \"StartArray\" was called in wrong place"s);
        }
    }

    Builder &Builder::EndDict() {
        using namespace std::string_literals;
        if (PrevIsDict()) {
            nodes_stack_.pop_back();
            return *this;
        } else {
            throw std::logic_error("The command \"EndDict\" was called in wrong place"s);
        }
    }

    Builder &Builder::EndArray() {
        using namespace std::string_literals;
        if (PrevIsArray()) {
            nodes_stack_.pop_back();
            return *this;
        } else {
            throw std::logic_error("The command \"EndArray\" was called in wrong place"s);
        }
    }

    json::Node Builder::Build() {
        using namespace std::string_literals;
        if (nodes_stack_.size() == 1 && !nodes_stack_.back()->IsNull()) {
            return root_;
        } else {
            throw std::logic_error("The command \"Build\" was called in wrong place"s);
        }
    }

    bool Builder::IsEmpty() {
        return nodes_stack_.back()->IsNull();
    }

    bool Builder::PrevIsArray() {
        return nodes_stack_.back()->IsArray();
    }

    bool Builder::PrevIsDict() {
        return nodes_stack_.back()->IsDict();
    }

    Node* Builder::AddElement(Node::Value value) {
        using namespace std::string_literals;
        if (IsEmpty()) {
            nodes_stack_.back()->GetValue() = std::move(value);
            return nodes_stack_.back();
        }
        else if (PrevIsArray()) {
            Array& link_arr = std::get<Array>(nodes_stack_.back()->GetValue());
            return &link_arr.emplace_back(value);
        }
        else if (PrevIsDict() && key_) {
            Dict& link_dict = std::get<Dict>(nodes_stack_.back()->GetValue());
            auto ptr = link_dict.emplace(make_pair(key_.value(), value));
            key_ = std::nullopt;
            return &ptr.first->second;
        }
        else {
            throw std::logic_error("The command \"Value\" was called in wrong place"s);
        }
    }

    Builder::Builder()
    : root_(nullptr)
    , nodes_stack_{&root_}
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
