#include <iostream>
#include <optional>
#include <format>

std::optional<int> getOptional(bool giveIt)
{
	if( giveIt)
	{
		return 42;
	}
	
	return std::nullopt; // or {}
}
int main()
{
	auto myOptional = getOptional(true);
	auto myOptional2 = getOptional(false);
	std::cout << "Example Optional" << std::endl;
	std::cout << std::format("has value() : myoptional {}", myOptional.has_value() ) << std::endl;
	std::cout << std::format("has value() : myoptional2 {}", myOptional2.has_value() ) << std::endl;

	if( myOptional.has_value())
	{
		std::cout << std::format("myOptional.value() = {} ", myOptional.value()) << std::endl;
		std::cout << std::format("*myOptional = {} ", *myOptional) << std::endl;
	}
	else
	{
		std::cout << std::format("No value") << std::endl;
	}
	// if no data is available, then use the value_or() method
	std::cout << std::format("myOptional2.value_or(100) = {}", myOptional2.value_or(100)) << std::endl;
	// if no data call value() method, it will throw an exception std::bad_optional_access
	return 0;
}
