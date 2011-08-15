from mtree.tests.fixtures.generator import ADD, REMOVE, QUERY
"""
actions = '3a'
dimensions = 1
remove_chance = 0.1
"""

DIMENSIONS = 1

ACTIONS = (
	ADD((61,), QUERY((27,), 56.729998774400265, 1)),
	ADD((35,), QUERY((11,), 16.876046517052607, 6)),
	ADD((47,), QUERY((78,), 10.571538844029567, 4)),
)
