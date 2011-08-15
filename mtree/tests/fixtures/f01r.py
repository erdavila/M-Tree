from mtree.tests.fixtures.generator import ADD, REMOVE, QUERY
"""
actions = 'ar'
dimensions = 4
remove_chance = 0.1
"""

DIMENSIONS = 4

ACTIONS = (
	ADD((11, 64, 76, 58), QUERY((86, 12, 34, 83), 3.0922637673500386, 4)),
	REMOVE((11, 64, 76, 58), QUERY((85, 90, 79, 42), 73.52161095669746, 2)),
)
