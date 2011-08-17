#!/usr/bin/env python
from collections import namedtuple
import optparse
import random


DEFAULT_ACTIONS = 100
DEFAULT_DIMENSIONS = random.randint(1, 5)
DEFAULT_REMOVE_CHANCE = 0.1


Fixture = namedtuple('Fixture', 'DIMENSIONS, ACTIONS')


class Options(object):
	def __init__(self,
	             actions=DEFAULT_ACTIONS,
	             dimensions=DEFAULT_DIMENSIONS,
	             remove_chance=DEFAULT_REMOVE_CHANCE,
	            ):
		self.actions = actions
		self.dimensions = dimensions
		self.remove_chance = remove_chance




class Action(object):
	def __init__(self, data):
		self.data = data


class ActionWithQuery(Action):
	def __init__(self, data, query):
		super(ActionWithQuery, self).__init__(data)
		self.query = query
	
	def __repr__(self):
		return '%s(%r, %r)' % (self.__class__.__name__, self.data, self.query)


class ADD(ActionWithQuery):
	pass


class REMOVE(ActionWithQuery):
	pass


class QUERY(Action):
	def __init__(self, data, radius, limit):
		super(QUERY, self).__init__(data)
		self.radius = radius
		self.limit = limit
	
	def __repr__(self):
		return '%s(%r, %r, %r)' % (self.__class__.__name__, self.data, self.radius, self.limit)



def generate_data(dimensions):
	return tuple(random.randint(0, 100) for _ in range(dimensions))

	


def generate_listed_commands(dimensions, commands):
	num = None
	
	for ch in commands + ',':
		if '0' <= ch <= '9':
			if num is None:
				num = 0
			num = 10 * num + int(ch)
		elif ch in ('a', 'r', 'q'):
			if num is None:
				num = 1
			command = {'a':ADD, 'r':REMOVE}[ch]
			for _ in range(num):
				yield command
			num = None
		elif ch == ',':
			pass
		else:
			assert False, ch


def generate_random_commands(num_commands, remove_chance):
	for _ in range(num_commands):
		if random.random() < remove_chance:
			yield REMOVE
		else:
			yield ADD
	


def generate_commands(options):
	try:
		num_commands = int(options.actions)
	except ValueError:
		generator = generate_listed_commands(options.dimensions, options.actions)
	else:
		generator = generate_random_commands(num_commands, options.remove_chance)
	
	return generator
	


def generate_test_data(options):
	all_data = set()
	actions = []
	
	for cmd in generate_commands(options):
		if cmd == REMOVE and len(all_data) == 0:
			# There is nothing to remove
			continue
		
		if cmd == ADD:
			while True:
				data = generate_data(options.dimensions)
				if data not in all_data:
					break
			all_data.add(data)
		elif cmd == REMOVE:
			data = random.choice(list(all_data))
			all_data.remove(data)
		else:
			assert False
		
		# Creates a QueryAction for every AddAction and RemoveAction
		query_data = generate_data(options.dimensions)
		radius = random.random() * 80
		limit = random.randint(min(len(all_data), 1), int((len(all_data) + 5)*1.1))
		query = QUERY(query_data, radius, limit)
		
		action = cmd(data, query)
		
		actions.append(action)
	
	return Fixture(
			DIMENSIONS=options.dimensions,
			ACTIONS=actions,
		)


def print_test_data(fixture, options):
	print 'from mtree.tests.fixtures.generator import ADD, REMOVE, QUERY'
	print '"""'
	print 'actions = %r' % options.actions
	print 'dimensions = %r' % options.dimensions
	print 'remove_chance = %r' % options.remove_chance
	print '"""'
	print
	print 'DIMENSIONS = %d' % fixture.DIMENSIONS
	print
	print 'def PERFORM(callback):'
	for n, action in enumerate(fixture.ACTIONS, 1):
		print "\tcallback(%r)   # %d" % (action, n)



def main():
	defaults = Options()
	
	parser = optparse.OptionParser()
	parser.add_option("-a", "--actions"      ,               default=defaults.actions      , help="Total of actions or description of actions")
	parser.add_option("-d", "--dimensions"   , type="int"  , default=defaults.dimensions   , help="Number of dimensions")
	parser.add_option("-r", "--remove_chance", type="float", default=defaults.remove_chance, help="Chance of generating a REMOVE action")
	
	options, _ = parser.parse_args()
	
	test_data = generate_test_data(options)
	print_test_data(test_data, options)


if __name__ == '__main__':
	main()
