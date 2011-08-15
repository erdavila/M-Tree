from mtree.tests.fixtures.generator import ADD, REMOVE, QUERY
"""
actions = '2a2r'
dimensions = 2
remove_chance = 0.1
"""

DIMENSIONS = 2

ACTIONS = (
	ADD((17, 96), QUERY((85, 21), 63.623841838829016, 4)),
	ADD((60, 56), QUERY((90, 54), 60.29663611853935, 6)),
	REMOVE((17, 96), QUERY((64, 35), 9.527956792264458, 4)),
	REMOVE((60, 56), QUERY((25, 73), 6.3533672300254995, 5)),
)
