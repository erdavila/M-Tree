#ifndef FIXTURE_H_
#define FIXTURE_H_


#include <fstream>
#include <vector>


class Fixture {
public:

	typedef std::vector<int> Data;

	struct Action {
		char cmd;
		Data data;
		Data queryData;
		double radius;
		size_t limit;

		Action(char cmd, const Data& data, const Data& queryData, double radius, size_t limit)
			: cmd(cmd),
			  data(data),
			  queryData(queryData),
			  radius(radius),
			  limit(limit)
		{}
	};


	size_t dimensions;
	std::vector<Action> actions;


	Fixture(const Fixture&) = default;

	Fixture(Fixture&& fixture)
		: dimensions(fixture.dimensions)
	{
		actions.swap(fixture.actions);
	}

	~Fixture() = default;

	Fixture& operator=(const Fixture&) = default;

	static std::string path(const std::string& fixtureName) {
		return "cpp/tests/fixtures/" + fixtureName + ".txt";
	}

	static Fixture load(const std::string& fixtureName) {
		std::string fixtureFileName = path(fixtureName);
		std::ifstream fixtureFile(fixtureFileName);
		assert(fixtureFile);

		size_t dimensions;
		fixtureFile >> dimensions;

		size_t count;
		fixtureFile >> count;

		Fixture fixture = Fixture(dimensions);

		for(size_t i = 0; i < count; i++) {
			char cmd;
			fixtureFile >> cmd;

			Data data = readData(fixtureFile, dimensions);

			Data queryData = readData(fixtureFile, dimensions);

			double radius;
			fixtureFile >> radius;

			size_t limit;
			fixtureFile >> limit;

			fixture.actions.push_back(Action(cmd, data, queryData, radius, limit));
		}

		return fixture;
	}

private:
	Fixture(size_t dimensions) : dimensions(dimensions) { }

	Fixture() = delete;

	Fixture& operator=(Fixture&& fixture) {
		dimensions = fixture.dimensions;
		actions.swap(fixture.actions);
		return *this;
	}


	static Data readData(std::ifstream& file, size_t dimensions) {
		Data data;
		for(size_t d = 0; d < dimensions; d++) {
			Data::value_type value;
			file >> value;
			data.push_back(value);
		}
		return data;
	}
};



#endif /* FIXTURE_H_ */
