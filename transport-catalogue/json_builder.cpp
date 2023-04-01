#include "json_builder.h"

namespace json {

	//Builder

	Node& Builder::Build() {
		if (!Ready()) 		{
			throw std::logic_error("a closing parenthesis is expected"s);
		}
		return root_;
	}

	bool Builder::Ready() {
		if (nodes_stack_.size()) 		{
			return false;
		}
		return start_;
	}

	void Builder::ReadyToAdd() {
		if (Ready()) 		{
			throw std::logic_error("the formation has already been completed"s);
		}
	}

	void Builder::StartArrayDict(const Node& value) {
		ReadyToAdd();
		if (nodes_stack_.size() == 0) {
			root_ = value;
			nodes_stack_.push_back(&root_);
		}
		else {
			if (nodes_stack_.back()->IsArray()) {
				auto& arr = const_cast<Array&>(nodes_stack_.back()->AsArray());
				arr.push_back(value);
				nodes_stack_.push_back(&(arr.back()));
			}
			else { //если добавляем в словарь, то нужно еще имя
				if (!key_received) {
					throw std::logic_error("the dictionary requires a key"s);
				}
				auto& dict = const_cast<Dict&>(nodes_stack_.back()->AsDict());
				dict.insert({ name_, value });
				nodes_stack_.push_back(&(dict.at(name_)));
				key_received = false;
			}
		}
	}

	BuilderBeforeKey Builder::StartDict() {
		StartArrayDict(Dict{});
		return	 BuilderBeforeKey{ *this };
	}

	Builder& Builder::EndDict() {
		ReadyToAdd();
		if (!nodes_stack_.back()->IsDict()) 		{
			throw std::logic_error("dictionary closure is not expected"s);
		}
		nodes_stack_.pop_back();
		start_ = true;
		return *this;
	}

	BuilderAfteKey Builder::Key(const std::string& s) {
		ReadyToAdd();
		if (!nodes_stack_.back()->IsDict()) 		{
			throw std::logic_error("the name is only for the dictionary"s);
		}
		if (key_received) 		{
			throw std::logic_error("the name has already been received"s);
		}
		name_ = s;
		key_received = true;
		return  *this;
	}

	BuilderArr Builder::StartArray() {
		StartArrayDict(Array{});
		return  BuilderArr{ *this };
	}

	Builder& Builder::EndArray() {
		ReadyToAdd();
		if (!nodes_stack_.back()->IsArray()) 		{
			throw std::logic_error("array closure is not expected"s);
		}
		nodes_stack_.pop_back();
		start_ = true;
		return *this;
	}

	Builder& Builder::Value(const Node& value) {
		ReadyToAdd();
		if (nodes_stack_.size())		{
			if (nodes_stack_.back()->IsArray())			{
				const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(value);
			}
			else {
				if (!key_received) 				{
					throw std::logic_error("the dictionary requires a key"s);
				}
				const_cast<Dict&>(nodes_stack_.back()->AsDict()).insert({ name_, {value} });
				key_received = false;
			}
		}
		else root_ = { value }; //это одиночный простой json
		start_ = true;
		return *this;
	}

	//BuilderBase

	Builder& BuilderBase::GetBuilder() const {
		return builder_;
	}

	BuilderAfteKey BuilderBase::Key(const std::string& s) {
		builder_.Key(s);
		return builder_;
	}

	BuilderBase& BuilderBase::EndDict() {
		builder_.EndDict();
		return *this;
	}

	BuilderArr BuilderBase::StartArray() {
		builder_.StartArray();
		return builder_;
	}

	BuilderBeforeKey BuilderBase::StartDict() {
		builder_.StartDict();
		return builder_;
	}

	BuilderBase& BuilderBase::EndArray() {
		builder_.EndArray();
		return *this;
	}

	BuilderBase& BuilderBase::Value(const Node& value) {
		builder_.Value(value);
		return *this;
	}

	Node& BuilderBase::Build() {
		return builder_.Build();
	}

	BuilderBeforeKey BuilderAfteKey::Value(const Node& value)
	{
		GetBuilder().Value(value);
		return GetBuilder();
	}

	BuilderArr BuilderArr::Value(const Node& value) {
		GetBuilder().Value(value);
		return GetBuilder();
	}

}