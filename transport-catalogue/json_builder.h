#pragma once

#include "json.h"

namespace json {

	using namespace std::literals;

	class BuilderBase;
	class BuilderBeforeKey;
	class BuilderAfteKey;
	class BuilderArr;

	class Builder {
	
	public:
		Node& Build();
		bool Ready();
		void ReadyToAdd();
		Builder& Value(const Node& value);
		BuilderBeforeKey StartDict();
		Builder& EndDict();
		BuilderAfteKey Key(const std::string& s);
		BuilderArr StartArray();
		Builder& EndArray();
		void StartArrayDict(const Node& value);

	private :
		Node root_;
		std::vector<Node*> nodes_stack_;
		std::string name_;
		bool key_received = false;
		bool start_ = false;
	};

	class BuilderBase { 
	public:
		BuilderBase(Builder& builder) : builder_(builder) {}

		BuilderAfteKey Key(const std::string& s);
		BuilderBeforeKey StartDict();
		BuilderBase& EndDict();
		BuilderArr StartArray();
		BuilderBase& EndArray();
		BuilderBase& Value(const Node& value);
		Node& Build();
		Builder& GetBuilder() const;
	private:
		Builder& builder_;
	};

	class BuilderBeforeKey : public BuilderBase { //пока не закроем словарь
	public:
		BuilderBeforeKey(Builder& builder) : BuilderBase(builder) {}

		BuilderBeforeKey StartDict() = delete;
		BuilderArr StartArray() = delete;
		BuilderBase EndArray() = delete;
		BuilderBase Value(const Node& value) = delete;
		Node& Build() = delete;
	};

	class BuilderAfteKey : public BuilderBase { //пока не закроем словарь
	public:
		BuilderAfteKey(Builder& builder) : BuilderBase(builder) {}

		BuilderBeforeKey Value(const Node& value);
		BuilderAfteKey Key(const std::string& s) = delete;
		BuilderBase EndDict() = delete;
		BuilderBase EndArray() = delete;
		Node& Build() = delete;
	};

	class BuilderArr : public BuilderBase {
	public:
		BuilderArr(Builder& builder) : BuilderBase(builder) {}

		BuilderAfteKey Key(const std::string& s) = delete;
		BuilderBase EndDict() = delete;
		BuilderArr Value(const Node& value);
		Node& Build() = delete;
	};

}  // namespace json