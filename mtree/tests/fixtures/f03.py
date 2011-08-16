from mtree.tests.fixtures.generator import ADD, REMOVE, QUERY
"""
actions = '3a3r3a3r'
dimensions = 4
remove_chance = 0.1
"""

DIMENSIONS = 4

def PERFORM(callback):
	callback(ADD((76, 37, 59, 92), QUERY((57, 30, 48, 39), 49.27564061937317, 4)))
	callback(ADD((50, 18, 85, 45), QUERY((39, 21, 39, 70), 37.69483637082036, 4)))
	callback(ADD((78, 25, 54, 0), QUERY((93, 78, 37, 31), 0.7284187591077362, 1)))
	callback(REMOVE((78, 25, 54, 0), QUERY((64, 84, 32, 57), 72.46412020708925, 5)))
	callback(REMOVE((50, 18, 85, 45), QUERY((27, 65, 93, 13), 64.11474980906463, 5)))
	callback(REMOVE((76, 37, 59, 92), QUERY((60, 4, 33, 59), 42.945752330314434, 0)))
	callback(ADD((70, 4, 97, 67), QUERY((85, 99, 62, 47), 17.638848416562205, 5)))
	callback(ADD((73, 71, 83, 34), QUERY((77, 92, 94, 84), 36.61127295233709, 3)))
	callback(ADD((68, 95, 14, 64), QUERY((2, 6, 91, 87), 55.49188671780003, 3)))
	callback(REMOVE((70, 4, 97, 67), QUERY((21, 10, 23, 86), 45.80958512596745, 7)))
	callback(REMOVE((68, 95, 14, 64), QUERY((46, 83, 34, 29), 1.790981022255398, 2)))
	callback(REMOVE((73, 71, 83, 34), QUERY((47, 82, 60, 46), 77.12167724151024, 4)))
