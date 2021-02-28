#pragma once

class NonCopyable {
public:
	NonCopyable() = default;
	virtual ~NonCopyable() = default;
	NonCopyable(NonCopyable& other) = delete;
	NonCopyable(NonCopyable&& other) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&&) = delete;
};