#include "EnvironmentBuilder.hpp"

//----------------- CONSTRUCTORS ---------------------//

Environment::Environment(std::vector<std::string>& envAsStringVect)
	:_envAsStrings(envAsStringVect)
{

}

Environment::Environment(const Environment &original)
{
	*this = original;
}

//----------------- DESTRUCTOR -----------------------//

Environment::~Environment()
{

}

//------------------- OPERATORS ----------------------//

Environment& Environment::operator=(const Environment &original)
{
	if (this == &original)
		return *this;

	_envAsStrings = original._envAsStrings;
	return *this;
}

//------------------- SETTERS ------------------------//

//------------------- GETTERS ------------------------//

char**	Environment::getEnv()
{
	for (size_t i = 0; i < _envAsStrings.size(); ++i) {
		_envAsCharPtr.push_back(const_cast<char*>(_envAsStrings[i].c_str()));
	}
	_envAsCharPtr.push_back(NULL);

	return &_envAsCharPtr[0];
}
