from mtree.tests.fixtures.generator import ADD, REMOVE, QUERY
"""
actions = 'arar'
dimensions = 2
remove_chance = 0.1
"""

DIMENSIONS = 2

def PERFORM(callback):
	callback(ADD((34, 23), QUERY((73, 25), 43.55632520107637, 4)))
	callback(REMOVE((34, 23), QUERY((36, 42), 67.77809887606128, 3)))
	callback(ADD((74, 74), QUERY((4, 95), 40.44656851152691, 5)))
	callback(REMOVE((74, 74), QUERY((12, 16), 68.96988175971654, 2)))
