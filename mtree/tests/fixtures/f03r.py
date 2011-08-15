from mtree.tests.fixtures.generator import ADD, REMOVE, QUERY
"""
actions = '3a3r'
dimensions = 1
remove_chance = 0.1
"""

DIMENSIONS = 1

ACTIONS = (
	ADD((34,), QUERY((88,), 52.52227754725948, 1)),
	ADD((44,), QUERY((90,), 70.69291307333901, 2)),
	ADD((0,), QUERY((88,), 41.130939000327025, 1)),
	REMOVE((0,), QUERY((3,), 65.29167245184956, 3)),
	REMOVE((34,), QUERY((24,), 7.610060642307399, 3)),
	REMOVE((44,), QUERY((69,), 18.384684628176522, 3)),
)
