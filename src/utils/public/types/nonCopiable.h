#pragma once

class NonCopiable {
public:
	NonCopiable() = default;
	virtual ~NonCopiable() = default;
	NonCopiable(NonCopiable& other) = delete;
	NonCopiable(NonCopiable&& other) = delete;
	NonCopiable& operator=(const NonCopiable&) = delete;
	NonCopiable& operator=(const NonCopiable&&) = delete;
};