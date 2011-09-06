#!/usr/bin/env python
import sys
import os.path


sys.path.append(os.path.abspath("py"))


def main():
	fixture_name = sys.argv[1]
	mtree = __import__("mtree.tests.fixtures." + fixture_name)
	fixture = getattr(mtree.tests.fixtures, fixture_name)
	
	actions = []
	
	def callback(action):
		actions.append(action)
	
	fixture.PERFORM(callback)
	
	print fixture.DIMENSIONS
	print len(actions)
	
	for action in actions:
		if isinstance(action, mtree.tests.fixtures.generator.ADD):
			print 'A', format_data(action.data), format_data(action.query.data), repr(action.query.radius), action.query.limit
		elif isinstance(action, mtree.tests.fixtures.generator.REMOVE):
			print 'R', format_data(action.data), format_data(action.query.data), repr(action.query.radius), action.query.limit 
		else:
			assert False


def format_data(data):
	return ' '.join(map(str, data))


if __name__ == "__main__":
	main()
