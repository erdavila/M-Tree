from mtree.tests.fixtures.generator import ADD, REMOVE, QUERY
"""
actions = '2a'
dimensions = 1
remove_chance = 0.1
"""

DIMENSIONS = 1

ACTIONS = (
	ADD((3,), QUERY((70,), 35.62683212650751, 6)),
	ADD((100,), QUERY((87,), 53.158693247000876, 6)),
)
